/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
// Forward declaration
class NotePadAudioProcessorEditor;

//==============================================================================
/**
*/

class NotePadAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NotePadAudioProcessor();
    ~NotePadAudioProcessor() override;
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState treeState;
    juce::String pSessionText; // Use this to check and debug text passed back to Processor
    
    // Todo functionality helpers
    bool isTodoMode() const { return treeState.state.getProperty("TodoMode", false); }
    void setTodoMode(bool todoMode) { treeState.state.setProperty("TodoMode", todoMode, nullptr); }

     // Pass through mode - always enabled
     bool isAudioPassThrough() const { return true; }
     
     // Store pointer to editor to save state before getStateInformation is called
     NotePadAudioProcessorEditor* currentEditor = nullptr;
     
     void setEditor(NotePadAudioProcessorEditor* editor) { currentEditor = editor; }

private:
    //==============================================================================    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotePadAudioProcessor)
};