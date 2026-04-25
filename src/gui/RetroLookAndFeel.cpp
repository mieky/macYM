#include "RetroLookAndFeel.h"

void drawBevel(juce::Graphics& g, juce::Rectangle<int> b, bool raised)
{
    auto light = raised ? RetroColours::bevelLight : RetroColours::bevelDark;
    auto dark  = raised ? RetroColours::bevelDark  : RetroColours::bevelLight;

    // Fill
    g.setColour(RetroColours::panelBg);
    g.fillRect(b);

    // Top and left edges (light)
    g.setColour(light);
    g.drawHorizontalLine(b.getY(), (float)b.getX(), (float)b.getRight());
    g.drawVerticalLine(b.getX(), (float)b.getY(), (float)b.getBottom());

    // Bottom and right edges (dark)
    g.setColour(dark);
    g.drawHorizontalLine(b.getBottom() - 1, (float)b.getX(), (float)b.getRight());
    g.drawVerticalLine(b.getRight() - 1, (float)b.getY(), (float)b.getBottom());
}

void drawInset(juce::Graphics& g, juce::Rectangle<int> b)
{
    drawBevel(g, b, false);
}
