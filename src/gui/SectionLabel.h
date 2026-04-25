#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SectionLabel : public juce::Component
{
public:
    explicit SectionLabel(const juce::String& title);
    void paint(juce::Graphics&) override;

private:
    juce::String title;
};
