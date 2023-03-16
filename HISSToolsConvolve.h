
#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include <array>
#include <condition_variable>
#include <thread>

#include "HISSTools_Controls.hpp"
#include "HISSTools_VU_Ballistics.hpp"
#include "Convolver.h"
#include "FileScheme.hpp"


const int kNumPrograms = 1;

enum EParams
{
  kDryGain = 0,
  kWetGain,
  kOutputSelect,
  kNumParams       // the last element is used to store the total number of parameters
};

using namespace iplug;
using namespace igraphics;

class LEDSender
{
  struct LEDValues
  {
    std::array<int, MAX_CHANNELS> mStates;
  };
  
public:
  
  LEDSender(int controlTag, bool horizontal) : mControlTag(controlTag), mHorizontal(horizontal), mQueue(32) {}
  
  void Set(const std::array<int, MAX_CHANNELS>& states)
  {
    mQueue.Push(LEDValues{ states });
  }
  
  void UpdateControl(IGraphics *g)
  {
    while (g && mQueue.ElementsAvailable())
    {
      LEDValues v;
      mQueue.Pop(v);

      HISSTools_Matrix *matrix = g->GetControlWithTag(mControlTag)->As<HISSTools_Matrix>();
      
      if (mHorizontal)
      {
        for (int i = 0 ; i < MAX_CHANNELS; i++)
          matrix->SetState(i, 0, v.mStates[i]);
      }
      else
      {
        for (int i = 0 ; i < MAX_CHANNELS; i++)
          matrix->SetState(0, i, v.mStates[i]);
      }
    }
  }
  
  void Reset()
  {
    LEDValues v;
    
    while (mQueue.ElementsAvailable())
      mQueue.Pop(v);
  }
  
private:
  
  int mControlTag;
  bool mHorizontal;
  IPlugQueue<LEDValues> mQueue;
};

class HISSToolsConvolve : public Plugin
{
public:
  
  HISSToolsConvolve(const InstanceInfo &info);
  ~HISSToolsConvolve();
  
  // Implement these if your audio or GUI logic requires doing something, when params change or when audio processing stops / starts.
  
  void OnIdle() override;
  void OnReset() override;
  void OnParamChange(int paramIdx, EParamSource source, int sampleOffset) override;
  void OnParamChangeUI(int paramIdx, EParamSource source) override;
  void ProcessBlock(double** inputs, double** outputs, int nFrames) override;

  bool SerializeState(IByteChunk& chunk) const override;
  int UnserializeState(const IByteChunk& chunk, int pos) override;
  
  void CheckConnections();
  
  void UpdateBaseName();
  void GUIUpdateFileDisplay();
  
  void LoadIRs();
    
//  bool IsInChannelConnected(int idx) { return IsChannelConnected(kInput, idx); }
//  bool IsOutChannelConnected(int idx) { return IsChannelConnected(kOutput, idx); }

  void SelectFile(const char *file);
  void IncrementChan(int xPos, int yPos);
  void FlipMute(int xPos, int yPos);
  void SetFile(int xPos, int yPos, const char *path);
  
private:
  
  IGraphics* CreateGraphics() override;
  void LayoutUI(IGraphics* pGraphics) override;
  
  void GUIUpdateDials();
  
  // Convolution Engine
  
  HISSTools::Convolver mConvolver;
    
  // Meter Ballistics
  
  HISSTools_VU_Ballistics mIBallistics;
  HISSTools_VU_Ballistics mOBallistics;
  
  // Meter Senders
  
  HISSTools_VUMeter::Sender mIVUSender;
  HISSTools_VUMeter::Sender mOVUSender;
  LEDSender mILEDSender;
  LEDSender mOLEDSender;
  
  // Parameters
  
  double mTargetDryGain;
  double mTargetWetGain;
  double mLastDryGain;
  double mLastWetGain;
  
  int mOutputSelect;
  int mCurrentIChans;
  int mCurrentOChans;
  
  bool mUpdateDisplay;
  
  // File Info
  
  FileList mFiles;
  
  WDL_String mBaseName;
  
public:
  
  // Threading
  
  std::thread mLoadThread;
  std::condition_variable_any mLoadEvent;
  
  bool mThreadExiting;
};
