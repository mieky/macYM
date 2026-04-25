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

    // Lock-free MIDI queue for editor -> audio thread note injection
    juce::MidiBuffer editorMidiBuffer;
    juce::SpinLock editorMidiLock;

private:
    juce::AudioProcessorValueTreeState parameters;
    YmEngine engine;
    int currentPreset = 0;

    // Cached parameter pointers (safe to read on audio thread without allocation)
    struct CachedParams {
        std::atomic<float>* chOn[3]{};
        std::atomic<float>* chFine[3]{};
        std::atomic<float>* chTone[3]{};
        std::atomic<float>* chNoise[3]{};
        std::atomic<float>* chEnv[3]{};
        std::atomic<float>* mainTune{};
        std::atomic<float>* noiseOn{};
        std::atomic<float>* noiseFreq{};
        std::atomic<float>* envShape{};
        std::atomic<float>* envPeriod{};
        std::atomic<float>* arpOn{};
        std::atomic<float>* arpSync{};
        std::atomic<float>* arpSpeed{};
        std::atomic<float>* arpLength{};
        std::atomic<float>* arpT1{};
        std::atomic<float>* wfOn{};
        std::atomic<float>* wfOneShot{};
        std::atomic<float>* wfSpeed{};
        std::atomic<float>* wfLength{};
        std::atomic<float>* portaOn{};
        std::atomic<float>* portaRate{};
        std::atomic<float>* sBendDepth{};
        std::atomic<float>* sBendSpeed{};
        std::atomic<float>* nBendDepth{};
        std::atomic<float>* nBendSpeed{};
        std::atomic<float>* tremDepth{};
        std::atomic<float>* tremSpeed{};
        std::atomic<float>* sidOn{};
        std::atomic<float>* polyOn{};
        std::atomic<float>* masterVol{};
    } cachedParams;

    void cacheParameterPointers();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyParametersToEngine();
    void handleMidiCC(int cc, int value);
};
