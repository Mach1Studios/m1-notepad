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
    };
    
    m1TextEditor->setBounds(0, 0, 800, 512 - 20);
    m1TextEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(40.0f, 40.0f, 40.0f, 0.10f));
    m1TextEditor->setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(251.0f, 251.0f, 251.0f, 1.0f));
    m1TextEditor->setColour(juce::TextEditor::highlightColourId, juce::Colour::fromFloatRGBA(242.0f, 255.0f, 95.0f, 0.25f));
    
    // Todo checkbox setup
    todoCheckbox.reset(new juce::ToggleButton("Todo Mode"));
    addAndMakeVisible(todoCheckbox.get());
    todoCheckbox->addListener(this);
    todoCheckbox->setToggleState(audioProcessor.isTodoMode(), juce::dontSendNotification);
    todoCheckbox->setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    
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
    todoInputField->setVisible(audioProcessor.isTodoMode());
    todoInputField->setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(40.0f, 40.0f, 40.0f, 0.10f));
    todoInputField->setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(251.0f, 251.0f, 251.0f, 1.0f));
    
    refreshTodoList();
    
    m1logo = juce::ImageCache::getFromMemory(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);
    
    setResizable(true, true);
    setSize(800, 512); // keep this here to not crash on scan
}

NotePadAudioProcessorEditor::~NotePadAudioProcessorEditor()
{
    m1TextEditor = nullptr;
    todoCheckbox = nullptr;
    todoInputField = nullptr;
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

void NotePadAudioProcessorEditor::resized()
{
    // Safety function to ensure we dont crash when making the window too small
    int w = getWidth();
    int h = getHeight();
    if (w < 150) w = 150; // set the minimum width
    if (h < 100) h = 100; // set the minimum height
    if (w != getWidth() || h != getHeight()) {
        setSize(w, h);
    }
    
    // Position the text editor for note mode
    m1TextEditor->setBounds(0, 0, getWidth(), getHeight() - 60);
    
    // Position the todo mode controls
    todoCheckbox->setBounds(0, getHeight() - 50, 100, 24);
    
    // Position the todo input field and items if in todo mode
    if (audioProcessor.isTodoMode())
    {
        int y = 10;
        
        // Position todo items
        for (int i = 0; i < todoItems.size(); ++i)
        {
            todoItems[i]->setBounds(10, y, 22, 20);
            
            if (todoEditors[i] != nullptr && todoEditors[i]->isVisible())
            {
                todoEditors[i]->setBounds(40, y, getWidth() - 50, 20);
            }
            else
            {
                todoLabels[i]->setBounds(40, y, getWidth() - 50, 20);
            }
            
            y += 30;
        }
        
        // Position input field at the bottom
        todoInputField->setBounds(10, y, getWidth() - 22, 24);
    }
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
    checkbox->setVisible(audioProcessor.isTodoMode());
    
    // Create label
    auto* label = new juce::Label("", text);
    todoLabels.add(label);
    addAndMakeVisible(label);
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setVisible(audioProcessor.isTodoMode());
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
    if (audioProcessor.isTodoMode())
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
        // Toggle between note mode and todo mode
        bool todoMode = todoCheckbox->getToggleState();
        audioProcessor.setTodoMode(todoMode);
        
        // Toggle visibility of UI elements based on mode
        m1TextEditor->setVisible(!todoMode);
        todoInputField->setVisible(todoMode);
        
        // Show/hide todo items based on mode
        for (auto& checkbox : todoItems)
            checkbox->setVisible(todoMode);
        for (auto& label : todoLabels)
            label->setVisible(todoMode);
            
        resized(); // Update layout
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
    for (int i = 0; i < todoLabels.size(); ++i)
    {
        if (todoLabels[i]->isVisible() && todoLabels[i]->getBounds().contains(e.getPosition()))
        {
            editTodoItem(i);
            break;
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
    checkbox->setVisible(audioProcessor.isTodoMode());
    
    // Create label with priority and category indicators
    auto* label = new juce::Label("", item.text);
    todoLabels.add(label);
    addAndMakeVisible(label);
    label->setColour(juce::Label::textColourId, getPriorityColour(item.priority));
    label->setVisible(audioProcessor.isTodoMode());
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
    if (audioProcessor.isTodoMode() && dragStartIndex >= 0)
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
                      
        todoItems[i]->setVisible(matches && audioProcessor.isTodoMode());
        todoLabels[i]->setVisible(matches && audioProcessor.isTodoMode());
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
