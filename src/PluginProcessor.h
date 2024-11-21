#pragma once

#include <JuceHeader.h>


extern const char* gMajorABinaryData;
extern const size_t gMajorABinaryDataSize;

extern const char* gMajorBBinaryData;
extern const size_t gMajorBBinaryDataSize;

extern const char* gMajorCBinaryData;
extern const size_t gMajorCBinaryDataSize;

extern const char* gMajorDBinaryData;
extern const size_t gMajorDBinaryDataSize;

extern const char* gMajorEBinaryData;
extern const size_t gMajorEBinaryDataSize;

extern const char* gMajorFSharpBinaryData;
extern const size_t gMajorFSharpBinaryDataSize;

extern const char* gMajorGBinaryData;
extern const size_t gMajorGBinaryDataSize;


class SimpleSamplerAudioProcessor  : public AudioProcessor,
                                     public ValueTree::Listener
{
public:
    //==============================================================================
    SimpleSamplerAudioProcessor();
    ~SimpleSamplerAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    int getNumSamplerSounds() { return mSampler.getNumSounds(); }
    AudioBuffer<float>& getWaveform() { return mWaveform; }

    void loadNoteSamples()
    {
        std::map<int, std::pair<const void*, size_t>> noteToBinaryData = {
            {69, {gMajorABinaryData, gMajorABinaryDataSize}}, // A
            {71, {gMajorBBinaryData, gMajorBBinaryDataSize}}, // B
            {72, {gMajorCBinaryData, gMajorCBinaryDataSize}}, // C
            {74, {gMajorDBinaryData, gMajorDBinaryDataSize}}, // D
            {76, {gMajorEBinaryData, gMajorEBinaryDataSize}}, // E
            {78, {gMajorFSharpBinaryData, gMajorFSharpBinaryDataSize}}, // F#
            {79, {gMajorGBinaryData, gMajorGBinaryDataSize}}, // G
        };

        // Clear previous sounds
        mSampler.clearSounds();

        for (const auto& [note, binaryData] : noteToBinaryData)
        {
            const auto& [data, size] = binaryData;

            // Create a MemoryInputStream for the binary data
            auto memoryStream = std::make_unique<juce::MemoryInputStream>(data, size, false);

            // Create a format reader for the binary stream
            if (auto formatReader = mFormatManager.createReaderFor(std::move(memoryStream)))
            {
                BigInteger range;
                range.setBit(note); // Assign this sound to the specific note

                // Add the sampler sound for this note
                mSampler.addSound(new SamplerSound("Sample", *formatReader, range, note, 0.01, 0.01, 360));
            }
            else
            {
                DBG("Failed to create format reader for note: " + juce::String(note));
            }
        }

        updateADSR();
    }

    
    void updateADSR();
    
    float attack { 0.0 };
    float decay { 0.0 };
    float sustain { 1.0 };
    float release { 0.0 };
    
    ADSR::Parameters& getADSRParams() { return mADSRParams; };
    
    AudioProcessorValueTreeState& getAPVTS() { return mAPVTS; }
    
    std::atomic<bool>& isNotePlayed() { return mIsNotePlayed; }
    std::atomic<int>& getSampleCount() { return mSampleCount; }
    
private:
    
    Synthesiser mSampler;
    const int mNumVoices { 8 };
    
    AudioBuffer<float> mWaveform;
    
    ADSR::Parameters mADSRParams;
    
    AudioFormatManager mFormatManager;
    AudioFormatReader* mFormatReader { nullptr };
    
    AudioProcessorValueTreeState mAPVTS;
    AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property ) override;
    
    std::atomic<bool> mShouldUpdate { false };
    std::atomic<bool> mIsNotePlayed { false };
    std::atomic<int> mSampleCount { 0 };

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSamplerAudioProcessor)
};
