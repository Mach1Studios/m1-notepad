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
class NotePadAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::TextEditor::Listener
{
public:
    NotePadAudioProcessorEditor (NotePadAudioProcessor&);
    ~NotePadAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void textEditorTextChanged (juce::TextEditor &editor) override;
    std::unique_ptr<juce::TextEditor> m1TextEditor;

private:
    NotePadAudioProcessor& audioProcessor;
    juce::Image m1logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotePadAudioProcessorEditor)
};
