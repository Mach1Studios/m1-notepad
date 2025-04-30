/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NotePadAudioProcessorEditor  : public juce::AudioProcessorEditor, 
                                    public juce::TextEditor::Listener,
                                    public juce::Button::Listener
{
public:
    enum class Priority { Low, Medium, High };
    
    struct TodoItem
    {
        juce::String text;
        bool completed;
        Priority priority;
        juce::Time dueDate;
        juce::String tags;
    };
    
    NotePadAudioProcessorEditor (NotePadAudioProcessor&);
    ~NotePadAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void textEditorTextChanged (juce::TextEditor &editor) override;
    void textEditorReturnKeyPressed (juce::TextEditor &editor) override;
    void buttonClicked (juce::Button* button) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;
    void addTodoItem(const TodoItem& item);
    void addTodoItem(const juce::String& text, bool checked);
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void refreshTodoList();
    void updateTodoItemsState();
    void editTodoItem(int index);
    void deleteTodoItem(int index);
    void moveSelection(int delta);
    void updateVisualState();
    void reorderItems(int fromIndex, int toIndex);
    void filterItems(const juce::String& searchText);
    void exportTodoList();
    void importTodoList();
    
    std::unique_ptr<juce::TextEditor> m1TextEditor;
    std::unique_ptr<juce::ToggleButton> todoCheckbox;
    std::unique_ptr<juce::TextEditor> todoInputField;
    std::unique_ptr<juce::ComboBox> priorityCombo;
    std::unique_ptr<juce::TextEditor> searchField;
    juce::OwnedArray<juce::ToggleButton> todoItems;
    juce::OwnedArray<juce::Label> todoLabels;
    juce::OwnedArray<juce::TextEditor> todoEditors;
    juce::Array<TodoItem> todoData;
    
    int editingIndex = -1;
    int selectedIndex = -1;
    int dragStartIndex = -1;
    bool isDragging = false;
    

private:
    NotePadAudioProcessor& audioProcessor;
    juce::Image m1logo;
    
    void updatePriorityColors();
    juce::Colour getPriorityColour(Priority p) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotePadAudioProcessorEditor)
};
