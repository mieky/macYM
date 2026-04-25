#pragma once
#include <juce_graphics/juce_graphics.h>

class BitmapFont
{
public:
    static constexpr int CHAR_W = 8;
    static constexpr int CHAR_H = 8;

    // Draw a string at (x, y) with the given scale and colour.
    static void drawText(juce::Graphics& g, const juce::String& text,
                         int x, int y, int scale, juce::Colour colour);

    // Return pixel width of a string at the given scale.
    static int textWidth(const juce::String& text, int scale);

private:
    // 96 printable ASCII chars (32-127), 8 bytes each.
    static const uint8_t fontData[96][8];
};
