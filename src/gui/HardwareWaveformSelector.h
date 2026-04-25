#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "BeveledButton.h"

class HardwareWaveformSelector : public juce::Component
{
public:
    HardwareWaveformSelector();
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

    int getSelectedShape() const { return selectedShape; }
    void setSelectedShape(int shape);

    BeveledButton chEnv1 { "ON", "OFF" };
    BeveledButton chEnv2 { "ON", "OFF" };
    BeveledButton chEnv3 { "ON", "OFF" };

    std::function<void(int)> onShapeChanged;

private:
    int selectedShape = 0;

    // The 8 visually distinct envelope shapes (indices into the 16 hardware shapes).
    // Shapes 0-3 are identical (single decay), 4-7 repeat, 8-15 are the 8 unique ones.
    static constexpr int SHAPE_MAP[8] = { 0, 4, 8, 10, 11, 12, 13, 14 };

    void drawEnvelopeIcon(juce::Graphics& g, juce::Rectangle<int> bounds, int shapeIndex, bool selected);
};
