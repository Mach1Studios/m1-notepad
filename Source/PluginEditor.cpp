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
    m1TextEditor->addListener(this);
    m1TextEditor->setMultiLine (true);
    m1TextEditor->setReturnKeyStartsNewLine (true);
    m1TextEditor->setReadOnly (false);
    m1TextEditor->setScrollbarsShown (true);
    m1TextEditor->setCaretVisible (true);
    m1TextEditor->setPopupMenuEnabled (true);
    m1TextEditor->setTabKeyUsedAsCharacter(true);
    m1TextEditor->setTextToShowWhenEmpty ("Keep session notes here...", juce::Colours::white);
    m1TextEditor->setText(audioProcessor.treeState.state.getProperty("SessionText")); // Grabs the string within property labeled "SessionText"
    m1TextEditor->setBounds (0, 0, 800, 512);
    m1TextEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(40.0f, 40.0f, 40.0f, 0.10f));
    m1TextEditor->setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(251.0f, 251.0f, 251.0f, 1.0f));
    m1TextEditor->setColour(juce::TextEditor::highlightColourId, juce::Colour::fromFloatRGBA(242.0f, 255.0f, 95.0f, 0.25f));
    
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
//    textEditor.applyColourToAllText(getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId), true);
    
    
    //m1logo
    g.drawImageWithin(m1logo, -15, getHeight() - 17, 161 / 2, 39 / 4, juce::RectanglePlacement());
}

void NotePadAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

 void NotePadAudioProcessorEditor::textEditorTextChanged (juce::TextEditor &editor)
 {
     // On key changes will save editor's string to property labeled/tagged "SessionText"
     audioProcessor.treeState.state.setProperty("SessionText", editor.getText(), nullptr);
 }
