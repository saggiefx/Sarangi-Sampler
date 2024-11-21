

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class LogoComponent    : public Component
{
public:
    LogoComponent();
    ~LogoComponent();

    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LogoComponent)
};
