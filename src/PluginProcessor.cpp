#include "PluginProcessor.h"
#include "PluginEditor.h"

static const juce::String CH_NAMES[] = { "1", "2", "3" };

juce::AudioProcessorValueTreeState::ParameterLayout YmvstProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int i = 0; i < 3; ++i)
    {
        auto ch = CH_NAMES[i];
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("ch" + ch + "_on", 1), "Channel " + ch, i == 0));
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID("ch" + ch + "_fine", 1), "Fine Tune " + ch, -50, 50, 0));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("ch" + ch + "_tone", 1), "Tone " + ch, true));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("ch" + ch + "_noise", 1), "Noise " + ch, false));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("ch" + ch + "_env", 1), "Envelope " + ch, false));
    }

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("main_tune", 1), "Main Tuning", -24, 24, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("noise_on", 1), "Noise On", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("noise_freq", 1), "Noise Frequency", 0, 31, 15));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("env_shape", 1), "Envelope Shape", 0, 15, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("env_period", 1), "Envelope Speed", 0, 65535, 1000));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("arp_on", 1), "Arp On", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("arp_sync", 1), "Arp Sync", true));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("arp_speed", 1), "Arp Speed", 1, 16, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("arp_length", 1), "Arp Length", 1, 16, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("arp_t1", 1), "Arp T1", -24, 24, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("wf_on", 1), "Waveform On", true));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("wf_speed", 1), "Waveform Speed", 1, 16, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("wf_length", 1), "Waveform Length", 1, 32, 32));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("wf_oneshot", 1), "Waveform One-Shot", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("porta_on", 1), "Portamento On", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("porta_rate", 1), "Portamento Rate", 0, 100, 4));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("sbend_depth", 1), "Sound Bend Depth", -50, 50, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("sbend_speed", 1), "Sound Bend Speed", 0, 30, 15));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("nbend_depth", 1), "Noise Bend Depth", -15, 15, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("nbend_speed", 1), "Noise Bend Speed", 0, 30, 5));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("trem_depth", 1), "Tremolo Depth", 0, 15, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("trem_speed", 1), "Tremolo Speed", 0, 30, 4));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("sid_on", 1), "SID Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("poly_on", 1), "Poly Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_vol", 1), "Master Volume", 0.0f, 1.0f, 1.0f));

    return { params.begin(), params.end() };
}

YmvstProcessor::YmvstProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "YMVST_PARAMS", createParameterLayout())
{
}

void YmvstProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    engine.prepare(sampleRate);
    applyParametersToEngine();
}

void YmvstProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    buffer.clear();
    applyParametersToEngine();

    for (const auto metadata : midi)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            engine.noteOn(msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isNoteOff())
            engine.noteOff(msg.getNoteNumber());
        else if (msg.isPitchWheel())
        {
            int bendVal = ((msg.getPitchWheelValue() - 8192) * 50) / 8192;
            engine.setSoundBendDepth(bendVal);
        }
        else if (msg.isController())
        {
            int cc = msg.getControllerNumber();
            int val = msg.getControllerValue(); // 0-127
            handleMidiCC(cc, val);
        }
    }

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    engine.processBlock(left, right, buffer.getNumSamples());
}

void YmvstProcessor::applyParametersToEngine()
{
    for (int i = 0; i < 3; ++i)
    {
        auto ch = CH_NAMES[i];
        engine.setChannelEnabled(i, *parameters.getRawParameterValue("ch" + ch + "_on") > 0.5f);
        engine.setFineTune(i, static_cast<int>(*parameters.getRawParameterValue("ch" + ch + "_fine")));
        engine.setChannelToneOn(i, *parameters.getRawParameterValue("ch" + ch + "_tone") > 0.5f);
        engine.setChannelNoiseOn(i, *parameters.getRawParameterValue("ch" + ch + "_noise") > 0.5f);
        engine.setEnvelopeEnabled(i, *parameters.getRawParameterValue("ch" + ch + "_env") > 0.5f);
    }

    engine.setMainTuning(static_cast<int>(*parameters.getRawParameterValue("main_tune")));
    engine.setNoiseEnabled(*parameters.getRawParameterValue("noise_on") > 0.5f);
    engine.setNoiseFrequency(static_cast<int>(*parameters.getRawParameterValue("noise_freq")));
    engine.setEnvelopeShape(static_cast<int>(*parameters.getRawParameterValue("env_shape")));
    engine.setEnvelopePeriod(static_cast<int>(*parameters.getRawParameterValue("env_period")));

    engine.setArpEnabled(*parameters.getRawParameterValue("arp_on") > 0.5f);
    engine.setArpSync(*parameters.getRawParameterValue("arp_sync") > 0.5f);
    engine.setArpSpeed(static_cast<int>(*parameters.getRawParameterValue("arp_speed")));
    engine.setArpLength(static_cast<int>(*parameters.getRawParameterValue("arp_length")));
    engine.setArpOffset(0, static_cast<int>(*parameters.getRawParameterValue("arp_t1")));

    engine.setWaveformEnabled(*parameters.getRawParameterValue("wf_on") > 0.5f);
    engine.setWaveformSpeed(static_cast<int>(*parameters.getRawParameterValue("wf_speed")));
    engine.setWaveformLength(static_cast<int>(*parameters.getRawParameterValue("wf_length")));
    engine.setWaveformOneShot(*parameters.getRawParameterValue("wf_oneshot") > 0.5f);

    engine.setPortamentoEnabled(*parameters.getRawParameterValue("porta_on") > 0.5f);
    engine.setPortamentoRate(static_cast<int>(*parameters.getRawParameterValue("porta_rate")));

    engine.setSoundBendDepth(static_cast<int>(*parameters.getRawParameterValue("sbend_depth")));
    engine.setSoundBendSpeed(static_cast<int>(*parameters.getRawParameterValue("sbend_speed")));
    engine.setNoiseBendDepth(static_cast<int>(*parameters.getRawParameterValue("nbend_depth")));
    engine.setNoiseBendSpeed(static_cast<int>(*parameters.getRawParameterValue("nbend_speed")));

    engine.setTremoloDepth(static_cast<int>(*parameters.getRawParameterValue("trem_depth")));
    engine.setTremoloSpeed(static_cast<int>(*parameters.getRawParameterValue("trem_speed")));
    engine.setSidMode(*parameters.getRawParameterValue("sid_on") > 0.5f);
    engine.setPolyMode(*parameters.getRawParameterValue("poly_on") > 0.5f);
    engine.setMasterVolume(*parameters.getRawParameterValue("master_vol"));
}

// MIDI CC mapping matching the original ymVST controller assignments.
// See https://www.preromanbritain.com/ymvst/midi.txt
void YmvstProcessor::handleMidiCC(int cc, int val)
{
    // Helper: set a parameter by ID, scaling 0-127 to the parameter's range
    auto setScaled = [&](const juce::String& id, int v) {
        if (auto* p = parameters.getParameter(id))
            p->setValueNotifyingHost(static_cast<float>(v) / 127.0f);
    };
    // Helper: set a bool parameter from CC (>=64 = on)
    auto setBool = [&](const juce::String& id, int v) {
        if (auto* p = parameters.getParameter(id))
            p->setValueNotifyingHost(v >= 64 ? 1.0f : 0.0f);
    };

    switch (cc)
    {
        case 1:   setScaled("trem_depth", val); break;      // Mod Wheel → Tremolo Depth
        case 3:   setScaled("main_tune", val); break;        // Hardwave Main Tune
        case 5:   setScaled("porta_rate", val); break;       // Portamento Time
        case 7:   setScaled("master_vol", val); break;       // Master Volume
        case 9:   setScaled("noise_freq", val); break;       // Noise Freq

        case 16:  setScaled("noise_freq", val); break;       // Noise Length (mapped to freq)
        case 17:  setScaled("wf_length", val); break;        // Square (waveform) Length
        case 19:  setScaled("arp_length", val); break;       // Arp Length

        // CC 20-29: write waveform envelope values (10 of 32 steps, spread evenly)
        case 20: case 21: case 22: case 23: case 24:
        case 25: case 26: case 27: case 28: case 29:
        {
            int step = (cc - 20) * 3;  // Maps CC 20-29 to steps 0,3,6,...27
            int wfVal = (val * 15) / 127;
            engine.setWaveformValue(step, wfVal);
            if (step + 1 < YmEngine::WAVEFORM_SIZE) engine.setWaveformValue(step + 1, wfVal);
            if (step + 2 < YmEngine::WAVEFORM_SIZE) engine.setWaveformValue(step + 2, wfVal);
            break;
        }

        case 30:  setScaled("nbend_depth", val); break;     // Noise Bend Depth
        case 31:  setScaled("nbend_speed", val); break;     // Noise Bend Rate

        case 64:  setScaled("env_period", val); break;      // Envelope Speed (Sustain)
        case 70:  setScaled("env_shape", val); break;       // Hardware Waveform shape
        case 71:  setScaled("sbend_speed", val); break;     // Pitch Bend Rate
        case 72:  setScaled("sbend_depth", val); break;     // Pitch Bend Depth

        case 75:  setBool("wf_oneshot", val); break;          // Square Sync (one-shot toggle)
        case 76:  setBool("ch1_env", val); break;            // Hardwave Sync (envelope toggle)
        case 77:  setBool("arp_sync", val); break;           // Arp Sync
        case 78:  setBool("sid_on", val); break;             // SID FX On/Off

        case 80:  setBool("noise_on", val); break;           // Noise On/Off
        case 81:  setBool("wf_on", val); break;              // Square (waveform) On/Off
        case 82:  setBool("ch1_env", val); break;            // Hardware Envelope On/Off
        case 83:  setBool("arp_on", val); break;             // Arp On/Off

        case 87:  setScaled("arp_speed", val); break;       // Arp Speed
        case 88:  engine.setArpOffset(0, (val * 48 / 127) - 24); break;  // Arp Step 0 (-24..+24)
        case 89:  engine.setArpOffset(1, (val * 48 / 127) - 24); break;  // Arp Step 1
        case 90:  engine.setArpOffset(2, (val * 48 / 127) - 24); break;  // Arp Step 2

        case 92:  setScaled("trem_speed", val); break;      // Tremolo Freq

        case 94:  // Hardwave Detune → fine tune channel 2
            engine.setFineTune(1, (val * 100 / 127) - 50);
            break;
        case 95:  // SID Detune → fine tune channel 3
            engine.setFineTune(2, (val * 100 / 127) - 50);
            break;

        default: break;
    }
}

const juce::String YmvstProcessor::getProgramName(int index)
{
    if (index >= 0 && index < NUM_FACTORY_PRESETS)
        return FACTORY_PRESETS[index].name;
    return {};
}

void YmvstProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < NUM_FACTORY_PRESETS)
        loadPreset(index);
}

void YmvstProcessor::loadPreset(int index)
{
    if (index < 0 || index >= NUM_FACTORY_PRESETS) return;
    currentPreset = index;
    const auto& p = FACTORY_PRESETS[index];

    auto set = [&](const juce::String& id, float val) {
        if (auto* param = parameters.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    static const juce::String CH_IDS[] = { "1", "2", "3" };
    for (int i = 0; i < 3; ++i)
    {
        auto ch = CH_IDS[i];
        set("ch" + ch + "_on",    p.chOn[i] ? 1.f : 0.f);
        set("ch" + ch + "_fine",  (float)p.chFine[i]);
        set("ch" + ch + "_tone",  p.chTone[i] ? 1.f : 0.f);
        set("ch" + ch + "_noise", p.chNoise[i] ? 1.f : 0.f);
        set("ch" + ch + "_env",   p.chEnv[i] ? 1.f : 0.f);
    }

    set("main_tune",   (float)p.mainTune);
    set("noise_on",    p.noiseOn ? 1.f : 0.f);
    set("noise_freq",  (float)p.noiseFreq);
    set("env_shape",   (float)p.envShape);
    set("env_period",  (float)p.envPeriod);
    set("arp_on",      p.arpOn ? 1.f : 0.f);
    set("arp_sync",    p.arpSync ? 1.f : 0.f);
    set("arp_speed",   (float)p.arpSpeed);
    set("arp_length",  (float)p.arpLength);
    set("arp_t1",      (float)p.arpT1);
    set("wf_on",       p.wfOn ? 1.f : 0.f);
    set("wf_oneshot",  p.wfOneShot ? 1.f : 0.f);
    set("wf_speed",    (float)p.wfSpeed);
    set("wf_length",   (float)p.wfLength);
    set("porta_on",    p.portaOn ? 1.f : 0.f);
    set("porta_rate",  (float)p.portaRate);
    set("sbend_depth", (float)p.sBendDepth);
    set("sbend_speed", (float)p.sBendSpeed);
    set("nbend_depth", (float)p.nBendDepth);
    set("nbend_speed", (float)p.nBendSpeed);
    set("trem_depth",  (float)p.tremDepth);
    set("trem_speed",  (float)p.tremSpeed);
    set("sid_on",      p.sidOn ? 1.f : 0.f);
    set("poly_on",     p.polyOn ? 1.f : 0.f);

    // Load waveform data directly into engine
    for (int i = 0; i < YmEngine::WAVEFORM_SIZE; ++i)
        engine.setWaveformValue(i, p.waveform[i]);
}

juce::AudioProcessorEditor* YmvstProcessor::createEditor()
{
    return new YmvstEditor(*this);
}

void YmvstProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    juce::ValueTree wfData("WAVEFORM");
    auto& wf = engine.getWaveformData();
    for (int i = 0; i < YmEngine::WAVEFORM_SIZE; ++i)
        wfData.setProperty(juce::Identifier("v" + juce::String(i)), wf[i], nullptr);
    state.addChild(wfData, -1, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void YmvstProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml == nullptr) return;

    auto state = juce::ValueTree::fromXml(*xml);
    if (!state.hasType(parameters.state.getType())) return;

    auto wfData = state.getChildWithName("WAVEFORM");
    if (wfData.isValid())
    {
        for (int i = 0; i < YmEngine::WAVEFORM_SIZE; ++i)
        {
            int val = wfData.getProperty(juce::Identifier("v" + juce::String(i)), 15);
            engine.setWaveformValue(i, val);
        }
    }

    state.removeChild(wfData, nullptr);
    parameters.replaceState(state);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new YmvstProcessor();
}
