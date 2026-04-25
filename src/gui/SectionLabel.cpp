#include "SectionLabel.h"
#include "RetroLookAndFeel.h"
#include "BitmapFont.h"

SectionLabel::SectionLabel(const juce::String& t) : title(t) {}

void SectionLabel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();

    // Border
    g.setColour(RetroColours::bevelLight);
    g.drawRect(b.reduced(1));

    // Title text centered at top, with background knockout
    int textW = BitmapFont::textWidth(title, 1);
    int textX = b.getCentreX() - textW / 2;
    int textY = b.getY() - 1;

    g.setColour(RetroColours::background);
    g.fillRect(textX - 4, textY, textW + 8, 10);

    BitmapFont::drawText(g, title, textX, textY + 1, 1, RetroColours::textCyan);
}
