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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 512);
    
    m1TextEditor.reset (new juce::TextEditor ("new text editor"));
    addAndMakeVisible (m1TextEditor.get());
    m1TextEditor->setMultiLine (true);
    m1TextEditor->setReturnKeyStartsNewLine (true);
    m1TextEditor->setReadOnly (false);
    m1TextEditor->setScrollbarsShown (true);
    m1TextEditor->setCaretVisible (true);
    m1TextEditor->setPopupMenuEnabled (true);
    m1TextEditor->setTextToShowWhenEmpty ("Keep session notes here...", juce::Colours::white);
    m1TextEditor->focusGained(focusChangedByMouseClick);
    m1TextEditor->setText(juce::String());
    m1TextEditor->setBounds (0, 0, 800, 512);
    
    m1logo = juce::ImageCache::getFromMemory(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);

}

NotePadAudioProcessorEditor::~NotePadAudioProcessorEditor()
{
    m1TextEditor = nullptr;
}

//==============================================================================
void NotePadAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(40, 40, 40));

    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white);
    
    //m1logo
    g.drawImageWithin(m1logo, getWidth() - 80, getHeight() - 17, 161 / 2, 39 / 4, juce::RectanglePlacement());
}

void NotePadAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

 void NotePadAudioProcessorEditor::textEditorTextChanged (juce::TextEditor &editor)
 {
     
     
 }
