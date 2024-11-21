
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class WaveformDisplay    : public Component
{
public:
    WaveformDisplay(SimpleSamplerAudioProcessor& p);
    ~WaveformDisplay();

    void paint (Graphics&) override;
    void resized() override;

private:
    
    TextButton mLoadButton { "" };
    
    std::vector<float> mAudioPoints;
    
    SimpleSamplerAudioProcessor& processor;
    
    String mFilename { "" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
