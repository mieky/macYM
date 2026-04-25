#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

class WaveformEditor : public juce::Component
{
public:
    static constexpr int NUM_CELLS = 32;
    static constexpr int MAX_VALUE = 15;

    WaveformEditor();

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;

    int getValue(int index) const;
    void setValue(int index, int value);
    const std::array<int, NUM_CELLS>& getData() const { return data; }

    std::function<void(int, int)> onValueChanged;

private:
    std::array<int, NUM_CELLS> data;
    void setCellFromMouse(const juce::MouseEvent& e);
};
