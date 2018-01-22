
#ifndef __AHCONVN_H__
#define __AHCONVN_H__

#include "IPlug_include_in_plug_hdr.h"
#include "Convolver.h"
#include "HISSTools_Controls.hpp"
#include "HISSTools_VU_Ballistics.hpp"
#include "FileScheme.hpp"

#define GUI_WIDTH   340
#define GUI_HEIGHT	480

// An enumerated list for all the parameters of the plugin

enum EParams 
{
	kDryGain,
	kWetGain,
	kOutputSelect,
	kFileSelect,      // dummy for file stuff (automation off)
	kMatrixControl,   // dummy for matrix stuff (automation off)
	kNumParams,       // the last element is used to store the total number of parameters
};

class AHConvN : public IPlug
{
public:
	
	AHConvN(IPlugInstanceInfo instanceInfo);
	~AHConvN();
	
	// Implement these if your audio or GUI logic requires doing something, when params change or when audio processing stops / starts.
	
	void OnReset();
	
	void CheckConnections(double** inputs = NULL, double** outputs = NULL);
	
	void UpdateBaseName();
	void UpdateFileDisplay();
	
	void LoadIRs();
	
  void OnParamChange(int paramIdx);//, ParamChangeSource source);	
	
	void ProcessBlock(double** inputs, double** outputs, int nFrames);
	
	bool SerializeState(IByteChunk& pChunk);
	int UnserializeState(IByteChunk& pChunk, int startPos);
	
	
private:
	
	// Convolution Engine
	
  HISSTools::Convolver mConvolver;
	
	// Controls and Drawing
	
	HISSTools_Dial *mDryGainDial;
	HISSTools_Dial *mWetGainDial;
	HISSTools_FileSelector *mFileDialog;
	HISSTools_TextBlock *mFileName;
	HISSTools_TextBlock *mFileChan;
	HISSTools_MeterTest *mIMeter;
	HISSTools_MeterTest *mOMeter;
	HISSTools_Matrix *mMatrix;
	HISSTools_Matrix *mILEDs;
	HISSTools_Matrix *mOLEDs;
	HISSTools_VecLib *mVecDraw;
	
	HISSTools_VU_Ballistics mIBallistics;
	HISSTools_VU_Ballistics mOBallistics;
	
	// Parameters
	
	double mTargetDryGain;
	double mTargetWetGain;
	double mLastDryGain;
	double mLastWetGain;
	
	int mOutputSelect;
	int mCurrentIChans;
	int mCurrentOChans;
	
	int mXPos;
	int mYPos;
	
	// File Info
	
	FileList mFiles;
	
	WDL_String mBaseName;
	
public:
	
	// Threading
	
	HANDLE WINAPI mLoadThread;
	HANDLE WINAPI mLoadEvent;
	
	bool mThreadExiting;
};

////////////////////////////////////////

#endif





