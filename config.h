#define PLUG_NAME "HISSTools Convolve"
#define PLUG_MFR "Alex Harker"
#define PLUG_VERSION_HEX 0x00010000
#define PLUG_VERSION_STR "1.0.0"
#define PLUG_UNIQUE_ID 'HTCn'
#define PLUG_MFR_ID 'AHAR'
#define PLUG_URL_STR "www.alexanderjharker.co.uk"
#define PLUG_EMAIL_STR "A.Harker@hud.ac.uk"
#define PLUG_COPYRIGHT_STR  "Copyright 2018-2022 Alex Harker"
#define PLUG_CLASS_NAME HISSToolsConvolve

#define BUNDLE_NAME "HISSToolsConvolve"
#define BUNDLE_MFR "AlexHarker"
#define BUNDLE_DOMAIN "com"

#define AUV2_ENTRY HISSToolsConvolve_Entry
#define AUV2_ENTRY_STR "HISSToolsConvolve_Entry"
#define AUV2_FACTORY HISSToolsConvolve_Factory
#define AUV2_VIEW_CLASS HISSToolsConvolve_View
#define AUV2_VIEW_CLASS_STR "HISSToolsConvolve_View"

#define APP_SIGNAL_VECTOR_SIZE 64

#define PLUG_WIDTH   340
#define PLUG_HEIGHT  480
#define PLUG_HOST_RESIZE 1

// ProTools stuff
#define PLUG_MFR_DIGI "AHarker\nAHarker\nAHar\n"
#define PLUG_NAME_DIGI "AHConvN\nIPEF"
#define EFFECT_TYPE_DIGI "Effect" // valid options "None" "EQ" "Dynamics" "PitchShift" "Reverb" "Delay" "Modulation" "Harmonic" "NoiseReduction" "Dither" "SoundField" "Effect" instrument determined by PLUG _IS _INST

#define MAX_CHANNELS 8
#define PLUG_CHANNEL_IO "1-1 2-2 3-3 4-4 5-5 6-6 7-7 8-8"
#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 0
#define PLUG_DOES_MIDI_OUT 0
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 1
#define PLUG_FPS 60
#define PLUG_HAS_UI 1
#define PLUG_SHARED_RESOURCES 0

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#ifdef SA_API
  #ifndef OS_IOS
    #include "app_wrapper/app_resource.h"
  #endif
#endif

// vst3 stuff
#define MFR_URL "www.olilarkin.co.uk"
#define MFR_EMAIL "spam@me.com"
#define EFFECT_TYPE_VST3 kFx

/* kFxAnalyzer, kFxDelay, kFxDistortion, kFxDynamics, kFxEQ, kFxFilter,
kFx, kFxInstrument, kFxInstrumentExternal, kFxSpatial, kFxGenerator,
kFxMastering, kFxModulation, kFxPitchShift, kFxRestoration, kFxReverb,
kFxSurround, kFxTools, kInstrument, kInstrumentDrum, kInstrumentSampler,
kInstrumentSynth, kInstrumentSynthSample, kInstrumentExternal, kSpatial,
kSpatialFx, kOnlyRealTime, kOnlyOfflineProcess, kMono, kStereo,
kSurround
*/
