/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

//class ValueTreeItem  : public juce::TreeViewItem,
//private juce::ValueTree::Listener
//{
//public:
//    ValueTreeItem (const juce::ValueTree& v, juce::UndoManager& um)
//        : tree (v), undoManager (um)
//    {
//        tree.addListener (this);
//    }
//
//private:
//    juce::ValueTree tree;
//    juce::UndoManager& undoManager;
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeItem)
//};

//==============================================================================

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
    juce::String pSessionText;

private:
    //==============================================================================
//    juce::TreeView tree;
//    std::unique_ptr<ValueTreeItem> rootItem;
//    juce::UndoManager undoManager;
//
//    void timerCallback()
//    {
//        undoManager.beginNewTransaction();
//    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotePadAudioProcessor)
};
