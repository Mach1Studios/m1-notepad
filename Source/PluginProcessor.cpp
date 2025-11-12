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
                       ),
treeState (*this, nullptr /* undomanager */, "TreeState", {std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("gain", 1), "Gain", -48.0f, 0.0f, -15.0f) })
#endif
{
    // Initialize default properties only if they don't exist
    // (They will be loaded from saved state if available via setStateInformation)
    if (!treeState.state.hasProperty("SessionText"))
        treeState.state.setProperty("SessionText", "", nullptr);
    
    // Create a treestate child node to store todo items (if it doesn't exist)
    treeState.state.getOrCreateChildWithName("TodoItems", nullptr);
    
    // Initialize todo mode property (only if it doesn't exist)
    if (!treeState.state.hasProperty("TodoMode"))
        treeState.state.setProperty("TodoMode", false, nullptr);
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
}

void NotePadAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NotePadAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    if (layouts.getMainInputChannelSet()  == juce::AudioChannelSet::disabled()
     || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;
 
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::createLCR().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::quadraphonic().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create5point0().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create5point1().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create7point0().size()
        && layouts.getMainOutputChannelSet().size() != juce::AudioChannelSet::create7point1().size())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
  #endif
}
#endif

void NotePadAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    bool audioPassThrough = isAudioPassThrough();

    if (!audioPassThrough)
    {
        // Mute: clear all output channels
        buffer.clear();
    }
    // If audioPassThrough is true, the audio already passes through (input is already in output channels)
    
    // In case we have more outputs than inputs, clear any output
    // channels that didn't contain input data (only needed when pass-through is enabled)
    if (audioPassThrough)
    {
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        {
            buffer.clear (i, 0, buffer.getNumSamples());
        }
    }
}

//==============================================================================
bool NotePadAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NotePadAudioProcessor::createEditor()
{
    NotePadAudioProcessorEditor* editor = new NotePadAudioProcessorEditor (*this);
    currentEditor = editor;
    return editor;
}

//==============================================================================
void NotePadAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // If editor exists, save its current state before saving
    // This ensures we capture the latest changes even if the editor hasn't been destroyed yet
    if (currentEditor != nullptr)
    {
        currentEditor->saveEditorStateToProcessor();
    }
    
    // Save the entire tree state to include todo items and session text
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NotePadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Load the entire tree state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
    {
        // Try to restore the state from XML - don't check tag name as it might vary
        juce::ValueTree newState = juce::ValueTree::fromXml(*xmlState);
        if (newState.isValid())
        {
            treeState.replaceState(newState);
            
            // Ensure required properties and child nodes exist after loading
            if (!treeState.state.hasProperty("SessionText"))
                treeState.state.setProperty("SessionText", "", nullptr);
            treeState.state.getOrCreateChildWithName("TodoItems", nullptr);
            if (!treeState.state.hasProperty("TodoMode"))
                treeState.state.setProperty("TodoMode", false, nullptr);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NotePadAudioProcessor();
}
