#include "SpinnerControl.h"
#include "RetroLookAndFeel.h"
#include "BitmapFont.h"

SpinnerControl::SpinnerControl(int minVal, int maxVal, int defaultVal)
    : value(defaultVal), minValue(minVal), maxValue(maxVal)
{
}

void SpinnerControl::resized()
{
    auto b = getLocalBounds();
    int arrowW = 14;
    leftArrow  = b.removeFromLeft(arrowW);
    rightArrow = b.removeFromRight(arrowW);
    display    = b;
}

void SpinnerControl::paint(juce::Graphics& g)
{
    // Arrow buttons
    drawBevel(g, leftArrow, true);
    drawBevel(g, rightArrow, true);
    BitmapFont::drawText(g, "<", leftArrow.getX() + 3, leftArrow.getCentreY() - 4, 1,
                         RetroColours::textWhite);
    BitmapFont::drawText(g, ">", rightArrow.getX() + 3, rightArrow.getCentreY() - 4, 1,
                         RetroColours::textWhite);

    // Value display (inset)
    drawInset(g, display);
    auto valStr = juce::String(value);
    int textW = BitmapFont::textWidth(valStr, 1);
    BitmapFont::drawText(g, valStr, display.getCentreX() - textW / 2,
                         display.getCentreY() - 4, 1, RetroColours::textCyan);
}

void SpinnerControl::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();
    int step = e.mods.isRightButtonDown() ? 10 : 1;
    if (leftArrow.contains(pos))
        setValue(value - step);
    else if (rightArrow.contains(pos))
        setValue(value + step);
}

void SpinnerControl::setValue(int v)
{
    int clamped = juce::jlimit(minValue, maxValue, v);
    if (clamped != value)
    {
        value = clamped;
        repaint();
        if (onValueChanged) onValueChanged(value);
    }
}
