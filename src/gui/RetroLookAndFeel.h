#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace RetroColours
{
    const juce::Colour background   { 0xff7b7b9b };
    const juce::Colour panelBg      { 0xff6b6b8b };
    const juce::Colour bevelLight   { 0xffa0a0c0 };
    const juce::Colour bevelDark    { 0xff505070 };
    const juce::Colour textCyan     { 0xff00e0e0 };
    const juce::Colour textWhite    { 0xffe0e0e0 };
    const juce::Colour textBlack    { 0xff202020 };
    const juce::Colour indicatorOn  { 0xffff40ff };
    const juce::Colour indicatorOff { 0xff606080 };
    const juce::Colour scopeGreen   { 0xff00ff40 };
    const juce::Colour scopeBg      { 0xff102010 };
    const juce::Colour gridCyan     { 0xff00c0c0 };
    const juce::Colour gridBg       { 0xff183038 };
}

// Draw a GEM-style 3D beveled rectangle.
void drawBevel(juce::Graphics& g, juce::Rectangle<int> bounds, bool raised = true);

// Draw a GEM-style inset (sunken) panel.
void drawInset(juce::Graphics& g, juce::Rectangle<int> bounds);
