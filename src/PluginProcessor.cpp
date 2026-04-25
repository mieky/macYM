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
    cacheParameterPointers();
}

void YmvstProcessor::cacheParameterPointers()
{
    static const juce::String chNames[] = { "1", "2", "3" };
    for (int i = 0; i < 3; ++i)
    {
        auto ch = chNames[i];
        cachedParams.chOn[i]    = parameters.getRawParameterValue("ch" + ch + "_on");
        cachedParams.chFine[i]  = parameters.getRawParameterValue("ch" + ch + "_fine");
        cachedParams.chTone[i]  = parameters.getRawParameterValue("ch" + ch + "_tone");
        cachedParams.chNoise[i] = parameters.getRawParameterValue("ch" + ch + "_noise");
        cachedParams.chEnv[i]   = parameters.getRawParameterValue("ch" + ch + "_env");
    }
    cachedParams.mainTune  = parameters.getRawParameterValue("main_tune");
    cachedParams.noiseOn   = parameters.getRawParameterValue("noise_on");
    cachedParams.noiseFreq = parameters.getRawParameterValue("noise_freq");
    cachedParams.envShape  = parameters.getRawParameterValue("env_shape");
    cachedParams.envPeriod = parameters.getRawParameterValue("env_period");
    cachedParams.arpOn     = parameters.getRawParameterValue("arp_on");
    cachedParams.arpSync   = parameters.getRawParameterValue("arp_sync");
    cachedParams.arpSpeed  = parameters.getRawParameterValue("arp_speed");
    cachedParams.arpLength = parameters.getRawParameterValue("arp_length");
    cachedParams.arpT1     = parameters.getRawParameterValue("arp_t1");
    cachedParams.wfOn      = parameters.getRawParameterValue("wf_on");
    cachedParams.wfOneShot = parameters.getRawParameterValue("wf_oneshot");
    cachedParams.wfSpeed   = parameters.getRawParameterValue("wf_speed");
    cachedParams.wfLength  = parameters.getRawParameterValue("wf_length");
    cachedParams.portaOn   = parameters.getRawParameterValue("porta_on");
    cachedParams.portaRate = parameters.getRawParameterValue("porta_rate");
    cachedParams.sBendDepth = parameters.getRawParameterValue("sbend_depth");
    cachedParams.sBendSpeed = parameters.getRawParameterValue("sbend_speed");
    cachedParams.nBendDepth = parameters.getRawParameterValue("nbend_depth");
    cachedParams.nBendSpeed = parameters.getRawParameterValue("nbend_speed");
    cachedParams.tremDepth = parameters.getRawParameterValue("trem_depth");
    cachedParams.tremSpeed = parameters.getRawParameterValue("trem_speed");
    cachedParams.sidOn     = parameters.getRawParameterValue("sid_on");
    cachedParams.polyOn    = parameters.getRawParameterValue("poly_on");
    cachedParams.masterVol = parameters.getRawParameterValue("master_vol");

    // Cache RangedAudioParameter pointers for CC handling
    cachedCCParams.tremDepth  = parameters.getParameter("trem_depth");
    cachedCCParams.mainTune   = parameters.getParameter("main_tune");
    cachedCCParams.portaRate  = parameters.getParameter("porta_rate");
    cachedCCParams.masterVol  = parameters.getParameter("master_vol");
    cachedCCParams.noiseFreq  = parameters.getParameter("noise_freq");
    cachedCCParams.wfLength   = parameters.getParameter("wf_length");
    cachedCCParams.arpLength  = parameters.getParameter("arp_length");
    cachedCCParams.nBendDepth = parameters.getParameter("nbend_depth");
    cachedCCParams.nBendSpeed = parameters.getParameter("nbend_speed");
    cachedCCParams.envPeriod  = parameters.getParameter("env_period");
    cachedCCParams.envShape   = parameters.getParameter("env_shape");
    cachedCCParams.sBendSpeed = parameters.getParameter("sbend_speed");
    cachedCCParams.sBendDepth = parameters.getParameter("sbend_depth");
    cachedCCParams.wfOneShot  = parameters.getParameter("wf_oneshot");
    cachedCCParams.ch1Env     = parameters.getParameter("ch1_env");
    cachedCCParams.arpSync    = parameters.getParameter("arp_sync");
    cachedCCParams.sidOn      = parameters.getParameter("sid_on");
    cachedCCParams.noiseOn    = parameters.getParameter("noise_on");
    cachedCCParams.wfOn       = parameters.getParameter("wf_on");
    cachedCCParams.arpOn      = parameters.getParameter("arp_on");
    cachedCCParams.arpSpeed   = parameters.getParameter("arp_speed");
    cachedCCParams.tremSpeed  = parameters.getParameter("trem_speed");
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

    // Merge editor MIDI (keyboard notes from GUI thread)
    {
        const juce::SpinLock::ScopedTryLockType lock(editorMidiLock);
        if (lock.isLocked())
        {
            midi.addEvents(editorMidiBuffer, 0, buffer.getNumSamples(), 0);
            editorMidiBuffer.clear();
        }
    }

    for (const auto metadata : midi)
    {
        auto msg = metadata.getMessage();
        if (msg.isAllNotesOff() || msg.isAllSoundOff())
            engine.noteOff();
        else if (msg.isNoteOn())
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
            handleMidiCC(msg.getControllerNumber(), msg.getControllerValue());
        }
    }

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    engine.processBlock(left, right, buffer.getNumSamples());
}

// No allocations - all reads are from cached atomic pointers
void YmvstProcessor::applyParametersToEngine()
{
    auto& p = cachedParams;
    for (int i = 0; i < 3; ++i)
    {
        engine.setChannelEnabled(i, p.chOn[i]->load() > 0.5f);
        engine.setFineTune(i, static_cast<int>(p.chFine[i]->load()));
        engine.setChannelToneOn(i, p.chTone[i]->load() > 0.5f);
        engine.setChannelNoiseOn(i, p.chNoise[i]->load() > 0.5f);
        engine.setEnvelopeEnabled(i, p.chEnv[i]->load() > 0.5f);
    }

    engine.setMainTuning(static_cast<int>(p.mainTune->load()));
    engine.setNoiseEnabled(p.noiseOn->load() > 0.5f);
    engine.setNoiseFrequency(static_cast<int>(p.noiseFreq->load()));
    engine.setEnvelopeShape(static_cast<int>(p.envShape->load()));
    engine.setEnvelopePeriod(static_cast<int>(p.envPeriod->load()));

    engine.setArpEnabled(p.arpOn->load() > 0.5f);
    engine.setArpSync(p.arpSync->load() > 0.5f);
    engine.setArpSpeed(static_cast<int>(p.arpSpeed->load()));
    engine.setArpLength(static_cast<int>(p.arpLength->load()));
    engine.setArpOffset(0, static_cast<int>(p.arpT1->load()));

    engine.setWaveformEnabled(p.wfOn->load() > 0.5f);
    engine.setWaveformSpeed(static_cast<int>(p.wfSpeed->load()));
    engine.setWaveformLength(static_cast<int>(p.wfLength->load()));
    engine.setWaveformOneShot(p.wfOneShot->load() > 0.5f);

    engine.setPortamentoEnabled(p.portaOn->load() > 0.5f);
    engine.setPortamentoRate(static_cast<int>(p.portaRate->load()));

    engine.setSoundBendDepth(static_cast<int>(p.sBendDepth->load()));
    engine.setSoundBendSpeed(static_cast<int>(p.sBendSpeed->load()));
    engine.setNoiseBendDepth(static_cast<int>(p.nBendDepth->load()));
    engine.setNoiseBendSpeed(static_cast<int>(p.nBendSpeed->load()));

    engine.setTremoloDepth(static_cast<int>(p.tremDepth->load()));
    engine.setTremoloSpeed(static_cast<int>(p.tremSpeed->load()));
    engine.setSidMode(p.sidOn->load() > 0.5f);
    engine.setPolyMode(p.polyOn->load() > 0.5f);
    engine.setMasterVolume(p.masterVol->load());
}

// MIDI CC mapping matching the original ymVST controller assignments.
// See https://www.preromanbritain.com/ymvst/midi.txt
// All parameter pointers are cached at construction - no string allocations.
void YmvstProcessor::handleMidiCC(int cc, int val)
{
    float scaled = static_cast<float>(val) / 127.0f;
    float boolVal = val >= 64 ? 1.0f : 0.0f;
    auto& c = cachedCCParams;

    auto setS = [scaled](juce::RangedAudioParameter* p) { if (p) p->setValueNotifyingHost(scaled); };
    auto setB = [boolVal](juce::RangedAudioParameter* p) { if (p) p->setValueNotifyingHost(boolVal); };

    switch (cc)
    {
        case 1:   setS(c.tremDepth); break;
        case 3:   setS(c.mainTune); break;
        case 5:   setS(c.portaRate); break;
        case 7:   setS(c.masterVol); break;
        case 9:   setS(c.noiseFreq); break;
        case 16:  setS(c.noiseFreq); break;
        case 17:  setS(c.wfLength); break;
        case 19:  setS(c.arpLength); break;
        case 20: case 21: case 22: case 23: case 24:
        case 25: case 26: case 27: case 28: case 29:
        {
            int step = (cc - 20) * 3;
            int wfVal = (val * 15) / 127;
            engine.setWaveformValue(step, wfVal);
            if (step + 1 < YmEngine::WAVEFORM_SIZE) engine.setWaveformValue(step + 1, wfVal);
            if (step + 2 < YmEngine::WAVEFORM_SIZE) engine.setWaveformValue(step + 2, wfVal);
            break;
        }
        case 30:  setS(c.nBendDepth); break;
        case 31:  setS(c.nBendSpeed); break;
        case 64:  setS(c.envPeriod); break;
        case 70:  setS(c.envShape); break;
        case 71:  setS(c.sBendSpeed); break;
        case 72:  setS(c.sBendDepth); break;
        case 75:  setB(c.wfOneShot); break;
        case 76:  setB(c.ch1Env); break;
        case 77:  setB(c.arpSync); break;
        case 78:  setB(c.sidOn); break;
        case 80:  setB(c.noiseOn); break;
        case 81:  setB(c.wfOn); break;
        case 82:  setB(c.ch1Env); break;
        case 83:  setB(c.arpOn); break;
        case 87:  setS(c.arpSpeed); break;
        case 88:  engine.setArpOffset(0, (val * 48 / 127) - 24); break;
        case 89:  engine.setArpOffset(1, (val * 48 / 127) - 24); break;
        case 90:  engine.setArpOffset(2, (val * 48 / 127) - 24); break;
        case 92:  setS(c.tremSpeed); break;
        case 94:  engine.setFineTune(1, (val * 100 / 127) - 50); break;
        case 95:  engine.setFineTune(2, (val * 100 / 127) - 50); break;
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
        set("ch" + ch + "_fine",  static_cast<float>(p.chFine[i]));
        set("ch" + ch + "_tone",  p.chTone[i] ? 1.f : 0.f);
        set("ch" + ch + "_noise", p.chNoise[i] ? 1.f : 0.f);
        set("ch" + ch + "_env",   p.chEnv[i] ? 1.f : 0.f);
    }

    set("main_tune",   static_cast<float>(p.mainTune));
    set("noise_on",    p.noiseOn ? 1.f : 0.f);
    set("noise_freq",  static_cast<float>(p.noiseFreq));
    set("env_shape",   static_cast<float>(p.envShape));
    set("env_period",  static_cast<float>(p.envPeriod));
    set("arp_on",      p.arpOn ? 1.f : 0.f);
    set("arp_sync",    p.arpSync ? 1.f : 0.f);
    set("arp_speed",   static_cast<float>(p.arpSpeed));
    set("arp_length",  static_cast<float>(p.arpLength));
    set("arp_t1",      static_cast<float>(p.arpT1));
    set("wf_on",       p.wfOn ? 1.f : 0.f);
    set("wf_oneshot",  p.wfOneShot ? 1.f : 0.f);
    set("wf_speed",    static_cast<float>(p.wfSpeed));
    set("wf_length",   static_cast<float>(p.wfLength));
    set("porta_on",    p.portaOn ? 1.f : 0.f);
    set("porta_rate",  static_cast<float>(p.portaRate));
    set("sbend_depth", static_cast<float>(p.sBendDepth));
    set("sbend_speed", static_cast<float>(p.sBendSpeed));
    set("nbend_depth", static_cast<float>(p.nBendDepth));
    set("nbend_speed", static_cast<float>(p.nBendSpeed));
    set("trem_depth",  static_cast<float>(p.tremDepth));
    set("trem_speed",  static_cast<float>(p.tremSpeed));
    set("sid_on",      p.sidOn ? 1.f : 0.f);
    set("poly_on",     p.polyOn ? 1.f : 0.f);

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

    // Save current preset index
    state.setProperty("currentPreset", currentPreset, nullptr);

    // Save waveform data
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

    // Restore preset index
    currentPreset = state.getProperty("currentPreset", 0);
    state.removeProperty("currentPreset", nullptr);

    // Restore waveform data
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
