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
 * Custom Label that supports strikethrough text
 */
class StrikethroughLabel : public juce::Label
{
public:
    StrikethroughLabel(const juce::String& name = juce::String(), 
                      const juce::String& text = juce::String()) 
        : juce::Label(name, text), hasStrikethrough(false) {}
    
    void setStrikethrough(bool strike)
    {
        if (hasStrikethrough != strike)
        {
            hasStrikethrough = strike;
            repaint();
        }
    }
    
    bool getStrikethrough() const { return hasStrikethrough; }
    
    void paintOverChildren(juce::Graphics& g) override
    {
        // Draw strikethrough line on top of the text
        if (hasStrikethrough)
        {
            auto bounds = getLocalBounds();
            auto labelFont = getFont();
            auto labelText = getText();
            
            if (labelText.isNotEmpty())
            {
                // Get the text area (accounting for border)
                auto labelBorder = getBorderSize();
                auto textArea = labelBorder.subtractedFrom(bounds).toFloat();
                
                // For left-aligned text (default for labels), calculate text position
                float textWidth = static_cast<float>(labelFont.getStringWidth(labelText));
                float textX = static_cast<float>(textArea.getX());
                float textY = static_cast<float>(textArea.getY());
                
                // Get justification to determine text position
                auto labelJustification = getJustificationType();
                if (labelJustification.testFlags(juce::Justification::right))
                {
                    textX = textArea.getRight() - textWidth;
                }
                else if (labelJustification.testFlags(juce::Justification::horizontallyCentred))
                {
                    textX = textArea.getCentreX() - textWidth / 2.0f;
                }
                
                // Calculate strikethrough Y position - centre of the text line
                // Font metrics: ascent is above baseline, descent is below
                float lineY = textY + labelFont.getHeight() / 2.0f;
                
                // Draw strikethrough line across the text width
                g.setColour(findColour(textColourId));
                g.drawLine(textX, lineY, textX + textWidth, lineY, 2.0f);
            }
        }
    }
    
private:
    bool hasStrikethrough;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StrikethroughLabel)
};

//==============================================================================
//==============================================================================
/**
 * Custom button that displays a maximize/restore icon
 */
class FullscreenButton : public juce::Button
{
public:
    FullscreenButton(const juce::String& name) : juce::Button(name), isFullscreen(false) {}
    
    void setFullscreen(bool fullscreen)
    {
        if (isFullscreen != fullscreen)
        {
            isFullscreen = fullscreen;
            repaint();
        }
    }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        
        // Draw button background
        auto bgColour = shouldDrawButtonAsHighlighted ? juce::Colour::fromFloatRGBA(0.3f, 0.3f, 0.3f, 1.0f) :
                          shouldDrawButtonAsDown ? juce::Colour::fromFloatRGBA(0.4f, 0.4f, 0.4f, 1.0f) :
                          juce::Colour::fromFloatRGBA(0.2f, 0.2f, 0.2f, 0.8f);
        
        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Draw border
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
        
        // Draw maximize or restore icon
        g.setColour(juce::Colours::white);
        float iconSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.6f;
        float iconX = bounds.getCentreX() - iconSize / 2.0f;
        float iconY = bounds.getCentreY() - iconSize / 2.0f;
        
        if (isFullscreen)
        {
            // Draw restore icon (two overlapping rectangles)
            float offset = iconSize * 0.15f;
            g.drawRect(iconX + offset, iconY + offset, iconSize * 0.4f, iconSize * 0.4f, 1.5f);
            g.drawRect(iconX, iconY, iconSize * 0.4f, iconSize * 0.4f, 1.5f);
        }
        else
        {
            // Draw maximize icon (square with diagonal corner)
            g.drawRect(iconX, iconY, iconSize * 0.7f, iconSize * 0.7f, 1.5f);
            // Draw corner accent
            float cornerSize = iconSize * 0.25f;
            g.drawLine(iconX + iconSize * 0.7f - cornerSize, iconY,
                      iconX + iconSize * 0.7f, iconY + cornerSize, 1.5f);
            g.drawLine(iconX + iconSize * 0.7f - cornerSize, iconY + iconSize * 0.7f,
                      iconX + iconSize * 0.7f, iconY + iconSize * 0.7f - cornerSize, 1.5f);
        }
    }
    
private:
    bool isFullscreen;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FullscreenButton)
};

//==============================================================================
/**
*/
class NotePadAudioProcessorEditor  : public juce::AudioProcessorEditor, 
                                    public juce::TextEditor::Listener,
                                    public juce::Button::Listener
{
public:
    enum class Priority { Low, Medium, High };
    enum class FullscreenMode { None, Left, Right };
    
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
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    
    void textEditorTextChanged (juce::TextEditor &editor) override;
    void textEditorReturnKeyPressed (juce::TextEditor &editor) override;
    void buttonClicked (juce::Button* button) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
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
    
    // Public so processor can call it before saving state
    void saveEditorStateToProcessor();
    
    std::unique_ptr<juce::TextEditor> m1TextEditor;
    std::unique_ptr<juce::ToggleButton> todoCheckbox;
    std::unique_ptr<juce::TextEditor> todoInputField;
    std::unique_ptr<juce::ComboBox> priorityCombo;
    std::unique_ptr<juce::TextEditor> searchField;
    std::unique_ptr<FullscreenButton> leftFullscreenButton;
    std::unique_ptr<FullscreenButton> rightFullscreenButton;
    juce::OwnedArray<juce::ToggleButton> todoItems;
    juce::OwnedArray<StrikethroughLabel> todoLabels;
    juce::OwnedArray<juce::TextEditor> todoEditors;
    juce::Array<TodoItem> todoData;
    
    int editingIndex = -1;
    int selectedIndex = -1;
    int dragStartIndex = -1;
    bool isDragging = false;
    FullscreenMode fullscreenMode = FullscreenMode::None;
    

private:
    NotePadAudioProcessor& audioProcessor;
    juce::Image m1logo;
    
    void updatePriorityColors();
    juce::Colour getPriorityColour(Priority p) const;
    void toggleFullscreen(FullscreenMode mode);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotePadAudioProcessorEditor)
};
