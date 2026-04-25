#pragma once
#include <set>
#include "PluginProcessor.h"
#include "gui/BeveledButton.h"
#include "gui/SpinnerControl.h"
#include "gui/SectionLabel.h"
#include "gui/WaveformEditor.h"
#include "gui/ScopeDisplay.h"
#include "gui/HardwareWaveformSelector.h"
#include "Presets.h"

class YmvstEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit YmvstEditor(YmvstProcessor&);
    ~YmvstEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

private:
    void timerCallback() override;
    void syncWidgetsToParams();
    void connectCallbacks();
    static int keyToMidiNote(int keyCode);

    YmvstProcessor& processor;
    std::set<int> keysDown; // Track held keys for noteOff

    // Section labels
    SectionLabel ampWfLabel    { "AMPLITUDE WAVEFORM" };
    SectionLabel hwWfLabel     { "HARDWARE ENVELOPE" };
    SectionLabel noiseLabel    { "NOISE" };
    SectionLabel arpLabel      { "ARPEGGIATOR" };

    // Amplitude waveform section
    WaveformEditor waveformEditor;
    ScopeDisplay scopeDisplay;
    BeveledButton wfOnBtn      { "ON", "OFF" };
    BeveledButton wfOneShotBtn { "1SHOT", "LOOP" };
    SpinnerControl wfSpeed     { 1, 16, 1 };
    SpinnerControl wfLength    { 1, 32, 32 };

    // Hardware waveform section
    HardwareWaveformSelector hwSelector;
    SpinnerControl envSpeed    { 0, 65535, 1000 };

    // Tuning
    SpinnerControl mainTune    { -24, 24, 0 };
    SpinnerControl fineTune1   { -50, 50, 0 };
    SpinnerControl fineTune2   { -50, 50, 0 };
    SpinnerControl fineTune3   { -50, 50, 0 };

    // Noise section
    BeveledButton noiseOnBtn   { "ON", "OFF" };
    SpinnerControl noiseFreq   { 0, 31, 15 };

    // Channel enables
    BeveledButton ch1Btn       { "ON", "OFF" };
    BeveledButton ch2Btn       { "ON", "OFF" };
    BeveledButton ch3Btn       { "ON", "OFF" };

    // Arpeggiator section
    BeveledButton arpOnBtn     { "ON", "OFF" };
    BeveledButton arpSyncBtn   { "SYNC ON", "SYNC OFF" };
    SpinnerControl arpT1       { -24, 24, 12 };
    SpinnerControl arpSpeed    { 1, 16, 2 };
    SpinnerControl arpLength   { 1, 16, 7 };

    // Preset selector
    SpinnerControl presetSelector { 0, NUM_FACTORY_PRESETS - 1, 0 };

    // Bottom row
    BeveledButton panicBtn     { "PANIC", "PANIC" };
    BeveledButton polyBtn      { "POLY", "MONO" };
    BeveledButton sidOnBtn     { "SID ON", "SID OFF" };
    SpinnerControl portaRate   { 0, 100, 4 };
    SpinnerControl sBendDepth  { -50, 50, 0 };
    SpinnerControl sBendSpeed  { 0, 30, 15 };
    SpinnerControl nBendDepth  { -15, 15, 0 };
    SpinnerControl nBendSpeed  { 0, 30, 5 };
    SpinnerControl tremDepth   { 0, 15, 0 };
    SpinnerControl tremSpeed   { 0, 30, 4 };
};
