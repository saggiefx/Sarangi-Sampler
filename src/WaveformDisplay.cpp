

#include <JuceHeader.h>
#include "WaveformDisplay.h"

//==============================================================================
WaveformDisplay::WaveformDisplay(SimpleSamplerAudioProcessor& p) : processor(p)
{

}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint (Graphics& g)
{
    
    g.fillAll(Colours::gainsboro);
    
    auto waveform = processor.getWaveform();
    
    // TODO: only paint if waveform changes.
    if (waveform.getNumSamples() > 0)
    {
        mLoadButton.setVisible(false);
        
        Path path;
        mAudioPoints.clear();
        
        
        auto waveform = processor.getWaveform();
        auto xRatio = waveform.getNumSamples() / getWidth();
        auto buffer = waveform.getReadPointer(0);
        
        // scale on x axis
        for (int sample = 0; sample < waveform.getNumSamples(); sample+=xRatio)
        {
            mAudioPoints.push_back(buffer[sample]);
        }
        
        path.startNewSubPath(0, getHeight()/2);
        
        // scale on y axis
        for (int sample = 0; sample < mAudioPoints.size(); ++sample)
        {
            auto point = jmap<float> (mAudioPoints[sample], -1.0f, 1.0f, getHeight(), 0);
            path.lineTo(sample, point);
        }
        
        auto roundedCornerPath = path.createPathWithRoundedCorners(1);
        g.strokePath(roundedCornerPath, PathStrokeType(2));
        
        g.setColour(Colours::white);
        
        auto textBounds = getLocalBounds().reduced (16, 16);
        g.drawFittedText(mFilename, textBounds, Justification::topRight, 1);
        
        auto playheadPosition = jmap<int> (processor.getSampleCount(), 0, processor.getWaveform().getNumSamples(), 0, getWidth());
        
        g.setColour(Colours::white);
        g.drawLine(playheadPosition, 0, playheadPosition, getHeight());
        
        g.setColour(Colours::black.withAlpha(0.2f));
        g.fillRect(0, 0, playheadPosition, getHeight());

    } else {
        g.setColour(Colours::white);
        g.setFont(18.0f);
        g.drawFittedText("Start Sampler", getLocalBounds(), Justification::centred, 1);
        mLoadButton.setBoundsRelative(0.25f, 0.7f, 0.5f, 0.2f);
        mLoadButton.setVisible(true);
    }
}

void WaveformDisplay::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}


