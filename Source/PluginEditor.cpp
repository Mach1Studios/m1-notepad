/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NotePadAudioProcessorEditor::NotePadAudioProcessorEditor (NotePadAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    m1TextEditor.reset(new StrikethroughTextEditor("new text editor"));
    addAndMakeVisible(m1TextEditor.get());
    m1TextEditor->addListener(this);
    m1TextEditor->setMultiLine(true);
    m1TextEditor->setReturnKeyStartsNewLine(true);
    m1TextEditor->setReadOnly(false);
    m1TextEditor->setScrollbarsShown(true);
    m1TextEditor->setCaretVisible(true);
    m1TextEditor->setPopupMenuEnabled(true);
    m1TextEditor->setTabKeyUsedAsCharacter(true);
    m1TextEditor->setTextToShowWhenEmpty("Keep session notes here...", juce::Colours::white);
    m1TextEditor->setText(audioProcessor.treeState.state.getProperty("SessionText")); // Grabs the string within property labeled "SessionText"
    
    // Load strikethrough ranges if they exist
    auto strikethroughData = audioProcessor.treeState.state.getProperty("StrikethroughRanges").toString();
    if (strikethroughData.isNotEmpty())
    {
        m1TextEditor->deserializeStrikethroughRanges(strikethroughData);
    }
    
    // Set up callback to save strikethrough ranges when they change
    m1TextEditor->onStrikethroughChanged = [this]()
    {
        auto strikethroughData = m1TextEditor->serializeStrikethroughRanges();
        audioProcessor.treeState.state.setProperty("StrikethroughRanges", strikethroughData, nullptr);
        updateStrikethroughButtonState();
    };
    
    // Strikethrough button setup
    strikethroughButton.reset(new StrikethroughButton());
    addAndMakeVisible(strikethroughButton.get());
    strikethroughButton->addListener(this);
    strikethroughButton->setTooltip("Toggle strikethrough on selected text");
    strikethroughButton->setVisible(true); // Always visible in notepad area
    
    // Start timer to update button state when selection changes
    startTimer(100); // Check every 100ms
    
    m1TextEditor->setBounds(0, 0, 800, 512 - 20);
    m1TextEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(40.0f, 40.0f, 40.0f, 0.10f));
    m1TextEditor->setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(251.0f, 251.0f, 251.0f, 1.0f));
    m1TextEditor->setColour(juce::TextEditor::highlightColourId, juce::Colour::fromFloatRGBA(242.0f, 255.0f, 95.0f, 0.25f));
    m1TextEditor->setVisible(true); // Always visible in notepad area
    
    // Todo checkbox setup - remove or hide it since we always show both views
    todoCheckbox.reset(new juce::ToggleButton("Todo Mode"));
    addAndMakeVisible(todoCheckbox.get());
    todoCheckbox->addListener(this);
    todoCheckbox->setToggleState(audioProcessor.isTodoMode(), juce::dontSendNotification);
    todoCheckbox->setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    todoCheckbox->setVisible(false); // Hide the checkbox since both views are always visible
    
    // Todo input field setup
    todoInputField.reset(new juce::TextEditor("todo input"));
    addAndMakeVisible(todoInputField.get());
    todoInputField->addListener(this);
    todoInputField->setMultiLine(false);
    todoInputField->setReturnKeyStartsNewLine(false);
    todoInputField->setReadOnly(false);
    todoInputField->setScrollbarsShown(false);
    todoInputField->setCaretVisible(true);
    todoInputField->setPopupMenuEnabled(true);
    todoInputField->setTextToShowWhenEmpty("Type a new todo and press Enter...", juce::Colours::white);
    todoInputField->setVisible(true); // Always visible in todo area
    todoInputField->setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(40.0f, 40.0f, 40.0f, 0.10f));
    todoInputField->setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(251.0f, 251.0f, 251.0f, 1.0f));
    
    refreshTodoList();
    
    m1logo = juce::ImageCache::getFromMemory(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);
    
    setResizable(true, true);
    setSize(1200, 512); // Wider default size to accommodate split view
}

NotePadAudioProcessorEditor::~NotePadAudioProcessorEditor()
{
    stopTimer();
    m1TextEditor = nullptr;
    todoCheckbox = nullptr;
    todoInputField = nullptr;
    strikethroughButton = nullptr;
    todoItems.clear();
    todoLabels.clear();
    todoEditors.clear();
}

//==============================================================================
void NotePadAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(40, 40, 40));

    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white);
    
    // Draw divider line between notepad and todo panes
    int dividerX = getWidth() / 2;
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawVerticalLine(dividerX, 0.0f, static_cast<float>(getHeight()));
    // Draw a second line slightly offset for a subtle 3D effect
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawVerticalLine(dividerX + 1, 0.0f, static_cast<float>(getHeight()));
    
    //m1logo
    g.drawImageWithin(m1logo, -15, getHeight() - 17, 161 / 2, 39 / 4, juce::RectanglePlacement());
}

void NotePadAudioProcessorEditor::resized()
{
    // Safety function to ensure we dont crash when making the window too small
    int w = getWidth();
    int h = getHeight();
    if (w < 300) w = 300; // Increased minimum width to accommodate split view
    if (h < 100) h = 100; // set the minimum height
    if (w != getWidth() || h != getHeight()) {
        setSize(w, h);
    }
    
    // Calculate split point (vertical divider in the middle)
    int dividerX = getWidth() / 2;
    int dividerWidth = 2; // Width of the divider line
    
    // Left pane: Notepad
    int notepadWidth = dividerX;
    int buttonHeight = 30;
    int buttonTopSpacing = 5;
    int buttonBottomSpacing = 5;
    int totalButtonSpace = buttonHeight + buttonTopSpacing + buttonBottomSpacing;
    
    // Position strikethrough button in notepad area
    if (strikethroughButton != nullptr)
    {
        strikethroughButton->setBounds(5, buttonTopSpacing, 50, buttonHeight);
    }
    
    // Position text editor in left pane (below button)
    int editorY = totalButtonSpace;
    int editorHeight = getHeight() - editorY;
    m1TextEditor->setBounds(0, editorY, notepadWidth, editorHeight);
    
    // Right pane: Todo list
    int todoPaneX = dividerX + dividerWidth;
    int todoPaneWidth = getWidth() - todoPaneX;
    int todoY = 10;
    int inputFieldHeight = 24;
    int inputFieldY = getHeight() - inputFieldHeight - 10; // Leave some space at bottom
    
    // Position todo items in right pane
    for (int i = 0; i < todoItems.size(); ++i)
    {
        int itemX = todoPaneX + 10;
        int itemWidth = todoPaneWidth - 20;
        
        // Always show all items (they may overflow, but that's okay)
        todoItems[i]->setVisible(true);
        todoItems[i]->setBounds(itemX, todoY, 22, 20);
        
        if (todoEditors[i] != nullptr && todoEditors[i]->isVisible())
        {
            todoEditors[i]->setVisible(true);
            todoEditors[i]->setBounds(itemX + 30, todoY, itemWidth - 30, 20);
        }
        else
        {
            todoLabels[i]->setVisible(true);
            todoLabels[i]->setBounds(itemX + 30, todoY, itemWidth - 30, 20);
        }
        
        todoY += 30;
    }
    
    // Position input field at the bottom of todo pane
    int inputFieldX = todoPaneX + 10;
    int inputFieldWidth = todoPaneWidth - 20;
    todoInputField->setBounds(inputFieldX, inputFieldY, inputFieldWidth, inputFieldHeight);
}

void NotePadAudioProcessorEditor::textEditorTextChanged (juce::TextEditor &editor)
{
    // On key changes will save editor's string to property labeled/tagged "SessionText"
    audioProcessor.treeState.state.setProperty("SessionText", editor.getText(), nullptr);
    
    // Save strikethrough ranges if this is the main text editor
    if (&editor == m1TextEditor.get())
    {
        // Adjust strikethrough ranges when text changes
        m1TextEditor->adjustStrikethroughRanges();
        
        auto strikethroughData = m1TextEditor->serializeStrikethroughRanges();
        audioProcessor.treeState.state.setProperty("StrikethroughRanges", strikethroughData, nullptr);
        
        // Update button state when text changes (selection might have moved)
        updateStrikethroughButtonState();
    }
}

void NotePadAudioProcessorEditor::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == todoInputField.get())
    {
        juce::String text = todoInputField->getText().trim();
        if (text.isNotEmpty())
        {
            TodoItem newItem;
            newItem.text = text;
            newItem.completed = false;
            addTodoItem(newItem);
            updateTodoItemsState();
            todoInputField->setText("");
            resized();
        }
    }
    else if (editingIndex >= 0 && editingIndex < todoEditors.size())
    {
        // Finish editing a todo item
        juce::String text = todoEditors[editingIndex]->getText().trim();
        if (text.isNotEmpty())
        {
            todoLabels[editingIndex]->setText(text, juce::dontSendNotification);
            todoEditors[editingIndex]->setVisible(false);
            todoLabels[editingIndex]->setVisible(true);
            updateTodoItemsState();
        }
        editingIndex = -1;
    }
}

void NotePadAudioProcessorEditor::editTodoItem(int index)
{
    if (index >= 0 && index < todoLabels.size())
    {
        editingIndex = index;
        todoLabels[index]->setVisible(false);
        
        if (todoEditors[index] == nullptr)
        {
            todoEditors.add(new juce::TextEditor());
            addAndMakeVisible(todoEditors[index]);
            todoEditors[index]->addListener(this);
            todoEditors[index]->setMultiLine(false);
            todoEditors[index]->setReturnKeyStartsNewLine(false);
            todoEditors[index]->setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(40.0f, 40.0f, 40.0f, 0.10f));
            todoEditors[index]->setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(251.0f, 251.0f, 251.0f, 1.0f));
        }
        
        todoEditors[index]->setText(todoLabels[index]->getText());
        todoEditors[index]->setVisible(true);
        todoEditors[index]->grabKeyboardFocus();
        resized();
    }
}

void NotePadAudioProcessorEditor::addTodoItem(const juce::String& text, bool checked)
{
    int index = todoItems.size();
    
    // Create checkbox
    auto* checkbox = new juce::ToggleButton("");
    todoItems.add(checkbox);
    addAndMakeVisible(checkbox);
    checkbox->addListener(this);
    checkbox->setToggleState(checked, juce::dontSendNotification);
    checkbox->setVisible(true); // Always visible in todo area
    
    // Create label
    auto* label = new juce::Label("", text);
    todoLabels.add(label);
    addAndMakeVisible(label);
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setVisible(true); // Always visible in todo area
    label->addMouseListener(this, false);
    
    // Set initial bounds
    checkbox->setBounds(10, 10 + index * 30, 22, 20);
    label->setBounds(40, 10 + index * 30, getWidth() - 50, 20);
    
    // Update selection
    selectedIndex = index;
    updateVisualState();
}

void NotePadAudioProcessorEditor::deleteTodoItem(int index)
{
    if (index >= 0 && index < todoItems.size())
    {
        todoItems.remove(index);
        todoLabels.remove(index);
        todoEditors.remove(index);
        
        if (editingIndex == index)
            editingIndex = -1;
            
        if (selectedIndex >= todoItems.size())
            selectedIndex = todoItems.size() - 1;
            
        updateTodoItemsState();
        updateVisualState();
        resized();
    }
}

void NotePadAudioProcessorEditor::moveSelection(int delta)
{
    if (todoItems.size() > 0)
    {
        int newIndex = selectedIndex + delta;
        if (newIndex >= 0 && newIndex < todoItems.size())
        {
            selectedIndex = newIndex;
            updateVisualState();
        }
    }
}

void NotePadAudioProcessorEditor::updateVisualState()
{
    for (int i = 0; i < todoLabels.size(); ++i)
    {
        const auto& item = todoData[i];
        bool isSelected = (i == selectedIndex);
        
        // Update label colors and style
        todoLabels[i]->setColour(juce::Label::textColourId,
            item.completed ? juce::Colours::grey :
            isSelected ? juce::Colours::lightblue :
            getPriorityColour(item.priority));
            
        // Add strikethrough for completed items
        todoLabels[i]->setFont(juce::Font(16.0f,
            item.completed ? juce::Font::bold : juce::Font::plain));
    }
}

bool NotePadAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    // Todo keyboard shortcuts work when focus is in todo area
    // Check if focus is on todo input field or a todo item
    bool isTodoFocused = todoInputField->hasKeyboardFocus(true);
    for (int i = 0; i < todoEditors.size(); ++i)
    {
        if (todoEditors[i] != nullptr && todoEditors[i]->hasKeyboardFocus(true))
        {
            isTodoFocused = true;
            break;
        }
    }
    
    if (isTodoFocused || selectedIndex >= 0)
    {
        if (key == juce::KeyPress::upKey)
        {
            moveSelection(-1);
            return true;
        }
        else if (key == juce::KeyPress::downKey)
        {
            moveSelection(1);
            return true;
        }
        else if (key == juce::KeyPress::spaceKey && selectedIndex >= 0)
        {
            todoItems[selectedIndex]->setToggleState(!todoItems[selectedIndex]->getToggleState(), juce::sendNotification);
            return true;
        }
        else if (key == juce::KeyPress::returnKey && selectedIndex >= 0)
        {
            editTodoItem(selectedIndex);
            return true;
        }
        else if (key == juce::KeyPress::deleteKey && selectedIndex >= 0)
        {
            deleteTodoItem(selectedIndex);
            return true;
        }
    }
    return false;
}

void NotePadAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == todoCheckbox.get())
    {
        // Checkbox is now hidden, but keep the logic for state management
        bool todoMode = todoCheckbox->getToggleState();
        audioProcessor.setTodoMode(todoMode);
        // Both views are always visible now, so no need to toggle visibility
        resized(); // Update layout
    }
    else if (button == strikethroughButton.get())
    {
        // Handle strikethrough button click
        auto selRange = m1TextEditor->getHighlightedRegion();
        if (selRange.getLength() > 0)
        {
            m1TextEditor->toggleStrikethrough(selRange);
            updateStrikethroughButtonState();
        }
    }
    else
    {
        // Check if it's one of the todo checkboxes
        for (int i = 0; i < todoItems.size(); ++i)
        {
            if (button == todoItems[i])
            {
                selectedIndex = i;
                updateVisualState();
                updateTodoItemsState();
                break;
            }
        }
    }
}

void NotePadAudioProcessorEditor::refreshTodoList()
{
    // Clear existing items
    todoItems.clear();
    todoLabels.clear();
    todoEditors.clear();
    
    // Load from state
    auto todoArray = audioProcessor.treeState.state.getChildWithName("TodoItems");
    if (todoArray.isValid())
    {
        for (int i = 0; i < todoArray.getNumChildren(); ++i)
        {
            auto todoItem = todoArray.getChild(i);
            juce::String text = todoItem.getProperty("Text").toString();
            bool checked = static_cast<bool>(todoItem.getProperty("Checked"));
            TodoItem newItem;
            newItem.text = text;
            newItem.completed = checked;
            addTodoItem(newItem);
        }
    }
}

void NotePadAudioProcessorEditor::updateTodoItemsState()
{
    // Create or get the todo items array
    juce::ValueTree todoArray = audioProcessor.treeState.state.getOrCreateChildWithName("TodoItems", nullptr);
    todoArray.removeAllChildren(nullptr);
    
    // Store each todo item
    for (int i = 0; i < todoItems.size(); ++i)
    {
        juce::ValueTree todoItem("TodoItem" + juce::String(i));
        todoItem.setProperty("Text", todoLabels[i]->getText(), nullptr);
        todoItem.setProperty("Checked", todoItems[i]->getToggleState(), nullptr);
        todoArray.appendChild(todoItem, nullptr);
    }
}

void NotePadAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& e)
{
    // Only handle double-click in todo area (right pane)
    int dividerX = getWidth() / 2;
    if (e.getPosition().x > dividerX)
    {
        for (int i = 0; i < todoLabels.size(); ++i)
        {
            if (todoLabels[i]->isVisible() && todoLabels[i]->getBounds().contains(e.getPosition()))
            {
                editTodoItem(i);
                break;
            }
        }
    }
}

void NotePadAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    // Set up drag start index for todo item reordering
    int dividerX = getWidth() / 2;
    if (e.getPosition().x > dividerX)
    {
        for (int i = 0; i < todoLabels.size(); ++i)
        {
            if (todoLabels[i]->isVisible() && todoLabels[i]->getBounds().contains(e.getPosition()))
            {
                dragStartIndex = i;
                selectedIndex = i;
                updateVisualState();
                break;
            }
        }
    }
}

void NotePadAudioProcessorEditor::addTodoItem(const TodoItem& item)
{
    todoData.add(item);
    
    int index = todoData.size() - 1;
    
    // Create checkbox
    auto* checkbox = new juce::ToggleButton("");
    todoItems.add(checkbox);
    addAndMakeVisible(checkbox);
    checkbox->addListener(this);
    checkbox->setToggleState(item.completed, juce::dontSendNotification);
    checkbox->setVisible(true); // Always visible in todo area
    
    // Create label with priority and category indicators
    auto* label = new juce::Label("", item.text);
    todoLabels.add(label);
    addAndMakeVisible(label);
    label->setColour(juce::Label::textColourId, getPriorityColour(item.priority));
    label->setVisible(true); // Always visible in todo area
    label->addMouseListener(this, false);
    
    // Set initial bounds
    checkbox->setBounds(10, 10 + index * 30, 22, 20);
    label->setBounds(40, 10 + index * 30, getWidth() - 50, 20);
    
    // Update selection
    selectedIndex = index;
    updateVisualState();
    updateTodoItemsState();
}

void NotePadAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
    // Allow dragging in todo area (right pane)
    int dividerX = getWidth() / 2;
    if (e.getPosition().x > dividerX && dragStartIndex >= 0)
    {
        isDragging = true;
        int currentIndex = -1;
        
        // Find which item we're dragging over
        for (int i = 0; i < todoLabels.size(); ++i)
        {
            if (todoLabels[i]->getBounds().contains(e.getPosition()))
            {
                currentIndex = i;
                break;
            }
        }
        
        if (currentIndex >= 0 && currentIndex != dragStartIndex)
        {
            reorderItems(dragStartIndex, currentIndex);
            dragStartIndex = currentIndex;
        }
    }
}

void NotePadAudioProcessorEditor::mouseUp(const juce::MouseEvent& e)
{
    if (isDragging)
    {
        isDragging = false;
        dragStartIndex = -1;
        updateVisualState();
    }
}

void NotePadAudioProcessorEditor::reorderItems(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < todoData.size() && 
        toIndex >= 0 && toIndex < todoData.size())
    {
        todoData.move(fromIndex, toIndex);
        todoItems.move(fromIndex, toIndex);
        todoLabels.move(fromIndex, toIndex);
        todoEditors.move(fromIndex, toIndex);
        
        if (editingIndex == fromIndex)
            editingIndex = toIndex;
            
        selectedIndex = toIndex;
        updateTodoItemsState();
        resized();
    }
}

void NotePadAudioProcessorEditor::filterItems(const juce::String& searchText)
{
    for (int i = 0; i < todoLabels.size(); ++i)
    {
        const auto& item = todoData[i];
        bool matches = searchText.isEmpty() ||
                      item.text.containsIgnoreCase(searchText) ||
                      item.tags.containsIgnoreCase(searchText);
                      
        todoItems[i]->setVisible(matches); // Always visible when matching
        todoLabels[i]->setVisible(matches); // Always visible when matching
    }
    resized();
}
juce::Colour NotePadAudioProcessorEditor::getPriorityColour(Priority p) const
{
    switch (p)
    {
        case Priority::High: return juce::Colours::red;
        case Priority::Medium: return juce::Colours::orange;
        case Priority::Low: return juce::Colours::white;
        default: return juce::Colours::white;
    }
}

void NotePadAudioProcessorEditor::updateStrikethroughButtonState()
{
    if (strikethroughButton == nullptr || m1TextEditor == nullptr)
        return;
        
    auto selRange = m1TextEditor->getHighlightedRegion();
    bool hasSelection = selRange.getLength() > 0;
    bool isStrikethrough = hasSelection && m1TextEditor->isRangeStrikethrough(selRange);
    
    strikethroughButton->setActive(isStrikethrough);
    strikethroughButton->setEnabled(hasSelection && !m1TextEditor->isReadOnly());
}

void NotePadAudioProcessorEditor::timerCallback()
{
    // Update strikethrough button state when selection changes
    updateStrikethroughButtonState();
}
