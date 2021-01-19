/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NotePadAudioProcessor::NotePadAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}


NotePadAudioProcessor::~NotePadAudioProcessor()
{
}

//==============================================================================
const juce::String NotePadAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NotePadAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NotePadAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NotePadAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NotePadAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NotePadAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NotePadAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NotePadAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NotePadAudioProcessor::getProgramName (int index)
{
    return {};
}

void NotePadAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NotePadAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NotePadAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NotePadAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::createLCR().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::quadraphonic().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create5point0().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create5point1().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create7point0().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create7point1().size())
        return false;

    return true;
  #endif
}
#endif

void NotePadAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //TODO: bypass processing
}

//==============================================================================
bool NotePadAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NotePadAudioProcessor::createEditor()
{
    editor =  new NotePadAudioProcessorEditor (*this);
    return editor;
}

//==============================================================================
void NotePadAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Save sessionText as raw string
    juce::MemoryOutputStream(destData,true).writeString(editor->m1TextEditor->getText());
}

void NotePadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Load sessionText as raw string
    editor->m1TextEditor->setText(juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readString());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NotePadAudioProcessor();
}
