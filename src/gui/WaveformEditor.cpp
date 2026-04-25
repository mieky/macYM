#include "WaveformEditor.h"
#include "RetroLookAndFeel.h"

WaveformEditor::WaveformEditor()
{
    data.fill(MAX_VALUE);
}

void WaveformEditor::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();

    // Background
    g.setColour(RetroColours::gridBg);
    g.fillRect(b);

    // Border (inset style)
    g.setColour(RetroColours::bevelDark);
    g.drawRect(b);

    float cellW = static_cast<float>(b.getWidth()) / NUM_CELLS;
    float cellH = static_cast<float>(b.getHeight()) / (MAX_VALUE + 1);

    // Draw filled cells
    g.setColour(RetroColours::gridCyan);
    for (int i = 0; i < NUM_CELLS; ++i)
    {
        int val = data[static_cast<size_t>(i)];
        for (int v = 0; v < val; ++v)
        {
            int row = MAX_VALUE - v;
            float x = b.getX() + i * cellW;
            float y = b.getY() + row * cellH;
            g.fillRect(x + 0.5f, y + 0.5f, cellW - 1.0f, cellH - 1.0f);
        }
    }

    // Grid lines (subtle)
    g.setColour(RetroColours::gridBg.brighter(0.15f));
    for (int i = 1; i < NUM_CELLS; ++i)
    {
        float x = b.getX() + i * cellW;
        g.drawVerticalLine(static_cast<int>(x), (float)b.getY(), (float)b.getBottom());
    }
    for (int v = 1; v <= MAX_VALUE; ++v)
    {
        float y = b.getY() + v * cellH;
        g.drawHorizontalLine(static_cast<int>(y), (float)b.getX(), (float)b.getRight());
    }
}

void WaveformEditor::mouseDown(const juce::MouseEvent& e)
{
    setCellFromMouse(e);
}

void WaveformEditor::mouseDrag(const juce::MouseEvent& e)
{
    setCellFromMouse(e);
}

void WaveformEditor::setCellFromMouse(const juce::MouseEvent& e)
{
    auto b = getLocalBounds();
    float cellW = static_cast<float>(b.getWidth()) / NUM_CELLS;
    float cellH = static_cast<float>(b.getHeight()) / (MAX_VALUE + 1);

    int col = static_cast<int>((e.x - b.getX()) / cellW);
    int row = static_cast<int>((e.y - b.getY()) / cellH);

    col = juce::jlimit(0, NUM_CELLS - 1, col);
    int val = juce::jlimit(0, MAX_VALUE, MAX_VALUE - row);

    if (data[static_cast<size_t>(col)] != val)
    {
        data[static_cast<size_t>(col)] = val;
        repaint();
        if (onValueChanged) onValueChanged(col, val);
    }
}

int WaveformEditor::getValue(int index) const
{
    if (index >= 0 && index < NUM_CELLS) return data[static_cast<size_t>(index)];
    return 0;
}

void WaveformEditor::setValue(int index, int value)
{
    if (index >= 0 && index < NUM_CELLS)
    {
        data[static_cast<size_t>(index)] = juce::jlimit(0, MAX_VALUE, value);
        repaint();
    }
}
