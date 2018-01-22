#define PLUG_MFR "AHarker"
#define PLUG_NAME "AHConvN"

#define PLUG_CLASS_NAME AHConvN

#define BUNDLE_DOMAIN "com"
#define BUNDLE_MFR "AHarker"
#define BUNDLE_NAME "AHConvN"

#define PLUG_ENTRY AHConvN_Entry
#define PLUG_VIEW_ENTRY AHConvN_ViewEntry

#define PLUG_ENTRY_STR "AHConvN_Entry"
#define PLUG_VIEW_ENTRY_STR "AHConvN_ViewEntry"

#define VIEW_CLASS AHConvN_View
#define VIEW_CLASS_STR "AHConvN_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VERSION_HEX 0x00010000
#define VST3_VER_STR "1.0.0"

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes. At least one capital letter
#define PLUG_UNIQUE_ID 'AhNC'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'AHar'

// ProTools stuff
#define PLUG_MFR_DIGI "AHarker\nAHarker\nAHar\n"
#define PLUG_NAME_DIGI "AHConvN\nIPEF"
#define EFFECT_TYPE_DIGI "Effect" // valid options "None" "EQ" "Dynamics" "PitchShift" "Reverb" "Delay" "Modulation" "Harmonic" "NoiseReduction" "Dither" "SoundField" "Effect" instrument determined by PLUG _IS _INST


#define MAX_CHANNELS 8
#define PLUG_CHANNEL_IO "1-1 2-2 3-3 4-4 5-5 6-6 7-7 8-8"
#define PLUG_LATENCY 0
#define PLUG_IS_INSTRUMENT 0

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 0

#define PLUG_DOES_STATE_CHUNKS 1

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
