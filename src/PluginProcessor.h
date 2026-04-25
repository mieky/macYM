#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "YmEngine.h"
#include "Presets.h"

class YmvstProcessor : public juce::AudioProcessor
{
public:
    YmvstProcessor();
    ~YmvstProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return NUM_FACTORY_PRESETS; }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void loadPreset(int index);

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    YmEngine& getEngine() { return engine; }
    juce::AudioProcessorValueTreeState& getParams() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    YmEngine engine;
    int currentPreset = 0;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyParametersToEngine();
};
