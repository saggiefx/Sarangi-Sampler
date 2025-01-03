#include "PluginProcessor.h"
#include "PluginEditor.h"


// Assign binary data for each note
const char* gMajorABinaryData = BinaryData::A_wav;
const size_t gMajorABinaryDataSize = BinaryData::A_wavSize;

const char* gMajorBBinaryData = BinaryData::B_wav;
const size_t gMajorBBinaryDataSize = BinaryData::B_wavSize;

const char* gMajorCBinaryData = BinaryData::C_wav;
const size_t gMajorCBinaryDataSize = BinaryData::C_wavSize;

const char* gMajorDBinaryData = BinaryData::D_wav;
const size_t gMajorDBinaryDataSize = BinaryData::D_wavSize;

const char* gMajorEBinaryData = BinaryData::E_wav;
const size_t gMajorEBinaryDataSize = BinaryData::E_wavSize;

const char* gMajorFSharpBinaryData = BinaryData::F_wav;
const size_t gMajorFSharpBinaryDataSize = BinaryData::F_wavSize;

const char* gMajorGBinaryData = BinaryData::G_wav;
const size_t gMajorGBinaryDataSize = BinaryData::G_wavSize;



//==============================================================================
SimpleSamplerAudioProcessor::SimpleSamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ), mAPVTS(*this, nullptr, "PARAMETERS", createParameters())
#endif
{
    mFormatManager.registerBasicFormats();
    mAPVTS.state.addListener (this);
    
    for (int i = 0; i < mNumVoices; i++){
        mSampler.addVoice(new SamplerVoice());
    }
}

SimpleSamplerAudioProcessor::~SimpleSamplerAudioProcessor()
{
}

//==============================================================================
const String SimpleSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleSamplerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSamplerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSamplerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleSamplerAudioProcessor::getNumPrograms()
{
    return 1;
}

int SimpleSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleSamplerAudioProcessor::setCurrentProgram (int index)
{
}

const String SimpleSamplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleSamplerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SimpleSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
    
    // Load samples for all notes
    loadNoteSamples();
}

void SimpleSamplerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleSamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleSamplerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (mShouldUpdate) {
      updateADSR();
    }
    
    MidiMessage m;
    MidiBuffer::Iterator iterator { midiMessages };
    int sample;
    
    while (iterator.getNextEvent(m, sample))
    {
        if (m.isNoteOn()){
            mIsNotePlayed = true;
        }
        else if (m.isNoteOff())
        {
            mIsNotePlayed = false;
        }
    }
    
    mSampleCount = mIsNotePlayed ? mSampleCount += buffer.getNumSamples() : 0;

    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool SimpleSamplerAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* SimpleSamplerAudioProcessor::createEditor()
{
    return new SimpleSamplerAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleSamplerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


void SimpleSamplerAudioProcessor::updateADSR()
{
    mADSRParams.attack = mAPVTS.getRawParameterValue("ATTACK")->load();
    mADSRParams.decay = mAPVTS.getRawParameterValue("DECAY")->load();
    mADSRParams.sustain = mAPVTS.getRawParameterValue("SUSTAIN")->load();
    mADSRParams.release = mAPVTS.getRawParameterValue("RELEASE")->load();
    
    for (int i = 0; i < mSampler.getNumSounds(); i++)
    {
        if (auto sound = dynamic_cast<SamplerSound*>(mSampler.getSound(i).get()))
        {
            sound->setEnvelopeParameters (mADSRParams);
        }
    }
    DBG ("A: " << attack << " D: " << decay << " S: " << sustain << " R: " << release);
}

AudioProcessorValueTreeState::ParameterLayout SimpleSamplerAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
    
    parameters.push_back (std::make_unique<AudioParameterFloat> ("ATTACK", "Attack", 0.001f, 5.0f, 0.001f));
    parameters.push_back (std::make_unique<AudioParameterFloat> ("DECAY", "Decay", 0.001f, 3.0f, 0.001f));
    parameters.push_back (std::make_unique<AudioParameterFloat> ("SUSTAIN", "Sustain", 0.001f, 1.0f, 1.0f));
    parameters.push_back (std::make_unique<AudioParameterFloat> ("RELEASE", "Release", 0.001f, 5.0f, 0.001f));
    
    return { parameters.begin(), parameters.end() };
}

void SimpleSamplerAudioProcessor::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    mShouldUpdate = true;
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSamplerAudioProcessor();
}
