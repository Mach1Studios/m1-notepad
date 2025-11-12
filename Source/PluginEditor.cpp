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
    m1TextEditor.reset(new juce::TextEditor("new text editor"));
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
    
    // Fullscreen buttons setup
    leftFullscreenButton.reset(new FullscreenButton("LeftFullscreen"));
    addAndMakeVisible(leftFullscreenButton.get());
    leftFullscreenButton->addListener(this);
    leftFullscreenButton->setTooltip("Fullscreen notepad");
    
    rightFullscreenButton.reset(new FullscreenButton("RightFullscreen"));
    addAndMakeVisible(rightFullscreenButton.get());
    rightFullscreenButton->addListener(this);
    rightFullscreenButton->setTooltip("Fullscreen todo list");
    
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
    m1TextEditor = nullptr;
    todoCheckbox = nullptr;
    todoInputField = nullptr;
    leftFullscreenButton = nullptr;
    rightFullscreenButton = nullptr;
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
    
    //m1logo
    g.drawImageWithin(m1logo, -15, getHeight() - 17, 161 / 2, 39 / 4, juce::RectanglePlacement());
}

void NotePadAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    // Only draw divider if not in fullscreen mode
    if (fullscreenMode == FullscreenMode::None)
    {
        // Draw strong thick divider line between notepad and todo panes - extends all the way to the top
        // This is drawn over children to ensure it's always visible, even in the header area
        int dividerWidth = 4; // Thick divider width for strong visibility (must match resized())
        int dividerX = getWidth() / 2;
        
        // Draw a thick solid divider bar from the very top (y=0) to the bottom
        // Center the divider at the midpoint
        int dividerStartX = dividerX - dividerWidth / 2;
        int componentHeight = getHeight();
        
        // Draw the main divider with a solid, highly visible color
        // Use a bright gray/white that stands out clearly against the dark background
        juce::Rectangle<int> dividerRect(dividerStartX, 0, dividerWidth, componentHeight);
        g.setColour(juce::Colour::fromFloatRGBA(0.7f, 0.7f, 0.7f, 1.0f)); // Bright gray, fully opaque for strong visibility
        g.fillRect(dividerRect);
        
        // Add a bright white highlight on the left edge for extra visibility and definition
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawLine(static_cast<float>(dividerStartX), 0.0f, 
                   static_cast<float>(dividerStartX), static_cast<float>(componentHeight), 1.5f);
    }
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
    
    int fullscreenButtonSize = 24;
    int fullscreenButtonMargin = 5;
    
    if (fullscreenMode == FullscreenMode::Left)
    {
        // Fullscreen left pane (notepad)
        int notepadWidth = getWidth();
        
        // Position fullscreen button in top-right corner
        if (leftFullscreenButton != nullptr)
        {
            leftFullscreenButton->setBounds(getWidth() - fullscreenButtonSize - fullscreenButtonMargin, 
                                           fullscreenButtonMargin, 
                                           fullscreenButtonSize, 
                                           fullscreenButtonSize);
            leftFullscreenButton->setFullscreen(true);
            leftFullscreenButton->setVisible(true);
            leftFullscreenButton->toFront(false); // Bring button to front so it receives clicks
        }
        if (rightFullscreenButton != nullptr)
        {
            rightFullscreenButton->setVisible(false);
        }
        
        // Position text editor to fill entire width (no button space needed)
        m1TextEditor->setBounds(0, 0, notepadWidth, getHeight());
        
        // Hide todo pane components
        todoInputField->setVisible(false);
        for (int i = 0; i < todoItems.size(); ++i)
        {
            todoItems[i]->setVisible(false);
            todoLabels[i]->setVisible(false);
            if (todoEditors[i] != nullptr)
                todoEditors[i]->setVisible(false);
        }
    }
    else if (fullscreenMode == FullscreenMode::Right)
    {
        // Fullscreen right pane (todo list)
        int todoPaneX = 0;
        int todoPaneWidth = getWidth();
        int todoY = 10;
        int inputFieldHeight = 24;
        int inputFieldY = getHeight() - inputFieldHeight - 10;
        
        // Hide notepad components
        m1TextEditor->setVisible(false);
        
        // Position fullscreen button in top-right corner
        int buttonX = getWidth() - fullscreenButtonSize - fullscreenButtonMargin;
        if (rightFullscreenButton != nullptr)
        {
            rightFullscreenButton->setBounds(buttonX, 
                                            fullscreenButtonMargin, 
                                            fullscreenButtonSize, 
                                            fullscreenButtonSize);
            rightFullscreenButton->setFullscreen(true);
            rightFullscreenButton->setVisible(true);
            rightFullscreenButton->toFront(false); // Bring button to front so it receives clicks
        }
        if (leftFullscreenButton != nullptr)
        {
            leftFullscreenButton->setVisible(false);
        }
        
        // Calculate button area to avoid overlap with todo items
        // Leave space for the button plus some padding
        int buttonAreaStart = buttonX - 10; // Button area starts 10px before the button
        
        // Position todo items (but avoid button area)
        for (int i = 0; i < todoItems.size(); ++i)
        {
            int itemX = todoPaneX + 10;
            // Make sure labels don't extend into button area
            int maxItemWidth = buttonAreaStart - itemX - 30; // Leave space for checkbox (22px) and padding (8px)
            // Ensure we don't go negative or too small
            int itemWidth = juce::jmax(50, juce::jmin(todoPaneWidth - 20, maxItemWidth));
            
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
        
        // Position input field at the bottom
        int inputFieldX = todoPaneX + 10;
        int inputFieldWidth = todoPaneWidth - 20;
        todoInputField->setBounds(inputFieldX, inputFieldY, inputFieldWidth, inputFieldHeight);
        todoInputField->setVisible(true);
    }
    else
    {
        // Split view mode (default)
        // Calculate split point (vertical divider in the middle)
        int dividerX = getWidth() / 2;
        int dividerWidth = 4; // Width of the divider line (must match paintOverChildren)
        
        // Left pane: Notepad
        int notepadWidth = dividerX;
        
        // Position left fullscreen button in top-right of left pane
        if (leftFullscreenButton != nullptr)
        {
            leftFullscreenButton->setBounds(notepadWidth - fullscreenButtonSize - fullscreenButtonMargin, 
                                           fullscreenButtonMargin, 
                                           fullscreenButtonSize, 
                                           fullscreenButtonSize);
            leftFullscreenButton->setFullscreen(false);
            leftFullscreenButton->setVisible(true);
            leftFullscreenButton->toFront(false); // Bring button to front so it receives clicks
        }
        
        // Position text editor in left pane (no button space needed)
        m1TextEditor->setBounds(0, 0, notepadWidth, getHeight());
        m1TextEditor->setVisible(true);
        
        // Right pane: Todo list
        int todoPaneX = dividerX + dividerWidth;
        int todoPaneWidth = getWidth() - todoPaneX;
        int todoY = 10;
        int inputFieldHeight = 24;
        int inputFieldY = getHeight() - inputFieldHeight - 10;
        
        // Position right fullscreen button in top-right of right pane
        int buttonX = getWidth() - fullscreenButtonSize - fullscreenButtonMargin;
        if (rightFullscreenButton != nullptr)
        {
            rightFullscreenButton->setBounds(buttonX, 
                                            fullscreenButtonMargin, 
                                            fullscreenButtonSize, 
                                            fullscreenButtonSize);
            rightFullscreenButton->setFullscreen(false);
            rightFullscreenButton->setVisible(true);
            rightFullscreenButton->toFront(false); // Bring button to front so it receives clicks
        }
        
        // Calculate button area to avoid overlap with todo items
        // Leave space for the button plus some padding
        int buttonAreaStart = buttonX - 10; // Button area starts 10px before the button
        
        // Position todo items in right pane (but avoid button area)
        for (int i = 0; i < todoItems.size(); ++i)
        {
            int itemX = todoPaneX + 10;
            // Make sure labels don't extend into button area
            int maxItemWidth = buttonAreaStart - itemX - 30; // Leave space for checkbox (22px) and padding (8px)
            // Ensure we don't go negative or too small
            int itemWidth = juce::jmax(50, juce::jmin(todoPaneWidth - 20, maxItemWidth));
            
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
        todoInputField->setVisible(true);
    }
}

void NotePadAudioProcessorEditor::textEditorTextChanged (juce::TextEditor &editor)
{
    // On key changes will save editor's string to property labeled/tagged "SessionText"
    audioProcessor.treeState.state.setProperty("SessionText", editor.getText(), nullptr);
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
            
            // Update todoData to keep it in sync
            if (editingIndex < todoData.size())
            {
                todoData.getReference(editingIndex).text = text;
            }
            
            updateTodoItemsState();
            updateVisualState(); // Update visual state to refresh strikethrough
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
    // Create TodoItem and use the other addTodoItem function
    TodoItem item;
    item.text = text;
    item.completed = checked;
    item.priority = Priority::Low;
    addTodoItem(item);
}

void NotePadAudioProcessorEditor::deleteTodoItem(int index)
{
    if (index >= 0 && index < todoItems.size())
    {
        todoItems.remove(index);
        todoLabels.remove(index);
        todoEditors.remove(index);
        
        if (index < todoData.size())
            todoData.remove(index);
        
        if (editingIndex == index)
            editingIndex = -1;
        else if (editingIndex > index)
            editingIndex--;
            
        if (selectedIndex >= todoItems.size())
            selectedIndex = todoItems.size() - 1;
        else if (selectedIndex > index)
            selectedIndex--;
            
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
        if (i >= todoData.size())
            continue;
            
        const auto& item = todoData[i];
        bool isSelected = (i == selectedIndex);
        
        // Update label colors and style
        todoLabels[i]->setColour(juce::Label::textColourId,
            item.completed ? juce::Colours::grey :
            isSelected ? juce::Colours::lightblue :
            getPriorityColour(item.priority));
            
        // Set strikethrough for completed items
        todoLabels[i]->setStrikethrough(item.completed);
        
        // Set font (plain for all, strikethrough visual is handled separately)
        todoLabels[i]->setFont(juce::Font(16.0f, juce::Font::plain));
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
    else if (button == leftFullscreenButton.get())
    {
        // Toggle left pane fullscreen
        if (fullscreenMode == FullscreenMode::Left)
            toggleFullscreen(FullscreenMode::None); // Restore to split view
        else
            toggleFullscreen(FullscreenMode::Left); // Fullscreen left pane
    }
    else if (button == rightFullscreenButton.get())
    {
        // Toggle right pane fullscreen
        if (fullscreenMode == FullscreenMode::Right)
            toggleFullscreen(FullscreenMode::None); // Restore to split view
        else
            toggleFullscreen(FullscreenMode::Right); // Fullscreen right pane
    }
    else
    {
        // Check if it's one of the todo checkboxes
        for (int i = 0; i < todoItems.size(); ++i)
        {
            if (button == todoItems[i])
            {
                selectedIndex = i;
                // Update the todoData completed state to match the checkbox
                if (i < todoData.size())
                {
                    todoData.getReference(i).completed = todoItems[i]->getToggleState();
                }
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
    todoData.clear();
    
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
    // Don't handle mouse events if clicking on fullscreen buttons
    if (leftFullscreenButton != nullptr && leftFullscreenButton->isVisible() && 
        leftFullscreenButton->getBounds().contains(e.getPosition()))
        return;
    if (rightFullscreenButton != nullptr && rightFullscreenButton->isVisible() && 
        rightFullscreenButton->getBounds().contains(e.getPosition()))
        return;
    
    // Only handle double-click in todo area (right pane)
    int dividerX = (fullscreenMode == FullscreenMode::None) ? getWidth() / 2 : 0;
    if (fullscreenMode == FullscreenMode::Right || (fullscreenMode == FullscreenMode::None && e.getPosition().x > dividerX))
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
    // Don't handle mouse events if clicking on fullscreen buttons
    if (leftFullscreenButton != nullptr && leftFullscreenButton->isVisible() && 
        leftFullscreenButton->getBounds().contains(e.getPosition()))
        return;
    if (rightFullscreenButton != nullptr && rightFullscreenButton->isVisible() && 
        rightFullscreenButton->getBounds().contains(e.getPosition()))
        return;
    
    // Set up drag start index for todo item reordering
    int dividerX = (fullscreenMode == FullscreenMode::None) ? getWidth() / 2 : 0;
    if (fullscreenMode == FullscreenMode::Right || (fullscreenMode == FullscreenMode::None && e.getPosition().x > dividerX))
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
    auto* label = new StrikethroughLabel("", item.text);
    todoLabels.add(label);
    addAndMakeVisible(label);
    label->setColour(juce::Label::textColourId, getPriorityColour(item.priority));
    label->setVisible(true); // Always visible in todo area
    label->addMouseListener(this, false);
    label->setStrikethrough(item.completed);
    
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


void NotePadAudioProcessorEditor::toggleFullscreen(FullscreenMode mode)
{
    fullscreenMode = mode;
    resized();
    repaint();
}
