#include "ScopeDisplay.h"
#include "RetroLookAndFeel.h"
#include "BitmapFont.h"

ScopeDisplay::ScopeDisplay()
{
    startTimerHz(30);
}

ScopeDisplay::~ScopeDisplay()
{
    stopTimer();
}

void ScopeDisplay::pushSample(float sample)
{
    buffer[static_cast<size_t>(writeIndex)] = sample;
    writeIndex = (writeIndex + 1) % BUFFER_SIZE;
}

void ScopeDisplay::timerCallback()
{
    repaint();
}

void ScopeDisplay::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();

    // Background
    g.setColour(RetroColours::scopeBg);
    g.fillRect(b);

    // Border
    g.setColour(RetroColours::bevelDark);
    g.drawRect(b);

    // Center line
    g.setColour(RetroColours::scopeGreen.withAlpha(0.3f));
    g.drawHorizontalLine(b.getCentreY(), (float)b.getX(), (float)b.getRight());

    // Scale labels
    BitmapFont::drawText(g, "Scope", b.getRight() - 44, b.getY() + 2, 1, RetroColours::scopeGreen);

    // Waveform
    g.setColour(RetroColours::scopeGreen);
    float xScale = static_cast<float>(b.getWidth()) / BUFFER_SIZE;
    float yCenter = static_cast<float>(b.getCentreY());
    float yRange = static_cast<float>(b.getHeight()) * 0.45f;

    int readIndex = (writeIndex + 1) % BUFFER_SIZE;
    float prevX = static_cast<float>(b.getX());
    float prevY = yCenter - buffer[static_cast<size_t>(readIndex)] * yRange;

    for (int i = 1; i < BUFFER_SIZE; ++i)
    {
        int idx = (readIndex + i) % BUFFER_SIZE;
        float x = b.getX() + i * xScale;
        float y = yCenter - buffer[static_cast<size_t>(idx)] * yRange;
        g.drawLine(prevX, prevY, x, y, 1.0f);
        prevX = x;
        prevY = y;
    }
}
