#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

class ScopeDisplay : public juce::Component, private juce::Timer
{
public:
    static constexpr int BUFFER_SIZE = 256;

    ScopeDisplay();
    ~ScopeDisplay() override;

    void paint(juce::Graphics&) override;
    void pushSample(float sample);

private:
    void timerCallback() override;
    std::array<float, BUFFER_SIZE> buffer {};
    int writeIndex = 0;
};
