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
 * Custom TextEditor with strikethrough support
 */
class StrikethroughTextEditor : public juce::TextEditor
{
public:
    std::function<void()> onStrikethroughChanged;
    
    StrikethroughTextEditor(const juce::String& name = juce::String()) : juce::TextEditor(name) 
    {
        setPopupMenuEnabled(true); // Enable popup menu so right-click works
    }
    
    void addStrikethrough(juce::Range<int> range)
    {
        if (range.getLength() <= 0)
            return;
            
        // Merge with overlapping ranges
        bool merged = false;
        for (int i = 0; i < strikethroughRanges.size(); ++i)
        {
            if (strikethroughRanges[i].intersects(range) || 
                strikethroughRanges[i].getEnd() == range.getStart() ||
                strikethroughRanges[i].getStart() == range.getEnd())
            {
                strikethroughRanges.set(i, strikethroughRanges[i].getUnionWith(range));
                merged = true;
                break;
            }
        }
        
        if (!merged)
            strikethroughRanges.add(range);
            
        repaint();
        if (onStrikethroughChanged)
            onStrikethroughChanged();
    }
    
    void removeStrikethrough(juce::Range<int> range)
    {
        bool changed = false;
        for (int i = strikethroughRanges.size() - 1; i >= 0; --i)
        {
            if (strikethroughRanges[i].intersects(range))
            {
                auto existing = strikethroughRanges[i];
                strikethroughRanges.remove(i);
                changed = true;
                
                // Add back the parts that don't overlap
                if (existing.getStart() < range.getStart())
                    strikethroughRanges.add(juce::Range<int>(existing.getStart(), range.getStart()));
                if (existing.getEnd() > range.getEnd())
                    strikethroughRanges.add(juce::Range<int>(range.getEnd(), existing.getEnd()));
            }
        }
        if (changed)
        {
            repaint();
            if (onStrikethroughChanged)
                onStrikethroughChanged();
        }
    }
    
    void toggleStrikethrough(juce::Range<int> range)
    {
        if (range.getLength() <= 0)
            return;
            
        // Check if the entire range is strikethrough
        bool allStrikethrough = true;
        for (int i = range.getStart(); i < range.getEnd(); ++i)
        {
            bool hasStrike = false;
            for (auto& r : strikethroughRanges)
            {
                if (r.contains(i))
                {
                    hasStrike = true;
                    break;
                }
            }
            if (!hasStrike)
            {
                allStrikethrough = false;
                break;
            }
        }
        
        if (allStrikethrough)
            removeStrikethrough(range);
        else
            addStrikethrough(range);
    }
    
    bool hasStrikethrough(int position) const
    {
        for (auto& range : strikethroughRanges)
        {
            if (range.contains(position))
                return true;
        }
        return false;
    }
    
    juce::Array<juce::Range<int>> getStrikethroughRanges() const { return strikethroughRanges; }
    
    void setStrikethroughRanges(const juce::Array<juce::Range<int>>& ranges)
    {
        strikethroughRanges = ranges;
        repaint();
    }
    
    // Serialize strikethrough ranges to a string
    juce::String serializeStrikethroughRanges() const
    {
        juce::String result;
        for (auto& range : strikethroughRanges)
        {
            if (result.isNotEmpty())
                result += ";";
            result += juce::String(range.getStart()) + "," + juce::String(range.getEnd());
        }
        return result;
    }
    
    // Deserialize strikethrough ranges from a string
    void deserializeStrikethroughRanges(const juce::String& data)
    {
        strikethroughRanges.clear();
        if (data.isEmpty())
            return;
            
        juce::StringArray ranges;
        ranges.addTokens(data, ";", "");
        for (auto& rangeStr : ranges)
        {
            juce::StringArray parts;
            parts.addTokens(rangeStr, ",", "");
            if (parts.size() == 2)
            {
                int start = parts[0].getIntValue();
                int end = parts[1].getIntValue();
                if (start >= 0 && end > start)
                    strikethroughRanges.add(juce::Range<int>(start, end));
            }
        }
        repaint();
    }
    
    void clearStrikethrough()
    {
        if (!strikethroughRanges.isEmpty())
        {
            strikethroughRanges.clear();
            repaint();
            if (onStrikethroughChanged)
                onStrikethroughChanged();
        }
    }
    
    // Adjust strikethrough ranges when text is inserted/deleted
    void adjustStrikethroughRanges()
    {
        // Validate and adjust ranges to ensure they're within bounds
        // Note: This doesn't track exact insertion/deletion points, but ensures
        // ranges remain valid after text modifications
        const int totalChars = getTotalNumChars();
        
        for (int i = strikethroughRanges.size() - 1; i >= 0; --i)
        {
            auto& range = strikethroughRanges.getReference(i);
            
            // Remove ranges that are completely out of bounds
            if (range.getStart() >= totalChars)
            {
                strikethroughRanges.remove(i);
                continue;
            }
            
            // Clamp ranges that extend beyond the text
            if (range.getEnd() > totalChars)
            {
                range = juce::Range<int>(range.getStart(), totalChars);
            }
            
            // Remove empty or invalid ranges
            if (range.getLength() <= 0 || range.getStart() < 0)
            {
                strikethroughRanges.remove(i);
            }
        }
        
        // Notify that strikethrough may have changed
        if (onStrikethroughChanged)
            onStrikethroughChanged();
            
        repaint();
    }
    
    void addPopupMenuItems(juce::PopupMenu& menuToAddTo,
                          const juce::MouseEvent* mouseClickEvent) override
    {
        juce::TextEditor::addPopupMenuItems(menuToAddTo, mouseClickEvent);
        
        auto selRange = getHighlightedRegion();
        bool hasSelection = selRange.getLength() > 0;
        
        menuToAddTo.addSeparator();
        menuToAddTo.addItem(1000, "Strikethrough", hasSelection && !isReadOnly());
    }
    
    void performPopupMenuAction(int menuItemID) override
    {
        if (menuItemID == 1000)
        {
            auto selRange = getHighlightedRegion();
            if (selRange.getLength() > 0)
                toggleStrikethrough(selRange);
        }
        else
        {
            juce::TextEditor::performPopupMenuAction(menuItemID);
        }
    }
    
protected:
    void paintOverChildren(juce::Graphics& g) override
    {
        juce::TextEditor::paintOverChildren(g);
        
        // Draw strikethrough lines over the text content
        if (strikethroughRanges.isEmpty())
            return;
            
        drawStrikethrough(g);
    }
    
private:
    void drawStrikethrough(juce::Graphics& g)
    {
        if (strikethroughRanges.isEmpty())
            return;
            
        auto textColour = findColour(textColourId);
        g.setColour(textColour);
        
        // Use TextEditor's getTextBounds to get accurate text positions
        for (auto& range : strikethroughRanges)
        {
            if (range.getStart() >= getTotalNumChars() || range.getLength() <= 0)
                continue;
                
            // Clamp range to valid text range
            auto clampedRange = range.getIntersectionWith(juce::Range<int>(0, getTotalNumChars()));
            if (clampedRange.isEmpty())
                continue;
                
            // Get the text bounds for this range (handles multiline and word wrap)
            auto textBounds = getTextBounds(clampedRange);
            
            // Draw strikethrough line for each rectangle in the bounds
            for (const auto& rect : textBounds)
            {
                auto font = getFont();
                float strikethroughY = static_cast<float>(rect.getCentreY());
                float x1 = static_cast<float>(rect.getX());
                float x2 = static_cast<float>(rect.getRight());
                
                // Draw the strikethrough line
                g.drawLine(x1, strikethroughY, x2, strikethroughY, 1.5f);
            }
        }
    }
    
    juce::Array<juce::Range<int>> strikethroughRanges;
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
    
    std::unique_ptr<StrikethroughTextEditor> m1TextEditor;
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
