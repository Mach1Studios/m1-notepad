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
    
    juce__textEditor.reset (new juce::TextEditor ("new text editor"));
    addAndMakeVisible (juce__textEditor.get());
    juce__textEditor->setMultiLine (true);
    juce__textEditor->setReturnKeyStartsNewLine (true);
    juce__textEditor->setReadOnly (false);
    juce__textEditor->setScrollbarsShown (true);
    juce__textEditor->setCaretVisible (true);
    juce__textEditor->setPopupMenuEnabled (true);
    juce__textEditor->setTextToShowWhenEmpty ("Keep session notes here...", juce::Colours::white);
    juce__textEditor->focusGained(focusChangedByMouseClick);
    juce__textEditor->setText(juce::String());
    juce__textEditor->setBounds (0, 0, 800, 512);
    
    m1logo = juce::ImageCache::getFromMemory(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);

}

NotePadAudioProcessorEditor::~NotePadAudioProcessorEditor()
{
    juce__textEditor = nullptr;
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
     
     //NotePadAudioProcessor::text = juce__textEditor->getText();
 }
