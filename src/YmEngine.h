#pragma once
#include <cmath>
#include <algorithm>
#include <array>

extern "C" {
#include "ayumi.h"
}

class YmEngine
{
public:
    static constexpr double CLOCK_RATE = 2000000.0;
    static constexpr double BASE_TICK_RATE = 50.0;
    static constexpr int NUM_CHANNELS = 3;
    static constexpr int WAVEFORM_SIZE = 32;
    static constexpr int MAX_ARP_SIZE = 16;

    YmEngine();
    void prepare(double sampleRate);
    void noteOn(int midiNote, int velocity);
    void noteOff();
    void noteOff(int midiNote); // Poly: release specific note
    void processBlock(float* leftOut, float* rightOut, int numSamples);
    void setPolyMode(bool enabled);

    void setChannelEnabled(int ch, bool enabled);
    void setMainTuning(int semitones);
    void setFineTune(int ch, int cents);
    void setNoiseEnabled(bool enabled);
    void setNoiseFrequency(int freq);
    void setChannelToneOn(int ch, bool on);
    void setChannelNoiseOn(int ch, bool on);
    void setEnvelopeShape(int shape);
    void setEnvelopePeriod(int period);
    void setEnvelopeEnabled(int ch, bool enabled);
    void setWaveformValue(int index, int value);
    void setWaveformSpeed(int speed);
    void setWaveformLength(int length);
    void setWaveformEnabled(bool enabled);
    void setWaveformOneShot(bool oneShot);
    void setArpEnabled(bool enabled);
    void setArpSync(bool sync);
    void setArpSpeed(int speed);
    void setArpLength(int length);
    void setArpOffset(int index, int semitones);
    void setPortamentoEnabled(bool enabled);
    void setPortamentoRate(int rate);
    void setSoundBendDepth(int depth);
    void setSoundBendSpeed(int speed);
    void setNoiseBendDepth(int depth);
    void setNoiseBendSpeed(int speed);
    void setTremoloDepth(int depth);
    void setTremoloSpeed(int speed);
    void setSidMode(bool enabled);
    float getLastOutputSample() const { return lastOutputSample; }
    const std::array<int, WAVEFORM_SIZE>& getWaveformData() const { return waveform; }

private:
    struct ayumi ay {};
    double sampleRate = 44100.0;
    int samplesPerTick = 882;

    // Per-voice state (one per YM channel)
    struct Voice {
        int note = -1;
        int velocity = 0;
        bool active = false;
        int waveformIndex = 0;
        int waveformCounter = 0;
        double period = 0.0;
        double targetPeriod = 0.0;
        uint32_t age = 0; // For voice stealing: higher = older
    };
    Voice voices[NUM_CHANNELS] = {};
    uint32_t voiceCounter = 0; // Monotonic counter for age tracking
    bool polyMode = false;

    // Mono-mode convenience (points at voices[0])
    bool anyVoiceActive() const;

    bool channelEnabled[NUM_CHANNELS] = { true, false, false };
    int mainTuning = 0;
    int fineTune[NUM_CHANNELS] = {};

    bool noiseEnabled = false;
    int noiseFrequency = 0;

    bool channelToneOn[NUM_CHANNELS] = { true, true, true };
    bool channelNoiseOn[NUM_CHANNELS] = { false, false, false };

    int envelopeShape = 0;
    int envelopePeriod = 0;
    bool envelopeEnabled[NUM_CHANNELS] = {};

    std::array<int, WAVEFORM_SIZE> waveform {};
    int waveformSpeed = 1;
    int waveformLength = WAVEFORM_SIZE;
    bool waveformEnabled = false;
    bool waveformOneShot = false;
    int waveformIndex = 0;
    int waveformCounter = 0;

    bool arpEnabled = false;
    bool arpSync = false;
    int arpSpeed = 1;
    int arpLength = 1;
    std::array<int, MAX_ARP_SIZE> arpOffsets {};
    int arpIndex = 0;
    int arpCounter = 0;

    bool portamentoEnabled = false;
    int portamentoRate = 0;
    double currentPeriod = 0.0;
    double targetPeriod = 0.0;

    int soundBendDepth = 0;
    int soundBendSpeed = 0;
    double soundBendAccum = 0.0;

    int noiseBendDepth = 0;
    int noiseBendSpeed = 0;
    double noiseBendAccum = 0.0;

    int tremoloDepth = 0;
    int tremoloSpeed = 0;
    double tremoloPhase = 0.0;

    bool sidMode = false;

    float lastOutputSample = 0.0f;
    int tickCounter = 0;

    int noteToTonePeriod(int midiNote, int fineTuneCents = 0) const;
    void tick();
    void updateChipRegisters();
};
