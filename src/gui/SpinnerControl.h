#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SpinnerControl : public juce::Component
{
public:
    SpinnerControl(int minVal, int maxVal, int defaultVal);

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

    int getValue() const { return value; }
    void setValue(int v);

    std::function<void(int)> onValueChanged;

private:
    int value, minValue, maxValue;
    juce::Rectangle<int> leftArrow, rightArrow, display;
};
