
#include <JuceHeader.h>
#include "LogoComponent.h"

//==============================================================================
LogoComponent::LogoComponent()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

LogoComponent::~LogoComponent()
{
}

void LogoComponent::paint (Graphics& g)
{
    g.setColour (Colours::grey);
    g.setFont (18.0f);
    g.drawText ("Simple Sampler", getLocalBounds().reduced(16, 16),
                Justification::centredLeft, true);
}

void LogoComponent::resized()
{
}
