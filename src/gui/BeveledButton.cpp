#include "BeveledButton.h"
#include "RetroLookAndFeel.h"
#include "BitmapFont.h"

BeveledButton::BeveledButton(const juce::String& onText, const juce::String& offText)
    : onLabel(onText), offLabel(offText)
{
}

void BeveledButton::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();
    drawBevel(g, b, !state);

    // Indicator dot
    auto indicatorColour = state ? RetroColours::indicatorOn : RetroColours::indicatorOff;
    g.setColour(indicatorColour);
    g.fillRect(b.getX() + 4, b.getCentreY() - 2, 5, 5);

    // Label
    auto label = state ? onLabel : offLabel;
    BitmapFont::drawText(g, label, b.getX() + 12, b.getCentreY() - 4, 1,
                         RetroColours::textWhite);
}

void BeveledButton::mouseDown(const juce::MouseEvent&)
{
    state = !state;
    repaint();
    if (onStateChanged) onStateChanged(state);
}
