
#include "AHConvN.h"
#include "IAudioFile.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include <math.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// LOADING THREAD ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD LoadingThread(LPVOID ConvPlugParam)
{
	AHConvN *ConvPlug = (AHConvN *) ConvPlugParam;
	
	while (TRUE)
	{
		WaitForSingleObject(ConvPlug->mLoadEvent, INFINITE);
		
		if (ConvPlug->mThreadExiting == TRUE)
			break;
		
		ConvPlug->LoadIRs();
	}
	
	ConvPlug->mThreadExiting = FALSE;
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The number of presets / programs

const int kNumPrograms = 1;

AHConvN::AHConvN(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mConvolver(8, 8, HISSTools::Convolver::kLatencyZero)
{
	TRACE;
	
	// Define parameter ranges, display units, labels.
	
	GetParam(kDryGain)->InitDouble("Dry Gain", 0, -60, 20, 0.1, "dB");
	GetParam(kWetGain)->InitDouble("Wet Gain", 0, -60, 20, 0.1, "dB");
	GetParam(kOutputSelect)->InitEnum("Output Select", 2 , 3);
	
	GetParam(kFileSelect)->InitBool("Load Multiple", false, "");
	GetParam(kMatrixControl)->InitBool("Matrix Control", false, "");
	
	GetParam(kFileSelect)->SetCanAutomate(false);
	GetParam(kMatrixControl)->SetCanAutomate(false);
    
	// Allocate Memory
	
	IGraphics* pGraphics = MakeGraphics(this, GUI_WIDTH, GUI_HEIGHT);
    
	IColor bgrb = IColor(255, 185, 195, 205);
	//IColor bgrb = IColor(255, 135, 135, 135);
	//IColor bgrb = IColor(255, 55, 75, 85);
	
	mVecDraw = new HISSTools_LICE_Vec_Lib(pGraphics->GetDrawBitmap());
	
	mFileDialog = new HISSTools_FileSelector(this, kFileSelect, mVecDraw, 110, 205, 150, 0, kFileOpen, "", "wav aif aiff aifc caf flac ogg oga", "label");
	mFileName = new HISSTools_TextBlock(this, mVecDraw, 75, 250, 150, 20, "", kHAlignLeft, kVAlignCenter, "small");
	mFileChan = new HISSTools_TextBlock(this, mVecDraw, 75, 270, 50, 20, "", kHAlignLeft, kVAlignCenter, "small");
	
	pGraphics->AttachControl(new HISSTools_Panel(this, mVecDraw, 65, 10, 210, 170));
	pGraphics->AttachControl(new HISSTools_Panel(this, mVecDraw, 65, 190, 210, 280, "tight"));
	pGraphics->AttachControl(new HISSTools_Panel(this, mVecDraw, 70, 195, 200, 94, "tight grey"));
	
	mDryGainDial = new HISSTools_Dial(this, kDryGain, mVecDraw, 75, 50, "red");
	mWetGainDial = new HISSTools_Dial(this, kWetGain, mVecDraw, 175, 50, "green");
	
	pGraphics->AttachControl(mDryGainDial);
	pGraphics->AttachControl(mWetGainDial);
	
	mIMeter = new HISSTools_MeterTest(this, mVecDraw, 15, 10, 40, 460);
	mOMeter = new HISSTools_MeterTest(this, mVecDraw, 285, 10, 40, 460, TRUE);
	
	//mIMeter = new HISSTools_MeterTest(this, mVecDraw, 230, 130, 400, 40);
	//mOMeter = new HISSTools_MeterTest(this, mVecDraw, 230, 190, 400, 40);
	
	mMatrix = new HISSTools_Matrix(this, kMatrixControl, mVecDraw, 90, 315, 8, 8);
	mILEDs = new HISSTools_Matrix(this, -1, mVecDraw, 91.5, 298, 8, 1, "round VU_Leds");
	mOLEDs = new HISSTools_Matrix(this, -1, mVecDraw, 236, 316.5, 1, 8, "round VU_Leds");
	//mOLEDs = new HISSTools_Matrix(this, -1, mVecDraw, 28, 226.5, 1, 8, "round VU_Leds");
	pGraphics->AttachControl(mILEDs);
	pGraphics->AttachControl(mOLEDs);
	
	pGraphics->AttachControl(mFileDialog);
	pGraphics->AttachControl(mFileName);
	pGraphics->AttachControl(mFileChan);
	pGraphics->AttachControl(mIMeter);
	pGraphics->AttachControl(mOMeter);
	
	pGraphics->AttachControl(new HISSTools_Switch(this, kOutputSelect, mVecDraw, 140, 20, 50, 20, 3));
	//pGraphics->AttachControl(new HISSTools_Button(this, -1, 360, 60, 100, 30, -1, TRUE));
	pGraphics->AttachControl(mMatrix);
	
	pGraphics->AttachPanelBackground(&bgrb);
	pGraphics->HandleMouseOver(TRUE);
	
	pGraphics->SetAllowRetina(false);
	
	AttachGraphics(pGraphics);
	
	mXPos = -1;
	mYPos = -1;
	
	mCurrentIChans = 0;
	mCurrentOChans = 0;
	
	mBaseName.Set("");
	
	// Threading
	
	mThreadExiting = FALSE;
	
	mLoadThread = CreateThread(NULL, 0, LoadingThread, this, 0, NULL);
	mLoadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	EnsureDefaultPreset();
	Reset();
}

AHConvN::~AHConvN() 
{	
	// Dispose of thread stuff.
	
	mThreadExiting = true;
	SetEvent(mLoadEvent);
	
	// Spin while we wait for the thread to exit
	
	// FIX - Not sure what to do here....
	
	//while (mThreadExiting == TRUE);
	
	CloseHandle(mLoadEvent);
	CloseHandle(mLoadThread);
	
	// Delete memory that will not otherwise be freed
	
	delete mVecDraw;
	
}

void AHConvN::Reset()
{
	TRACE;
	IMutexLock lock(this);
	
	// FIX - Need to empty buffers here.....
}

void AHConvN::CheckConnections(double** inputs, double** outputs)
{
	int i;
	
	unsigned int currentIChans = mCurrentIChans;
	
	for (i = 0; i < MAX_CHANNELS; i++)
		if (!IsInChannelConnected(i) || (inputs && inputs[i] == NULL))
			break;
	
	mCurrentIChans = i;
	
	for (i = 0; i < MAX_CHANNELS; i++)
		if (!IsOutChannelConnected(i) || (outputs && outputs[i] == NULL))
			break;
	
	mCurrentOChans = i;
	
	// FIX - IPlug for these.
	
	mCurrentOChans = mCurrentIChans;
	
	// FIX - Temp for display niceness...
	//if (currentIChans != mCurrentIChans)
	//	LoadIRs();
}

void AHConvN::UpdateBaseName()
{
	WDL_String currentBaseName("");
	WDL_String baseName("");
    
	FileScheme scheme;
	
	for (FileList::iterator it = mFiles.begin(); it != mFiles.end(); it++)
	{
		if (!(*it)->mFilePath.GetLength())
			continue;
		
		scheme.getBaseName(&currentBaseName, &(*it)->mFilePath);
		
		if (baseName.GetLength() && strcmp(baseName.Get(), currentBaseName.Get()))
		{
			baseName.Set("<Multiple>");
			break;
		}
		
		baseName.Set(&currentBaseName);
	}
	
	mBaseName.Set(&baseName);
}


void AHConvN::UpdateFileDisplay()
{
	WDL_String filePath("");
	WDL_String fileName("");
	
	const char *fileNameCString;
	char chanInfo[32];
	
	bool mute;
	
	int numChans = 0;
	int chan = 0;
	int frames = 0;
	int sampleRate = 0;
	
	FileScheme scheme;
    
	if (mXPos > -1)
	{		
		mFiles.getFile(mXPos, mYPos, &fileNameCString, &chan, &mute);
		mFiles.getInfo(mXPos, mYPos, &frames, &sampleRate, &numChans);
		filePath.Set(fileNameCString);
		scheme.getFileFromPath(&fileName, &filePath);
	}
	else
		fileName.Set(&mBaseName);
	
	if (numChans)
		sprintf(chanInfo, "%d of %d", chan + 1, numChans);
	else
		sprintf(chanInfo, "");
	
	mFileName->setText(fileName.Get());
	mFileChan->setText(chanInfo);
}

void AHConvN::LoadIRs()
{	
	const char *filePath;
	bool mute;
	int chan;
	
	CheckConnections();
	
	for (FileList::iterator it = mFiles.begin(); it != mFiles.end(); it++)
	{
		int inChan = it.getIn();
		int outChan = it.getOut();
		
		bool channelActive = inChan < mCurrentIChans && outChan < mCurrentOChans;
		
		// FIX - Decide on how to display non active channels where something should be loaded
		
		if (it.getFile(&filePath, &chan, &mute, TRUE) == FALSE || channelActive == FALSE)
		{
			if (*filePath == 0 || channelActive == FALSE)
				mMatrix->SetState(inChan, outChan, (channelActive == FALSE) ? 0 : 1);
			continue;
		}
		
		int result = 0;
		
		if (mute == FALSE)
		{
			HISSTools::IAudioFile file(filePath);
			
			if (file.isOpen() && !file.getErrorFlags())
			{
				float *audio = new float[file.getFrames()];
				file.readChannel(audio, file.getFrames(), chan);
				mConvolver.set(inChan, outChan, audio, file.getFrames(), true);
				mFiles.setInfo(inChan, outChan, file.getFrames(), file.getSamplingRate(), file.getChannels());
				delete audio;
				
				result = 1;
			}
		}
		else 
		{
			// FIX - surely this is always true?
			
			result = mute == TRUE ? 2 : 0;
			mConvolver.clear(inChan, outChan, false);
		}
		
		IMutexLock lock(this);
		mMatrix->SetState(inChan, outChan, channelActive ? result + 1 : 0);
	}
	
	mMatrix->SetHilite(FALSE);
	
	UpdateBaseName();
	UpdateFileDisplay();
}

void AHConvN::OnParamChange(int paramIdx, ParamChangeSource source)
{
	IMutexLock lock(this);
	
	WDL_String path, tempStr;
	
	CheckConnections();
	
	switch (paramIdx)
	{
		case kDryGain:
			mTargetDryGain = GetParam(kDryGain)->DBToAmp();
			break;
			
		case kWetGain:
			mTargetWetGain = GetParam(kWetGain)->DBToAmp();
			break;
			
		case kOutputSelect:
			mOutputSelect = GetParam(kOutputSelect)->Int();
			switch (mOutputSelect)
		{	
			case 0:
				mDryGainDial->GrayOut(FALSE);
				mWetGainDial->GrayOut(TRUE);
				break;
				
			case 1:
				mDryGainDial->GrayOut(FALSE);
				mWetGainDial->GrayOut(FALSE);
				break;
				
			case 2:
				mDryGainDial->GrayOut(TRUE);
				mWetGainDial->GrayOut(FALSE);
				break;
		}
			break;
			
		case kFileSelect:
			
			if (mFileDialog->validReport() == TRUE)
			{
				FileScheme scheme;
				
				mFileDialog->GetLastSelectedFileForPlug(&path);
				scheme.loadWithScheme(&path, &mFiles, mCurrentIChans, mCurrentOChans);
				
				SetEvent(mLoadEvent);
			}
			
			break;
			
		case kMatrixControl:
			
			if (mMatrix->validReport() == FALSE)
				return;
			
			mXPos = mMatrix->mXPos;
			mYPos = mMatrix->mYPos;
			
			UpdateFileDisplay();
			
			switch (mMatrix->mMousing)
		{
			case kMouseDown: 
				
				if (mXPos > -1 && IsInChannelConnected(mXPos) && IsInChannelConnected(mYPos))
				{
					if (mMatrix->mPMod.A == TRUE)
						break;
					
					if (mMatrix->mPMod.S == TRUE)
					{
						if (mFiles.incrementChan(mXPos, mYPos))
							SetEvent(mLoadEvent);						
						break;
					}
					
					if (mFiles.flipMute(mXPos, mYPos))
						SetEvent(mLoadEvent);
					break;
					
				case kMouseDblClick:
					
					if (mXPos > -1 && IsInChannelConnected(mXPos) && IsInChannelConnected(mYPos))
					{
						if (mMatrix->mPMod.A == TRUE)
						{
							mFiles.setFile(mXPos, mYPos, "");
							SetEvent(mLoadEvent);
							break;
						}
						
						unsigned char currentState = mMatrix->GetState(mXPos, mYPos);
						mMatrix->SetState(mXPos, mYPos, 1);
						GetGUI()->PromptForFile(&path, kFileOpen, &tempStr,  "wav aif aiff aifc caf flac ogg oga");
						
						if (path.GetLength())
						{
							mFiles.setFile(mXPos, mYPos, path.Get());
							SetEvent(mLoadEvent);
						}
						else 
						{
							mMatrix->SetState(mXPos, mYPos, currentState);
							mMatrix->SetHilite(FALSE);
						}
					}
				}
				break;
				
			case kMouseOver:
				
				if (mXPos > -1 && IsInChannelConnected(mXPos) && IsInChannelConnected(mYPos))
					mMatrix->SetHilite(TRUE);
				
				break;
				
			case kMouseOut:
				mMatrix->SetHilite(FALSE);
				break;
				
			default:
				break;
		}
			
			break;
			
		default:
			break;
	}
}

void AHConvN::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
	double targetDryGain = mOutputSelect != 2 ? mTargetDryGain : 0;
	double targetWetGain = mOutputSelect != 0 ? mTargetWetGain : 0;
	double lastRamp;
	
	CheckConnections(inputs, outputs);
	
	mIBallistics.calcVULevels(inputs, mCurrentIChans, nFrames);
	mIMeter->setLevels(mIBallistics.getPeak(), mIBallistics.getRMS(), mIBallistics.getPeakHold(), mIBallistics.getOver());
	
	for (unsigned int i = 0; i < mCurrentIChans; i++)
		mILEDs->SetState(i, 0, mIBallistics.getledVUState(i));
	
	mConvolver.process((const double **) inputs, outputs, (uint32_t) mCurrentIChans, (uint32_t) mCurrentOChans, (uintptr_t) nFrames);
	
	// Do Ramps
	
	// Wet
	
	lastRamp = mLastWetGain;
	
	for (unsigned int i = 0; i < mCurrentOChans; i++)
		for (unsigned int j = 0; j < nFrames; j++)
			outputs[i][j] *= lastRamp = lastRamp - 0.01 * (lastRamp - targetWetGain);
	
	mLastWetGain = lastRamp;
	
	// Dry
	
	lastRamp = mLastDryGain;

	for (unsigned int i = 0; (i < mCurrentIChans && i < mCurrentOChans); i++)
	{
		for (unsigned int j = 0; j < nFrames; j++)
		{
			lastRamp = lastRamp - 0.01 * (lastRamp - targetDryGain);
			outputs[i][j] += inputs[i][j] * lastRamp;
		}
	}
	
	mLastDryGain = lastRamp;
	
	mOBallistics.calcVULevels(outputs, mCurrentOChans, nFrames);
	mOMeter->setLevels(mOBallistics.getPeak(), mOBallistics.getRMS(), mOBallistics.getPeakHold(), mOBallistics.getOver());
	
	for (unsigned int i = 0; i < mCurrentOChans; i++)
		mOLEDs->SetState(0, i, mOBallistics.getledVUState(i));
}

bool AHConvN::SerializeState(ByteChunk* pChunk)
{
	IMutexLock lock(this);
	
	const char *filePath;
	bool mute;
	int chan;
	
    // Store the number of the preset system (allows backward compatibility for presets
	
	int presetVersion = 1;
	if (pChunk->PutStr("HISSTools_Convolver_Preset") <= 0) 
		return FALSE;
	if (pChunk->Put(&presetVersion) <= 0) 
		return FALSE;
	
	for (FileList::iterator it = mFiles.begin(); it != mFiles.end(); it++)
	{ 
		it.getFile(&filePath, &chan, &mute);
		
		if (pChunk->PutBool(mute) <= 0) 
			return FALSE;
		if (pChunk->Put(&chan) <= 0) 
			return FALSE;
		if (pChunk->PutStr(filePath) <= 0) 
			return FALSE;
	}
	
	return SerializeParams(pChunk);
}

int AHConvN::UnserializeState(ByteChunk* pChunk, int startPos)
{
	IMutexLock lock(this);
	
	FileScheme scheme;
	
	int presetVersion = 0;
	int stringEndPos;
	double v = 0.0;
	bool mute;
	int chan;
	
	WDL_String pStr;
	startPos = pChunk->GetStr(&pStr, startPos);
	
	if (strcmp(pStr.Get(), "HISSTools_Convolver_Preset") == 0)
		startPos = pChunk->Get(&presetVersion, startPos);
	
	CheckConnections();
	
	switch (presetVersion)
	{
		case 0:
			
			// Fixed Structure for Previous Presets
			
			mFileDialog->SetLastSelectedFileFromPlug(pStr.Get());
			startPos = pChunk->Get(&v, startPos);
			GetParam(kDryGain)->Set(v);
			startPos = pChunk->Get(&v, startPos);
			GetParam(kWetGain)->Set(v);
			startPos = pChunk->Get(&v, startPos);
			GetParam(kOutputSelect)->Set(v);
			startPos = pChunk->Get(&v, startPos);
			
			// N.B. File Selector value is meaningless so we don't restore the final value
			
			// FIX - Check loading here....
			
			scheme.loadWithScheme(&pStr, &mFiles, mCurrentIChans, mCurrentOChans);
			LoadIRs();
			
			OnParamReset(kPresetRecall);
			break;
			
		case 1:
			
			for (FileList::iterator it = mFiles.begin(); it != mFiles.end(); it++)
			{ 
				startPos = pChunk->GetBool(&mute, startPos);
				startPos = pChunk->Get(&chan, startPos);
				stringEndPos = pChunk->GetStr(&pStr, startPos);
				
				// Check For Empty String (pStr is not updated correctly in this case)
				
				if (stringEndPos == startPos + 4)   
					it.setFile("");
				else
					it.setFile(pStr.Get(), chan, mute);
				
				startPos = stringEndPos;
			}
			
			LoadIRs();
			startPos = UnserializeParams(pChunk, startPos);
	}
	
	// Stop wet/dry gain from ramping on preset recall
	
	mLastDryGain = mTargetDryGain;
	mLastWetGain = mTargetWetGain;
	
	return startPos;
}
