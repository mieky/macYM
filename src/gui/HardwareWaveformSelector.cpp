#include "HardwareWaveformSelector.h"
#include "RetroLookAndFeel.h"
#include "BitmapFont.h"

HardwareWaveformSelector::HardwareWaveformSelector()
{
    addAndMakeVisible(chEnv1);
    addAndMakeVisible(chEnv2);
    addAndMakeVisible(chEnv3);
}

void HardwareWaveformSelector::resized()
{
    auto b = getLocalBounds();

    // Channel toggles on the left (wide enough for "OFF" text)
    auto toggleArea = b.removeFromLeft(60);
    chEnv1.setBounds(toggleArea.removeFromTop(22).reduced(2));
    chEnv2.setBounds(toggleArea.removeFromTop(22).reduced(2));
    chEnv3.setBounds(toggleArea.removeFromTop(22).reduced(2));
}

void HardwareWaveformSelector::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();
    auto shapeArea = b.withTrimmedLeft(62);

    // Draw 8 envelope shape icons in 2 rows of 4
    int iconW = shapeArea.getWidth() / 4;
    int iconH = shapeArea.getHeight() / 2;

    for (int i = 0; i < 8; ++i)
    {
        int col = i % 4;
        int row = i / 4;
        auto iconBounds = juce::Rectangle<int>(
            shapeArea.getX() + col * iconW,
            shapeArea.getY() + row * iconH,
            iconW, iconH).reduced(2);
        drawEnvelopeIcon(g, iconBounds, i, SHAPE_MAP[i] == selectedShape);
    }
}

void HardwareWaveformSelector::mouseDown(const juce::MouseEvent& e)
{
    auto b = getLocalBounds();
    auto shapeArea = b.withTrimmedLeft(62);

    if (!shapeArea.contains(e.getPosition())) return;

    int iconW = shapeArea.getWidth() / 4;
    int iconH = shapeArea.getHeight() / 2;
    int col = (e.x - shapeArea.getX()) / iconW;
    int row = (e.y - shapeArea.getY()) / iconH;
    int idx = row * 4 + col;

    if (idx >= 0 && idx < 8)
    {
        selectedShape = SHAPE_MAP[idx];
        repaint();
        if (onShapeChanged) onShapeChanged(selectedShape);
    }
}

void HardwareWaveformSelector::setSelectedShape(int shape)
{
    selectedShape = shape;
    repaint();
}

void HardwareWaveformSelector::drawEnvelopeIcon(juce::Graphics& g,
    juce::Rectangle<int> bounds, int shapeIndex, bool selected)
{
    // Background
    g.setColour(selected ? RetroColours::gridBg.brighter(0.3f) : RetroColours::gridBg);
    g.fillRect(bounds);
    g.setColour(selected ? RetroColours::textCyan : RetroColours::bevelDark);
    g.drawRect(bounds);

    // Draw simplified envelope shape as a polyline.
    // Each shape is defined as a series of (x, y) points normalized to 0-1.
    float x0 = (float)bounds.getX() + 2;
    float y0 = (float)bounds.getBottom() - 2;
    float w = (float)bounds.getWidth() - 4;
    float h = (float)bounds.getHeight() - 4;

    g.setColour(RetroColours::textCyan);

    // Simplified envelope paths for the 8 shapes
    struct Pt { float x, y; };
    Pt shapes[8][4] = {
        {{0,0},{0.3f,1},{1,1},{1,1}},       // 0: decay down
        {{0,1},{0.3f,0},{1,0},{1,0}},       // 4: attack up
        {{0,0},{0.25f,1},{0.5f,0},{0.75f,1}}, // 8: decay repeat
        {{0,0},{0.25f,1},{0.5f,1},{1,1}},   // 10: decay then hold high
        {{0,1},{0.25f,0},{0.5f,1},{0.75f,0}}, // 11: attack repeat
        {{0,1},{0.25f,0},{0.5f,0},{1,0}},   // 12: attack then hold low
        {{0,1},{0.5f,0},{0.5f,0},{1,0}},    // 13: attack hold low
        {{0,0},{0.5f,1},{0.5f,1},{1,1}},    // 14: decay hold high
    };

    auto& pts = shapes[shapeIndex];
    float prevPx = x0 + pts[0].x * w;
    float prevPy = y0 - pts[0].y * h;
    for (int i = 1; i < 4; ++i)
    {
        float px = x0 + pts[i].x * w;
        float py = y0 - pts[i].y * h;
        g.drawLine(prevPx, prevPy, px, py, 1.0f);
        prevPx = px;
        prevPy = py;
    }
}
