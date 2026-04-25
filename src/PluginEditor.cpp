#include "PluginEditor.h"
#include "gui/RetroLookAndFeel.h"
#include "gui/BitmapFont.h"

YmvstEditor::YmvstEditor(YmvstProcessor& p)
    : AudioProcessorEditor(p), processor(p)
{
    setSize(800, 480);

    // Add all children
    std::initializer_list<juce::Component*> allChildren = {
        &ampWfLabel, &hwWfLabel, &noiseLabel, &arpLabel, &presetLabel, &controlsLabel,
        &waveformEditor, &scopeDisplay, &wfOnBtn, &wfOneShotBtn, &wfSpeed, &wfLength,
        &hwSelector, &envSpeed, &mainTune, &fineTune1, &fineTune2, &fineTune3,
        &noiseOnBtn, &noiseFreq,
        &ch1Btn, &ch2Btn, &ch3Btn,
        &arpOnBtn, &arpSyncBtn, &arpT1, &arpSpeed, &arpLength,
        &presetSelector, &helpBtn,
        &panicBtn, &polyBtn, &sidOnBtn, &portaRate, &sBendDepth, &sBendSpeed, &nBendDepth, &nBendSpeed,
        &tremDepth, &tremSpeed
    };
    for (auto* c : allChildren)
        addAndMakeVisible(c);

    // Set initial button states
    ch1Btn.setState(true);
    wfOnBtn.setState(true);
    wfOneShotBtn.setState(true);

    connectCallbacks();
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();
    startTimerHz(30);
    syncWidgetsToParams();
}

YmvstEditor::~YmvstEditor()
{
    stopTimer();
}

// Tracker-style keyboard layout (matches Renoise):
//  Lower octave (C-3):  Z=C  S=C# X=D  D=D# C=E  V=F  G=F# B=G  H=G# N=A  J=A# M=B
//  Upper octave (C-4):  Q=C  2=C# W=D  3=D# E=E  R=F  5=F# T=G  6=G# Y=A  7=A# U=B
//                        I=C5 9=C#5 O=D5 0=D#5 P=E5
void YmvstEditor::mouseDown(const juce::MouseEvent&)
{
    if (showingHelp) { showingHelp = false; repaint(); }
}

int YmvstEditor::keyToMidiNote(int keyCode)
{
    switch (keyCode)
    {
        // Lower octave (C-3, MIDI 48)
        case 'Z': return 48;  // C3
        case 'S': return 49;  // C#3
        case 'X': return 50;  // D3
        case 'D': return 51;  // D#3
        case 'C': return 52;  // E3
        case 'V': return 53;  // F3
        case 'G': return 54;  // F#3
        case 'B': return 55;  // G3
        case 'H': return 56;  // G#3
        case 'N': return 57;  // A3
        case 'J': return 58;  // A#3
        case 'M': return 59;  // B3
        // Upper octave (C-4, MIDI 60)
        case 'Q': return 60;  // C4
        case '2': return 61;  // C#4
        case 'W': return 62;  // D4
        case '3': return 63;  // D#4
        case 'E': return 64;  // E4
        case 'R': return 65;  // F4
        case '5': return 66;  // F#4
        case 'T': return 67;  // G4
        case '6': return 68;  // G#4
        case 'Y': return 69;  // A4
        case '7': return 70;  // A#4
        case 'U': return 71;  // B4
        // Extended (C-5)
        case 'I': return 72;  // C5
        case '9': return 73;  // C#5
        case 'O': return 74;  // D5
        case '0': return 75;  // D#5
        case 'P': return 76;  // E5
        default: return -1;
    }
}

bool YmvstEditor::keyPressed(const juce::KeyPress& key)
{
    if (showingHelp)
    {
        showingHelp = false;
        repaint();
        return true;
    }

    if (key == juce::KeyPress::escapeKey)
    {
        const juce::SpinLock::ScopedLockType lock(processor.editorMidiLock);
        processor.editorMidiBuffer.addEvent(juce::MidiMessage::allNotesOff(1), 0);
        keysDown.clear();
        return true;
    }

    int keyCode = key.getTextCharacter();
    if (keyCode >= 'a' && keyCode <= 'z')
        keyCode -= 32; // to uppercase

    int note = keyToMidiNote(keyCode);
    if (note >= 0 && keysDown.find(keyCode) == keysDown.end())
    {
        keysDown.insert(keyCode);
        // Route through MIDI buffer (thread-safe, processed on audio thread)
        const juce::SpinLock::ScopedLockType lock(processor.editorMidiLock);
        processor.editorMidiBuffer.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)100), 0);
        return true;
    }
    return false;
}

bool YmvstEditor::keyStateChanged(bool /*isKeyDown*/)
{
    std::vector<int> released;
    for (int keyCode : keysDown)
    {
        if (!juce::KeyPress::isKeyCurrentlyDown(keyCode) &&
            !juce::KeyPress::isKeyCurrentlyDown(keyCode + 32))
        {
            released.push_back(keyCode);
        }
    }
    for (int keyCode : released)
    {
        keysDown.erase(keyCode);
        int note = keyToMidiNote(keyCode);
        if (note >= 0)
        {
            const juce::SpinLock::ScopedLockType lock(processor.editorMidiLock);
            processor.editorMidiBuffer.addEvent(juce::MidiMessage::noteOff(1, note), 0);
        }
    }
    return !released.empty();
}

void YmvstEditor::timerCallback()
{
    // Read scope samples from engine's lock-free buffer (written on audio thread)
    auto& engine = processor.getEngine();
    int wp = engine.scopeWritePos.load(std::memory_order_acquire);
    for (int i = 0; i < ScopeDisplay::BUFFER_SIZE; ++i)
    {
        int idx = (wp - ScopeDisplay::BUFFER_SIZE + i + YmEngine::SCOPE_BUFFER_SIZE) % YmEngine::SCOPE_BUFFER_SIZE;
        scopeDisplay.pushSample(engine.scopeBuffer[idx]);
    }
}

void YmvstEditor::paint(juce::Graphics& g)
{
    g.fillAll(RetroColours::background);

    // Waveform control labels
    BitmapFont::drawText(g, "SPEED", 210, 164, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "LENGTH", 315, 164, 1, RetroColours::textWhite);

    // Noise labels
    BitmapFont::drawText(g, "NOISE FRE.", 88, 236, 1, RetroColours::textWhite);

    // Hardware envelope section labels
    BitmapFont::drawText(g, "SPEED", 680, 48, 1, RetroColours::textWhite);

    // Labels above the channel/tuning row
    BitmapFont::drawText(g, "CH1", 412, 116, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "CH2", 458, 116, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "CH3", 504, 116, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "FINE TUNE", 568, 116, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "MAIN", 738, 116, 1, RetroColours::textWhite);

    // Arpeggiator labels
    BitmapFont::drawText(g, "T1", 600, 190, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "SPEED", 620, 230, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "ARP LEN", 700, 230, 1, RetroColours::textWhite);

    // Preset name
    if (processor.getCurrentProgram() >= 0 && processor.getCurrentProgram() < NUM_FACTORY_PRESETS)
        BitmapFont::drawText(g, FACTORY_PRESETS[processor.getCurrentProgram()].name,
                             80, 299, 1, RetroColours::textCyan);

    // Controls section labels
    BitmapFont::drawText(g, "PORTAMENTO", 255, 340, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "SOUND", 400, 340, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "BEND DEPTH", 395, 348, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "BEND SPEED", 395, 372, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "NOISE+OO", 540, 340, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "BEND DEPTH", 535, 348, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "TREM.DEPTH", 680, 336, 1, RetroColours::textWhite);
    BitmapFont::drawText(g, "TREM.SPEED", 680, 372, 1, RetroColours::textWhite);

    // Version label
    BitmapFont::drawText(g, "YM-VST V1.0", 10, 466, 1, RetroColours::textCyan);

    // Help overlay (solid background so text is legible)
    if (showingHelp)
    {
        g.setColour(RetroColours::background);
        g.fillRect(getLocalBounds());

        int y = 20;
        int x = 20;
        int dy = 12;
        auto line = [&](const char* text, juce::Colour col = RetroColours::textWhite) {
            BitmapFont::drawText(g, text, x, y, 1, col);
            y += dy;
        };

        line("YM-VST HELP", RetroColours::textCyan);
        y += 4;
        line("KEYBOARD");
        line("  QWERTY KEYS  Play notes (tracker layout)", RetroColours::textCyan);
        line("  Z-M = C3-B3   Q-U = C4-B4   I-P = C5-E5", RetroColours::textCyan);
        line("  ESCAPE       Panic - silence all", RetroColours::textCyan);
        y += 4;
        line("SPINNERS (< VALUE >)");
        line("  CLICK        +-1", RetroColours::textCyan);
        line("  SHIFT+CLICK  +-10", RetroColours::textCyan);
        line("  CMD+CLICK    +-100", RetroColours::textCyan);
        line("  RIGHT-CLICK  +-10", RetroColours::textCyan);
        y += 4;
        line("SECTIONS");
        line("  AMPLITUDE WF  Draw volume envelope, 1SHOT/LOOP", RetroColours::textCyan);
        line("  HW ENVELOPE   Chip envelope shapes (enable per ch)", RetroColours::textCyan);
        line("  ARPEGGIATOR   Cycle pitch offsets at speed", RetroColours::textCyan);
        line("  ARP SYNC      Reset arp to step 0 on new note", RetroColours::textCyan);
        line("  SID MODE      Hard-sync ch1 to ch0 (metallic)", RetroColours::textCyan);
        line("  POLY/MONO     3-voice polyphony or mono w/ legato", RetroColours::textCyan);
        line("  FINE TUNE     Per-channel detune in cents", RetroColours::textCyan);
        y += 4;
        line("CLICK ANYWHERE TO CLOSE", RetroColours::indicatorOn);
    }
}

void YmvstEditor::resized()
{
    // Section borders
    ampWfLabel.setBounds(8, 8, 380, 200);
    hwWfLabel.setBounds(396, 8, 396, 155);
    noiseLabel.setBounds(8, 216, 380, 55);
    arpLabel.setBounds(396, 170, 396, 100);
    presetLabel.setBounds(8, 278, 380, 42);
    controlsLabel.setBounds(8, 328, 784, 110);

    // Amplitude waveform editor + scope
    waveformEditor.setBounds(18, 24, 180, 140);
    scopeDisplay.setBounds(206, 24, 172, 140);

    // Waveform controls
    wfOnBtn.setBounds(18, 174, 45, 16);
    wfOneShotBtn.setBounds(70, 174, 60, 16);
    wfSpeed.setBounds(206, 174, 70, 16);
    wfLength.setBounds(310, 174, 70, 16);

    // Hardware envelope section
    hwSelector.setBounds(406, 24, 240, 80);
    envSpeed.setBounds(680, 60, 100, 16);

    // Channel on/off + tuning row
    ch1Btn.setBounds(406, 126, 40, 16);
    ch2Btn.setBounds(452, 126, 40, 16);
    ch3Btn.setBounds(498, 126, 40, 16);
    fineTune1.setBounds(560, 126, 48, 16);
    fineTune2.setBounds(612, 126, 48, 16);
    fineTune3.setBounds(664, 126, 48, 16);
    mainTune.setBounds(732, 126, 52, 16);

    // Noise
    noiseOnBtn.setBounds(18, 234, 55, 16);
    noiseFreq.setBounds(170, 234, 90, 16);

    // Arpeggiator
    arpOnBtn.setBounds(406, 188, 55, 16);
    arpSyncBtn.setBounds(406, 210, 100, 16);
    arpT1.setBounds(620, 188, 60, 16);
    arpSpeed.setBounds(620, 240, 60, 16);
    arpLength.setBounds(700, 240, 60, 16);

    // Preset
    presetSelector.setBounds(18, 296, 55, 16);

    // Controls section
    panicBtn.setBounds(18, 346, 60, 16);
    polyBtn.setBounds(86, 346, 65, 16);
    helpBtn.setBounds(160, 346, 50, 16);
    sidOnBtn.setBounds(18, 370, 75, 16);
    portaRate.setBounds(260, 370, 80, 16);
    sBendDepth.setBounds(400, 356, 70, 16);
    sBendSpeed.setBounds(400, 380, 70, 16);
    nBendDepth.setBounds(540, 356, 70, 16);
    nBendSpeed.setBounds(540, 380, 70, 16);
    tremDepth.setBounds(680, 346, 70, 16);
    tremSpeed.setBounds(680, 380, 70, 16);
}

void YmvstEditor::connectCallbacks()
{
    auto& params = processor.getParams();
    auto setParam = [&](const juce::String& id, float val) {
        if (auto* p = params.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(val));
    };

    ch1Btn.onStateChanged = [&, setParam](bool on) { setParam("ch1_on", on ? 1.f : 0.f); };
    ch2Btn.onStateChanged = [&, setParam](bool on) { setParam("ch2_on", on ? 1.f : 0.f); };
    ch3Btn.onStateChanged = [&, setParam](bool on) { setParam("ch3_on", on ? 1.f : 0.f); };

    noiseOnBtn.onStateChanged = [&, setParam](bool on) { setParam("noise_on", on ? 1.f : 0.f); };
    noiseFreq.onValueChanged = [&, setParam](int v) { setParam("noise_freq", (float)v); };

    mainTune.onValueChanged = [&, setParam](int v) { setParam("main_tune", (float)v); };
    fineTune1.onValueChanged = [&, setParam](int v) { setParam("ch1_fine", (float)v); };
    fineTune2.onValueChanged = [&, setParam](int v) { setParam("ch2_fine", (float)v); };
    fineTune3.onValueChanged = [&, setParam](int v) { setParam("ch3_fine", (float)v); };

    hwSelector.onShapeChanged = [&, setParam](int s) { setParam("env_shape", (float)s); };
    hwSelector.chEnv1.onStateChanged = [&, setParam](bool on) { setParam("ch1_env", on ? 1.f : 0.f); };
    hwSelector.chEnv2.onStateChanged = [&, setParam](bool on) { setParam("ch2_env", on ? 1.f : 0.f); };
    hwSelector.chEnv3.onStateChanged = [&, setParam](bool on) { setParam("ch3_env", on ? 1.f : 0.f); };
    envSpeed.onValueChanged = [&, setParam](int v) { setParam("env_period", (float)v); };

    arpOnBtn.onStateChanged = [&, setParam](bool on) { setParam("arp_on", on ? 1.f : 0.f); };
    arpSyncBtn.onStateChanged = [&, setParam](bool on) { setParam("arp_sync", on ? 1.f : 0.f); };
    arpT1.onValueChanged = [&, setParam](int v) { setParam("arp_t1", (float)v); };
    arpSpeed.onValueChanged = [&, setParam](int v) { setParam("arp_speed", (float)v); };
    arpLength.onValueChanged = [&, setParam](int v) { setParam("arp_length", (float)v); };

    wfOnBtn.onStateChanged = [&, setParam](bool on) { setParam("wf_on", on ? 1.f : 0.f); };
    wfOneShotBtn.onStateChanged = [&, setParam](bool on) { setParam("wf_oneshot", on ? 1.f : 0.f); };
    wfSpeed.onValueChanged = [&, setParam](int v) { setParam("wf_speed", (float)v); };
    wfLength.onValueChanged = [&, setParam](int v) { setParam("wf_length", (float)v); };

    waveformEditor.onValueChanged = [this](int idx, int val) {
        // Single int write is naturally atomic on modern architectures
        processor.getEngine().setWaveformValue(idx, val);
    };

    presetSelector.onValueChanged = [this](int v) {
        processor.loadPreset(v);
        syncWidgetsToParams();
        // Update waveform editor display
        auto& wf = processor.getEngine().getWaveformData();
        for (int i = 0; i < YmEngine::WAVEFORM_SIZE; ++i)
            waveformEditor.setValue(i, wf[i]);
        repaint();
    };
    helpBtn.onStateChanged = [this](bool) {
        showingHelp = !showingHelp;
        helpBtn.setState(false);
        repaint();
    };
    panicBtn.onStateChanged = [this](bool) {
        // Send all-notes-off via MIDI buffer (processed on audio thread)
        const juce::SpinLock::ScopedLockType lock(processor.editorMidiLock);
        processor.editorMidiBuffer.addEvent(juce::MidiMessage::allNotesOff(1), 0);
        panicBtn.setState(false); // Reset to unpressed
    };
    polyBtn.onStateChanged = [&, setParam](bool on) { setParam("poly_on", on ? 1.f : 0.f); };
    sidOnBtn.onStateChanged = [&, setParam](bool on) { setParam("sid_on", on ? 1.f : 0.f); };
    portaRate.onValueChanged = [&, setParam](int v) { setParam("porta_rate", (float)v); };
    sBendDepth.onValueChanged = [&, setParam](int v) { setParam("sbend_depth", (float)v); };
    sBendSpeed.onValueChanged = [&, setParam](int v) { setParam("sbend_speed", (float)v); };
    nBendDepth.onValueChanged = [&, setParam](int v) { setParam("nbend_depth", (float)v); };
    nBendSpeed.onValueChanged = [&, setParam](int v) { setParam("nbend_speed", (float)v); };
    tremDepth.onValueChanged = [&, setParam](int v) { setParam("trem_depth", (float)v); };
    tremSpeed.onValueChanged = [&, setParam](int v) { setParam("trem_speed", (float)v); };
}

void YmvstEditor::syncWidgetsToParams()
{
    // Called to sync widget states from parameter values (e.g., after state restore)
    auto& params = processor.getParams();
    auto getVal = [&](const juce::String& id) -> float {
        if (auto* p = params.getRawParameterValue(id)) return *p;
        return 0.f;
    };

    // Channels
    ch1Btn.setState(getVal("ch1_on") > 0.5f);
    ch2Btn.setState(getVal("ch2_on") > 0.5f);
    ch3Btn.setState(getVal("ch3_on") > 0.5f);
    fineTune1.setValue(static_cast<int>(getVal("ch1_fine")));
    fineTune2.setValue(static_cast<int>(getVal("ch2_fine")));
    fineTune3.setValue(static_cast<int>(getVal("ch3_fine")));

    // Hardware envelope
    hwSelector.setSelectedShape(static_cast<int>(getVal("env_shape")));
    hwSelector.chEnv1.setState(getVal("ch1_env") > 0.5f);
    hwSelector.chEnv2.setState(getVal("ch2_env") > 0.5f);
    hwSelector.chEnv3.setState(getVal("ch3_env") > 0.5f);
    envSpeed.setValue(static_cast<int>(getVal("env_period")));

    // Tuning
    mainTune.setValue(static_cast<int>(getVal("main_tune")));

    // Noise
    noiseOnBtn.setState(getVal("noise_on") > 0.5f);
    noiseFreq.setValue(static_cast<int>(getVal("noise_freq")));

    // Arpeggiator
    arpOnBtn.setState(getVal("arp_on") > 0.5f);
    arpSyncBtn.setState(getVal("arp_sync") > 0.5f);
    arpT1.setValue(static_cast<int>(getVal("arp_t1")));
    arpSpeed.setValue(static_cast<int>(getVal("arp_speed")));
    arpLength.setValue(static_cast<int>(getVal("arp_length")));

    // Waveform
    wfOnBtn.setState(getVal("wf_on") > 0.5f);
    wfOneShotBtn.setState(getVal("wf_oneshot") > 0.5f);
    wfSpeed.setValue(static_cast<int>(getVal("wf_speed")));
    wfLength.setValue(static_cast<int>(getVal("wf_length")));

    // Bottom row
    sidOnBtn.setState(getVal("sid_on") > 0.5f);
    polyBtn.setState(getVal("poly_on") > 0.5f);
    portaRate.setValue(static_cast<int>(getVal("porta_rate")));
    sBendDepth.setValue(static_cast<int>(getVal("sbend_depth")));
    sBendSpeed.setValue(static_cast<int>(getVal("sbend_speed")));
    nBendDepth.setValue(static_cast<int>(getVal("nbend_depth")));
    nBendSpeed.setValue(static_cast<int>(getVal("nbend_speed")));
    tremDepth.setValue(static_cast<int>(getVal("trem_depth")));
    tremSpeed.setValue(static_cast<int>(getVal("trem_speed")));

    presetSelector.setValue(processor.getCurrentProgram());
}
