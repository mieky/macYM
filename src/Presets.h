#pragma once
#include "YmEngine.h"
#include <array>

struct Preset
{
    const char* name;

    // Channels (per ch: on, fine, tone, noise, env)
    bool chOn[3];
    int chFine[3];
    bool chTone[3];
    bool chNoise[3];
    bool chEnv[3];

    int mainTune;
    bool noiseOn;
    int noiseFreq;
    int envShape;
    int envPeriod;

    bool arpOn;
    bool arpSync;
    int arpSpeed;
    int arpLength;
    int arpT1;

    bool wfOn;
    bool wfOneShot;
    int wfSpeed;
    int wfLength;
    std::array<int, YmEngine::WAVEFORM_SIZE> waveform;

    bool portaOn;
    int portaRate;
    int sBendDepth;
    int sBendSpeed;
    int nBendDepth;
    int nBendSpeed;
    int tremDepth;
    int tremSpeed;
    bool sidOn;
    bool polyOn;
};

// Waveform helper: decay from 15 to 0
static constexpr std::array<int, 32> WF_DECAY = {
    15,14,13,12,11,10,9,8, 7,6,5,5,4,4,3,3, 2,2,2,1,1,1,1,1, 0,0,0,0,0,0,0,0
};
// Flat max
static constexpr std::array<int, 32> WF_FLAT = {
    15,15,15,15,15,15,15,15, 15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15, 15,15,15,15,15,15,15,15
};
// Short pluck
static constexpr std::array<int, 32> WF_PLUCK = {
    15,12,9,7,5,4,3,2, 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
};
// Slow swell
static constexpr std::array<int, 32> WF_SWELL = {
    0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15, 15,14,13,12,11,10,9,8, 7,6,5,4,3,2,1,0
};
// Tremolo pulse
static constexpr std::array<int, 32> WF_PULSE = {
    15,15,15,15,0,0,0,0, 15,15,15,15,0,0,0,0, 15,15,15,15,0,0,0,0, 15,15,15,15,0,0,0,0
};
// Kick: sharp punch then silence
static constexpr std::array<int, 32> WF_KICK = {
    15,15,14,12,10,8,6,4, 2,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
};
// Snare: noise burst with quick decay
static constexpr std::array<int, 32> WF_SNARE = {
    15,14,13,11,9,7,6,5, 4,3,2,2,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
};
// Hihat: very short burst
static constexpr std::array<int, 32> WF_HIHAT = {
    15,10,6,3,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
};
// Sub bass: slow attack, sustained
static constexpr std::array<int, 32> WF_SUB = {
    8,10,12,14,15,15,15,15, 15,15,14,14,13,13,12,12, 11,11,10,10,9,9,8,8, 7,6,5,4,3,2,1,0
};
// Wobble: stepped volume for chiptune vibrato feel
static constexpr std::array<int, 32> WF_WOBBLE = {
    15,15,12,12,15,15,12,12, 15,15,12,12,15,15,12,12, 10,10,8,8,10,10,8,8, 6,6,4,4,2,2,0,0
};
// Stab: very short full-volume hit
static constexpr std::array<int, 32> WF_STAB = {
    15,15,15,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
};

static const Preset FACTORY_PRESETS[] = {
    {
        "Chip Pluck",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Square Lead",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        false, false, 1, 32, WF_FLAT,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Noise Hit",
        {true,false,false}, {0,0,0}, {false,false,false}, {true,false,false}, {false,false,false},
        0, true, 8, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 10, WF_PLUCK,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Arp Major",
        {true,true,true}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        true, true, 2, 4, 4,
        true, false, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Arp Minor",
        {true,true,true}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        true, true, 2, 4, 3,
        true, false, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "SID Sync",
        {true,true,false}, {0,5,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, true, false
    },
    {
        "Poly Pad",
        {true,true,true}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, false, 2, 32, WF_SWELL,
        false, 4, 0, 15, 0, 5, 0, 4, false, true
    },
    {
        "Laser",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 16, WF_PLUCK,
        false, 4, 30, 8, 0, 5, 0, 4, false, false
    },
    {
        "Thick Bass",
        {true,true,false}, {0,-7,0}, {true,true,true}, {false,false,false}, {false,false,false},
        -12, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    // --- Kicks ---
    {
        "Kick Punch",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        -24, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 10, WF_KICK,
        false, 4, 40, 4, 0, 5, 0, 4, false, false
    },
    {
        "Kick Deep",
        {true,true,false}, {0,0,0}, {true,true,true}, {true,false,false}, {false,false,false},
        -24, true, 2, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 12, WF_KICK,
        false, 4, 50, 3, 0, 5, 0, 4, false, false
    },
    // --- Snares ---
    {
        "Snare Tight",
        {true,false,false}, {0,0,0}, {true,true,true}, {true,false,false}, {false,false,false},
        0, true, 12, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 16, WF_SNARE,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Snare Noise",
        {true,false,false}, {0,0,0}, {false,false,false}, {true,false,false}, {false,false,false},
        0, true, 6, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 20, WF_SNARE,
        false, 4, 0, 15, 5, 8, 0, 4, false, false
    },
    // --- Hihats ---
    {
        "Hihat Closed",
        {true,false,false}, {0,0,0}, {false,false,false}, {true,false,false}, {false,false,false},
        0, true, 20, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 6, WF_HIHAT,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Hihat Open",
        {true,false,false}, {0,0,0}, {false,false,false}, {true,false,false}, {false,false,false},
        0, true, 22, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 18, WF_DECAY,
        false, 4, 0, 15, 3, 10, 0, 4, false, false
    },
    // --- Basses ---
    {
        "Sub Bass",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        -12, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 2, 32, WF_SUB,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Buzz Bass",
        {true,true,false}, {0,12,0}, {true,true,true}, {false,false,false}, {false,false,false},
        -12, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, true, false
    },
    {
        "Wobble Bass",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        -12, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, false, 1, 32, WF_WOBBLE,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    // --- Leads ---
    {
        "Saw Lead",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {true,false,false},
        0, false, 15, 12, 400,
        false, true, 1, 1, 0,
        false, false, 1, 32, WF_FLAT,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Vibrato Lead",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        false, false, 1, 32, WF_FLAT,
        false, 4, 0, 15, 0, 5, 3, 8, false, false
    },
    {
        "Detuned Lead",
        {true,true,true}, {0,3,-3}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Portamento Lead",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        false, false, 1, 32, WF_FLAT,
        true, 8, 0, 15, 0, 5, 0, 4, false, false
    },
    // --- FX ---
    {
        "Zap",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        12, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 8, WF_STAB,
        false, 4, -50, 2, 0, 5, 0, 4, false, false
    },
    {
        "Explosion",
        {true,true,true}, {0,0,0}, {false,false,false}, {true,true,true}, {false,false,false},
        0, true, 3, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 8, 4, 0, 4, false, false
    },
    {
        "Powerup",
        {true,false,false}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 2, 32, WF_DECAY,
        false, 4, -30, 12, 0, 5, 0, 4, false, false
    },
    // --- Chords ---
    {
        "Arp Dim",
        {true,true,true}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        true, true, 2, 4, 3,
        true, false, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, false
    },
    {
        "Poly Pluck",
        {true,true,true}, {0,0,0}, {true,true,true}, {false,false,false}, {false,false,false},
        0, false, 15, 0, 1000,
        false, true, 1, 1, 0,
        true, true, 1, 32, WF_DECAY,
        false, 4, 0, 15, 0, 5, 0, 4, false, true
    },
};

static constexpr int NUM_FACTORY_PRESETS = sizeof(FACTORY_PRESETS) / sizeof(FACTORY_PRESETS[0]);
