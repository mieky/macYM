#include "YmEngine.h"
#include <cstring>

YmEngine::YmEngine()
{
    std::memset(&ay, 0, sizeof(ay));
    const int defaultWaveform[WAVEFORM_SIZE] = {
        15, 14, 13, 12, 11, 10, 9, 8,
         7,  6,  5,  5,  4,  4,  3,  3,
         2,  2,  2,  1,  1,  1,  1,  1,
         0,  0,  0,  0,  0,  0,  0,  0
    };
    for (int i = 0; i < WAVEFORM_SIZE; ++i)
        waveform[i] = defaultWaveform[i];
}

void YmEngine::prepare(double sr)
{
    sampleRate = sr;
    samplesPerTick = static_cast<int>(sr / BASE_TICK_RATE);
    ayumi_configure(&ay, 1, CLOCK_RATE, static_cast<int>(sr));
    for (int i = 0; i < NUM_CHANNELS; ++i)
        ayumi_set_pan(&ay, i, 0.5, 0);
}

bool YmEngine::anyVoiceActive() const
{
    for (int i = 0; i < NUM_CHANNELS; ++i)
        if (voices[i].active) return true;
    return false;
}

void YmEngine::noteOn(int midiNote, int velocity)
{
    if (polyMode)
    {
        // Find a free voice, or steal the oldest
        int target = -1;
        uint32_t oldestAge = UINT32_MAX;
        for (int i = 0; i < NUM_CHANNELS; ++i)
        {
            if (!voices[i].active) { target = i; break; }
            if (voices[i].age < oldestAge) { oldestAge = voices[i].age; target = i; }
        }

        Voice& v = voices[target];
        v.note = midiNote;
        v.velocity = velocity;
        v.active = true;
        v.targetPeriod = static_cast<double>(noteToTonePeriod(midiNote));
        v.period = v.targetPeriod;
        v.waveformIndex = 0;
        v.waveformCounter = 0;
        v.age = ++voiceCounter;
    }
    else
    {
        // Mono: all enabled channels play this note
        double prevPeriod = voices[0].period;
        for (int i = 0; i < NUM_CHANNELS; ++i)
        {
            voices[i].note = midiNote;
            voices[i].velocity = velocity;
            voices[i].active = true;
            voices[i].targetPeriod = static_cast<double>(noteToTonePeriod(midiNote));
            voices[i].age = ++voiceCounter;
        }

        if (portamentoEnabled && prevPeriod > 0.0)
            voices[0].period = prevPeriod;
        else
            voices[0].period = voices[0].targetPeriod;

        // Copy period to other channels in mono
        for (int i = 1; i < NUM_CHANNELS; ++i)
            voices[i].period = voices[0].period;

        if (arpSync || !arpEnabled) {
            arpIndex = 0;
            arpCounter = 0;
        }
        waveformIndex = 0;
        waveformCounter = 0;
        soundBendAccum = 0.0;
        noiseBendAccum = 0.0;
        tremoloPhase = 0.0;
    }

    tickCounter = 0;
    updateChipRegisters();
}

void YmEngine::noteOff()
{
    // Release all voices (mono mode, or "all notes off")
    for (int i = 0; i < NUM_CHANNELS; ++i)
    {
        voices[i].active = false;
        voices[i].note = -1;
        voices[i].velocity = 0;
        ayumi_set_volume(&ay, i, 0);
    }
}

void YmEngine::noteOff(int midiNote)
{
    if (polyMode)
    {
        // Release the voice playing this specific note
        for (int i = 0; i < NUM_CHANNELS; ++i)
        {
            if (voices[i].active && voices[i].note == midiNote)
            {
                voices[i].active = false;
                voices[i].note = -1;
                voices[i].velocity = 0;
                ayumi_set_volume(&ay, i, 0);
                return; // Only release one voice per noteOff
            }
        }
    }
    else
    {
        noteOff();
    }
}

void YmEngine::setPolyMode(bool enabled) { polyMode = enabled; }

void YmEngine::processBlock(float* leftOut, float* rightOut, int numSamples)
{
    bool active = anyVoiceActive();

    for (int s = 0; s < numSamples; ++s)
    {
        if (active && ++tickCounter >= samplesPerTick)
        {
            tickCounter = 0;
            tick();
            updateChipRegisters();
        }

        ayumi_process(&ay);
        ayumi_remove_dc(&ay);

        leftOut[s] = static_cast<float>(ay.left);
        rightOut[s] = static_cast<float>(ay.right);
    }

    if (numSamples > 0)
        lastOutputSample = (leftOut[numSamples - 1] + rightOut[numSamples - 1]) * 0.5f;
}

int YmEngine::noteToTonePeriod(int midiNote, int fineTuneCents) const
{
    int adjustedNote = midiNote + mainTuning;
    double freq = 440.0 * std::pow(2.0, (adjustedNote - 69 + fineTuneCents / 100.0) / 12.0);
    int period = static_cast<int>(CLOCK_RATE / (16.0 * freq));
    return std::clamp(period, 1, 4095);
}

void YmEngine::tick()
{
    if (!polyMode)
    {
        // Mono features: arp, global waveform, portamento
        if (arpEnabled && arpLength > 0)
        {
            if (++arpCounter >= arpSpeed)
            {
                arpCounter = 0;
                arpIndex = (arpIndex + 1) % arpLength;
            }
        }

        if (waveformEnabled && waveformLength > 0)
        {
            if (waveformIndex < waveformLength - 1 || !waveformOneShot)
            {
                if (++waveformCounter >= waveformSpeed)
                {
                    waveformCounter = 0;
                    if (waveformOneShot)
                        waveformIndex = std::min(waveformIndex + 1, waveformLength - 1);
                    else
                        waveformIndex = (waveformIndex + 1) % waveformLength;
                }
            }
        }

        if (portamentoEnabled && portamentoRate > 0 && voices[0].period != voices[0].targetPeriod)
        {
            double step = (voices[0].targetPeriod - voices[0].period) / portamentoRate;
            if (std::abs(step) < 1.0)
                voices[0].period = voices[0].targetPeriod;
            else
                voices[0].period += step;
            for (int i = 1; i < NUM_CHANNELS; ++i)
                voices[i].period = voices[0].period;
        }
    }
    else
    {
        // Poly: per-voice waveform advancement
        for (int i = 0; i < NUM_CHANNELS; ++i)
        {
            if (!voices[i].active) continue;
            if (waveformEnabled && waveformLength > 0)
            {
                if (voices[i].waveformIndex < waveformLength - 1 || !waveformOneShot)
                {
                    if (++voices[i].waveformCounter >= waveformSpeed)
                    {
                        voices[i].waveformCounter = 0;
                        if (waveformOneShot)
                            voices[i].waveformIndex = std::min(voices[i].waveformIndex + 1, waveformLength - 1);
                        else
                            voices[i].waveformIndex = (voices[i].waveformIndex + 1) % waveformLength;
                    }
                }
            }
        }
    }

    // Global modulations (both modes)
    if (soundBendDepth != 0 && soundBendSpeed > 0)
        soundBendAccum += static_cast<double>(soundBendDepth) / (soundBendSpeed * 50.0);

    if (noiseBendDepth != 0 && noiseBendSpeed > 0)
        noiseBendAccum += static_cast<double>(noiseBendDepth) / (noiseBendSpeed * 50.0);

    if (tremoloDepth > 0 && tremoloSpeed > 0)
    {
        tremoloPhase += static_cast<double>(tremoloSpeed) / 50.0;
        if (tremoloPhase > 1.0) tremoloPhase -= 1.0;
    }
}

void YmEngine::updateChipRegisters()
{
    for (int ch = 0; ch < NUM_CHANNELS; ++ch)
    {
        Voice& v = voices[ch];

        if (polyMode)
        {
            // Poly: each channel is an independent voice
            if (!v.active)
            {
                ayumi_set_volume(&ay, ch, 0);
                ayumi_set_mixer(&ay, ch, 1, 1, 0);
                continue;
            }

            double period = static_cast<double>(noteToTonePeriod(v.note, fineTune[ch]));
            period += soundBendAccum;
            period = std::clamp(period, 1.0, 4095.0);
            ayumi_set_tone(&ay, ch, static_cast<int>(period));

            ayumi_set_mixer(&ay, ch, 0, noiseEnabled ? 0 : 1, envelopeEnabled[ch] ? 1 : 0);

            if (envelopeEnabled[ch])
            {
                ayumi_set_volume(&ay, ch, 0);
            }
            else
            {
                int vol = (v.velocity * 15) / 127;
                if (waveformEnabled && waveformLength > 0)
                    vol = waveform[v.waveformIndex];
                if (tremoloDepth > 0)
                {
                    double tremMod = std::sin(tremoloPhase * 2.0 * 3.14159265358979);
                    int tremOffset = static_cast<int>(tremMod * tremoloDepth);
                    vol = std::clamp(vol + tremOffset, 0, 15);
                }
                ayumi_set_volume(&ay, ch, vol);
            }
        }
        else
        {
            // Mono: original behavior
            if (!channelEnabled[ch])
            {
                ayumi_set_volume(&ay, ch, 0);
                ayumi_set_mixer(&ay, ch, 1, 1, 0);
                continue;
            }

            if (!v.active) continue;

            int baseVolume = (v.velocity * 15) / 127;
            int arpOffset = (arpEnabled && arpLength > 0) ? arpOffsets[arpIndex] : 0;

            double period;
            if (arpEnabled && arpLength > 0)
                period = static_cast<double>(noteToTonePeriod(v.note + arpOffset, fineTune[ch]));
            else
                period = static_cast<double>(noteToTonePeriod(v.note, fineTune[ch]));

            if (portamentoEnabled && ch == 0)
                period = voices[0].period;

            period += soundBendAccum;
            period = std::clamp(period, 1.0, 4095.0);
            ayumi_set_tone(&ay, ch, static_cast<int>(period));

            int toneOff = channelToneOn[ch] ? 0 : 1;
            int noiseOff = (channelNoiseOn[ch] || noiseEnabled) ? 0 : 1;
            int envOn = envelopeEnabled[ch] ? 1 : 0;
            ayumi_set_mixer(&ay, ch, toneOff, noiseOff, envOn);

            if (envelopeEnabled[ch])
            {
                ayumi_set_volume(&ay, ch, 0);
            }
            else
            {
                int vol = baseVolume;
                if (waveformEnabled && waveformLength > 0)
                    vol = waveform[waveformIndex];
                if (tremoloDepth > 0)
                {
                    double tremMod = std::sin(tremoloPhase * 2.0 * 3.14159265358979);
                    int tremOffset = static_cast<int>(tremMod * tremoloDepth);
                    vol = std::clamp(vol + tremOffset, 0, 15);
                }
                ayumi_set_volume(&ay, ch, vol);
            }
        }
    }

    int noiseP = noiseFrequency + static_cast<int>(noiseBendAccum);
    noiseP = std::clamp(noiseP, 0, 31);
    ayumi_set_noise(&ay, noiseP);

    ayumi_set_envelope(&ay, envelopePeriod);
    ayumi_set_envelope_shape(&ay, envelopeShape);
}

// --- Setters ---

void YmEngine::setChannelEnabled(int ch, bool enabled) {
    if (ch >= 0 && ch < NUM_CHANNELS) channelEnabled[ch] = enabled;
}
void YmEngine::setMainTuning(int semitones) { mainTuning = semitones; }
void YmEngine::setFineTune(int ch, int cents) {
    if (ch >= 0 && ch < NUM_CHANNELS) fineTune[ch] = cents;
}
void YmEngine::setNoiseEnabled(bool enabled) { noiseEnabled = enabled; }
void YmEngine::setNoiseFrequency(int freq) { noiseFrequency = std::clamp(freq, 0, 31); }
void YmEngine::setChannelToneOn(int ch, bool on) {
    if (ch >= 0 && ch < NUM_CHANNELS) channelToneOn[ch] = on;
}
void YmEngine::setChannelNoiseOn(int ch, bool on) {
    if (ch >= 0 && ch < NUM_CHANNELS) channelNoiseOn[ch] = on;
}
void YmEngine::setEnvelopeShape(int shape) { envelopeShape = std::clamp(shape, 0, 15); }
void YmEngine::setEnvelopePeriod(int period) { envelopePeriod = std::clamp(period, 0, 65535); }
void YmEngine::setEnvelopeEnabled(int ch, bool enabled) {
    if (ch >= 0 && ch < NUM_CHANNELS) envelopeEnabled[ch] = enabled;
}
void YmEngine::setWaveformValue(int index, int value) {
    if (index >= 0 && index < WAVEFORM_SIZE) waveform[index] = std::clamp(value, 0, 15);
}
void YmEngine::setWaveformSpeed(int speed) { waveformSpeed = std::max(1, speed); }
void YmEngine::setWaveformLength(int length) { waveformLength = std::clamp(length, 1, WAVEFORM_SIZE); }
void YmEngine::setWaveformEnabled(bool enabled) { waveformEnabled = enabled; }
void YmEngine::setWaveformOneShot(bool oneShot) { waveformOneShot = oneShot; }
void YmEngine::setArpEnabled(bool enabled) { arpEnabled = enabled; }
void YmEngine::setArpSync(bool sync) { arpSync = sync; }
void YmEngine::setArpSpeed(int speed) { arpSpeed = std::max(1, speed); }
void YmEngine::setArpLength(int length) { arpLength = std::clamp(length, 1, MAX_ARP_SIZE); }
void YmEngine::setArpOffset(int index, int semitones) {
    if (index >= 0 && index < MAX_ARP_SIZE) arpOffsets[index] = semitones;
}
void YmEngine::setPortamentoEnabled(bool enabled) { portamentoEnabled = enabled; }
void YmEngine::setPortamentoRate(int rate) { portamentoRate = std::max(0, rate); }
void YmEngine::setSoundBendDepth(int depth) { soundBendDepth = depth; }
void YmEngine::setSoundBendSpeed(int speed) { soundBendSpeed = std::max(0, speed); }
void YmEngine::setNoiseBendDepth(int depth) { noiseBendDepth = depth; }
void YmEngine::setNoiseBendSpeed(int speed) { noiseBendSpeed = std::max(0, speed); }
void YmEngine::setTremoloDepth(int depth) { tremoloDepth = std::clamp(depth, 0, 15); }
void YmEngine::setTremoloSpeed(int speed) { tremoloSpeed = std::max(0, speed); }
void YmEngine::setSidMode(bool enabled) { sidMode = enabled; }
