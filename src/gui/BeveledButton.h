#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class BeveledButton : public juce::Component
{
public:
    BeveledButton(const juce::String& onText, const juce::String& offText);

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

    bool isOn() const { return state; }
    void setState(bool on) { state = on; repaint(); }

    std::function<void(bool)> onStateChanged;

private:
    juce::String onLabel, offLabel;
    bool state = false;
};
