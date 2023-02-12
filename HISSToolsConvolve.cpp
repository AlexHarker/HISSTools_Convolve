
#include "HISSToolsConvolve.h"
#include "IPlug_include_in_plug_src.h"
#include "IAudioFile.h"

enum ETags
{
    kTagMatrix = 0,
    kTagFileName,
    kTagFileChan,
    kTagIMeter,
    kTagOMeter,
    kTagILEDs,
    kTagOLEDs
};

class HISSTools_CFileSelector : public HISSTools_FileSelector
{
    
public:
    
    HISSTools_CFileSelector(HISSToolsConvolve *plug, double x, double y, double w, double h, EFileAction action, char* dir = "", char* extensions = "", const char *type = 0, HISSTools_Design_Scheme *designScheme = &DefaultDesignScheme)
    :HISSTools_FileSelector(kNoParameter, x, y, w, h, action, dir, extensions, type, designScheme, "Select"), mPlug(plug) {}
    
private:
    
    void reportToPlug() override
    {
        mPlug->SelectFile(GetLastSelectedFileForPlug().Get());
    }
    
    HISSToolsConvolve *mPlug;
};

class HISSTools_FileMatrix : public HISSTools_Matrix
{
    
public:
    
    HISSTools_FileMatrix(HISSToolsConvolve *plug,double x, double y, int xDim, int yDim, const char *type = 0, HISSTools_Design_Scheme *designScheme = &DefaultDesignScheme, HISSTools_Design_Scheme *stateScheme = nullptr)
    : HISSTools_Matrix(kNoParameter, x, y, xDim, yDim, type, designScheme, stateScheme), mPlug(plug)  {}
    
private:
    
    bool IsChannelConnected(int chan)
    {
        return mPlug->IsChannelConnected(kInput, chan);
    }
    
    void reportToPlug() override
    {
        WDL_String path, tempStr;

        mPlug->GUIUpdateFileDisplay();
        
        switch (mMousing)
        {
            case kMouseDown:
                
                if (mXPos > -1 && IsChannelConnected(mXPos) && IsChannelConnected(mYPos))
                {
                    if (mPMod.A)
                        break;
                    
                    if (mPMod.S)
                    {
                        mPlug->IncrementChan(mXPos, mYPos);
                        break;
                    }
                    
                    mPlug->FlipMute(mXPos, mYPos);
                    break;
                    
                case kMouseDblClick:
                    
                    if (mXPos > -1 && IsChannelConnected(mXPos) && IsChannelConnected(mYPos))
                    {
                        if (mPMod.A)
                        {
                            mPlug->SetFile(mXPos, mYPos, "");
                            break;
                        }
                        
                        unsigned char currentState = GetState(mXPos, mYPos);
                        SetState(mXPos, mYPos, 1);
                        GetUI()->PromptForFile(tempStr, path, EFileAction::Open,  "wav aif aiff aifc");
                        
                        if (path.GetLength())
                        {
                            mPlug->SetFile(mXPos, mYPos, path.Get());
                        }
                        else
                        {
                            SetState(mXPos, mYPos, currentState);
                            SetHilite(false);
                        }
                    }
                }
                break;
                
            case kMouseOver:
                
                if (mXPos > -1 && IsChannelConnected(mXPos) && IsChannelConnected(mYPos))
                    SetHilite(true);
                
                break;
                
            case kMouseOut:
                SetHilite(false);
                break;
                
            default:
                break;
        }
    }
    
    HISSToolsConvolve *mPlug;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// LOADING THREAD ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD LoadingThread(LPVOID ConvPlugParam)
{
    HISSToolsConvolve *ConvPlug = (HISSToolsConvolve *) ConvPlugParam;
    
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

void HISSToolsConvolve::SelectFile(const char *file)
{
    WDL_String path(file);
    FileScheme scheme;

    scheme.loadWithScheme(&path, &mFiles, mCurrentIChans, mCurrentOChans);

    SetEvent(mLoadEvent);
}

void HISSToolsConvolve::IncrementChan(int x, int y)
{
    if (mFiles.incrementChan(x, y))
        SetEvent(mLoadEvent);
}

void HISSToolsConvolve::FlipMute(int x, int y)
{
    if (mFiles.flipMute(x, y))
        SetEvent(mLoadEvent);
}

void HISSToolsConvolve::SetFile(int x, int y, const char *path)
{
    mFiles.setFile(mXPos, mYPos, path);
    SetEvent(mLoadEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The number of presets / programs

HISSToolsConvolve::HISSToolsConvolve(const InstanceInfo &info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
, mConvolver(8, 8, kLatencyZero)
, mIVUSender(kTagIMeter)
, mOVUSender(kTagOMeter)
, mILEDSender(kTagILEDs, true)
, mOLEDSender(kTagOLEDs, false)
{
    TRACE;
    
    // Define parameter ranges, display units, labels.
    
    GetParam(kDryGain)->InitDouble("Dry Gain", 0, -60, 20, 0.1, "dB");
    GetParam(kWetGain)->InitDouble("Wet Gain", 0, -60, 20, 0.1, "dB");
    GetParam(kOutputSelect)->InitEnum("Output Select", 2 , 3);
    
//    GetParam(kFileSelect)->InitBool("Load Multiple", false, "", IParam::kFlagCannotAutomate);
//    GetParam(kMatrixControl)->InitBool("Matrix Control", false, "", IParam::kFlagCannotAutomate);
    
    // Allocate Memory
            
    mXPos = -1;
    mYPos = -1;
    
    mCurrentIChans = 0;
    mCurrentOChans = 0;
    
    mBaseName.Set("");
    
    // Threading
    
    mThreadExiting = FALSE;
    
    mLoadThread = CreateThread(NULL, 0, LoadingThread, this, 0, NULL);
    mLoadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    
    MakeDefaultPreset("-", kNumPrograms);
}

HISSToolsConvolve::~HISSToolsConvolve()
{
    // Dispose of thread stuff.
    
    mThreadExiting = true;
    SetEvent(mLoadEvent);
    
    // Spin while we wait for the thread to exit
    
    // FIX - Not sure what to do here....
    
    //while (mThreadExiting == TRUE);
    
    CloseHandle(mLoadEvent);
    CloseHandle(mLoadThread);
}

IGraphics* HISSToolsConvolve::CreateGraphics()
{
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, 60, 1.);
}

void HISSToolsConvolve::LayoutUI(IGraphics* pGraphics)
{
    if (!pGraphics->NControls())
    {
        pGraphics->LoadFont("Arial Bold", "Arial", ETextStyle::Bold);
        IColor bgrb = IColor(255, 185, 195, 205);
        
        pGraphics->AttachControl(new HISSTools_CFileSelector(this, 110, 205, 150, 0, EFileAction::Open, "", "wav aif aiff aifc", "label"));
        pGraphics->AttachControl(new HISSTools_TextBlock(75, 250, 150, 20, "", kHAlignLeft, kVAlignCenter, "small"), kTagFileName);
        pGraphics->AttachControl(new HISSTools_TextBlock(75, 270, 50, 20, "", kHAlignLeft, kVAlignCenter, "small"), kTagFileChan);
        
        pGraphics->AttachControl(new HISSTools_Panel(65, 10, 210, 170));
        pGraphics->AttachControl(new HISSTools_Panel(65, 190, 210, 280, "tight"));
        pGraphics->AttachControl(new HISSTools_Panel(70, 195, 200, 94, "tight grey"));

        pGraphics->AttachControl(new HISSTools_Dial(kDryGain, 75, 50, "red"));
        pGraphics->AttachControl(new HISSTools_Dial(kWetGain, 175, 50, "green"));
        
        pGraphics->AttachControl(new HISSTools_FileMatrix(this, 90, 315, 8, 8), kTagMatrix);
        pGraphics->AttachControl(new HISSTools_Matrix(kNoParameter, 91.5, 298, 8, 1, "round VU_Leds"), kTagILEDs);
        pGraphics->AttachControl(new HISSTools_Matrix(kNoParameter, 236, 316.5, 1, 8, "round VU_Leds"), kTagOLEDs);
        //mOLEDs = new HISSTools_Matrix(*this, -1, &mVecDraw, 28, 226.5, 1, 8, "round VU_Leds");

        pGraphics->AttachControl(new HISSTools_VUMeter(15, 10, 40, 460), kTagIMeter);
        pGraphics->AttachControl(new HISSTools_VUMeter(285, 10, 40, 460, true), kTagOMeter);
        //mIMeter = new HISSTools_MeterTest(230, 130, 400, 40);
        //mOMeter = new HISSTools_MeterTest(230, 190, 400, 40);
        
        pGraphics->AttachControl(new HISSTools_Switch(kOutputSelect, 140, 20, 50, 20, 3));
        
        pGraphics->AttachPanelBackground(bgrb);
        
        // Finalise Graphics
        
        pGraphics->EnableMouseOver(true);
    }
}

void HISSToolsConvolve::OnReset()
{
    TRACE;
    //IMutexLock lock(this);
    
    CheckConnections();
    
    mIVUSender.Reset();
    mOVUSender.Reset();
    mILEDSender.Reset();
    mOLEDSender.Reset();
    
    // FIX - Need to empty buffers here.....
}

void HISSToolsConvolve::CheckConnections()
{
    //unsigned int currentIChans = mCurrentIChans;
    
    mCurrentIChans = NChannelsConnected(kInput);
    mCurrentOChans = NChannelsConnected(kOutput);
    
    // FIX - IPlug for these.
    
    mCurrentOChans = mCurrentIChans;
    
    // FIX - Temp for display niceness...
    //if (currentIChans != mCurrentIChans)
    //    LoadIRs();
}

void HISSToolsConvolve::UpdateBaseName()
{
    WDL_String currentBaseName("");
    WDL_String baseName("");
    
    FileScheme scheme;
    
    for (auto it = mFiles.begin(); it != mFiles.end(); it++)
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


void HISSToolsConvolve::GUIUpdateFileDisplay()
{
    if (!GetUI())
        return;
        
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
    
    GetUI()->GetControlWithTag(kTagFileName)->As<HISSTools_TextBlock>()->setText(fileName.Get());
    GetUI()->GetControlWithTag(kTagFileChan)->As<HISSTools_TextBlock>()->setText(chanInfo);
}

void HISSToolsConvolve::LoadIRs()
{
    const char *filePath;
    bool mute;
    int chan;
    int result = 0;
    
    HISSTools_Matrix *matrix = nullptr;
    
    if (GetUI())
        matrix = GetUI()->GetControlWithTag(kTagMatrix)->As<HISSTools_Matrix>();
    
    CheckConnections();
    
    for (auto it = mFiles.begin(); it != mFiles.end(); it++)
    {
        int inChan = it.getIn();
        int outChan = it.getOut();
        
        bool channelActive = inChan < mCurrentIChans && outChan < mCurrentOChans;
        
        // FIX - Decide on how to display non active channels where something should be loaded
        
        if (!it.getFile(&filePath, &chan, &mute, true) || !channelActive)
        {
            if (*filePath == 0 || !channelActive)
                if (matrix)
                    matrix->SetState(inChan, outChan, channelActive);
            continue;
        }
                
        if (!mute)
        {
            HISSTools::IAudioFile file(filePath);
            
            if (file.isOpen() && !file.getErrorFlags())
            {
                std::vector<float> audio(file.getFrames());

                file.readChannel(audio.data(), file.getFrames(), chan);
                mConvolver.set(inChan, outChan, audio.data(), file.getFrames(), true);
                mFiles.setInfo(inChan, outChan, file.getFrames(), file.getSamplingRate(), file.getChannels());
                
                result = 1;
            }
        }
        else
        {
            result = 2;
            mConvolver.clear(inChan, outChan, false);
        }
        
        //IMutexLock lock(this);
        if (matrix)
            matrix->SetState(inChan, outChan, channelActive ? result + 1 : 0);
    }
    
    if (matrix)
        matrix->SetHilite(false);
    
    UpdateBaseName();
    GUIUpdateFileDisplay();
}

void HISSToolsConvolve::OnParamChange(int paramIdx, EParamSource source, int sampleOffset)
{
    //IMutexLock lock(this);
        
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
            break;
                        
        default:
            break;
    }
}

void HISSToolsConvolve::OnParamChangeUI(int paramIdx, EParamSource source)
{
    switch (paramIdx)
    {
        case kDryGain:                              break;
        case kWetGain:                              break;
        case kOutputSelect:     GUIUpdateDials();   break;
            
        default:
            break;
    }
}

void HISSToolsConvolve::GUIUpdateDials()
{
    int outputSelect = GetParam(kOutputSelect)->Int();
    
    if (!GetUI())
        return;
    
    GetUI()->DisableControl(kDryGain, outputSelect == 2);
    GetUI()->DisableControl(kWetGain, outputSelect == 0);
}

void HISSToolsConvolve::ProcessBlock(double** inputs, double** outputs, int nFrames)
{
    double targetDryGain = mOutputSelect != 2 ? mTargetDryGain : 0;
    double targetWetGain = mOutputSelect != 0 ? mTargetWetGain : 0;
    double lastRamp;
    
    std::array<int, MAX_CHANNELS> LEDStates;
    
    CheckConnections();
    
    mIBallistics.calcVULevels(inputs, mCurrentIChans, nFrames);
    mIVUSender.Set(mIBallistics.getPeak(), mIBallistics.getRMS(), mIBallistics.getPeakHold(), mIBallistics.getOver());
    
    for (unsigned int i = 0; i < mCurrentIChans; i++)
        LEDStates[i] = mIBallistics.getledVUState(i);
    for (unsigned int i = mCurrentIChans; i < MAX_CHANNELS; i++)
        LEDStates[i] = 0;
    
    mILEDSender.Set(LEDStates);
    
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
    mOVUSender.Set(mOBallistics.getPeak(), mOBallistics.getRMS(), mOBallistics.getPeakHold(), mOBallistics.getOver());
    
    for (unsigned int i = 0; i < mCurrentOChans; i++)
        LEDStates[i] = mOBallistics.getledVUState(i);
    for (unsigned int i = mCurrentOChans; i < MAX_CHANNELS; i++)
        LEDStates[i] = 0;
    
    mOLEDSender.Set(LEDStates);
}

bool HISSToolsConvolve::SerializeState(IByteChunk& chunk) const
{
    //IMutexLock lock(this);
    
    const char *filePath;
    bool mute;
    int chan;
    
    // Store the number of the preset system (allows backward compatibility for presets)
    
    int presetVersion = 1;
    if (chunk.PutStr("HISSTools_Convolver_Preset") <= 0)
        return false;
    if (chunk.Put(&presetVersion) <= 0)
        return false;
    
    for (auto it = mFiles.cbegin(); it != mFiles.cend(); it++)
    {
        it.getFile(&filePath, &chan, &mute);
        
        if (chunk.Put(&mute) <= 0)
            return false;
        if (chunk.Put(&chan) <= 0)
            return false;
        if (chunk.PutStr(filePath) <= 0)
            return false;
    }
    
    return SerializeParams(chunk);
}

int HISSToolsConvolve::UnserializeState(const IByteChunk& chunk, int pos)
{
    //IMutexLock lock(this);
    
    FileScheme scheme;
    
    int presetVersion = 0;
    int stringEndPos;
    double v = 0.0;
    bool mute;
    int chan;
    
    WDL_String pStr;
    pos = chunk.GetStr(pStr, pos);
    
    if (strcmp(pStr.Get(), "HISSTools_Convolver_Preset") == 0)
        pos = chunk.Get(&presetVersion, pos);
    
    CheckConnections();
    
    switch (presetVersion)
    {
        case 0:
            
            // Fixed Structure for Previous Presets
            
            // FIX
            //mFileDialog->SetLastSelectedFileFromPlug(pStr.Get());
            pos = chunk.Get(&v, pos);
            GetParam(kDryGain)->Set(v);
            pos = chunk.Get(&v, pos);
            GetParam(kWetGain)->Set(v);
            pos = chunk.Get(&v, pos);
            GetParam(kOutputSelect)->Set(v);
            pos = chunk.Get(&v, pos);
            
            // N.B. File Selector value is meaningless so we don't restore the final value
            
            // FIX - Check loading here....
            
            scheme.loadWithScheme(&pStr, &mFiles, mCurrentIChans, mCurrentOChans);
            LoadIRs();
            
            OnParamReset(kPresetRecall);
            break;
            
        case 1:
            
            for (auto it = mFiles.begin(); it != mFiles.end(); it++)
            {
                pos = chunk.Get(&mute, pos);
                pos = chunk.Get(&chan, pos);
                stringEndPos = chunk.GetStr(pStr, pos);
                
                // Check For Empty String (pStr is not updated correctly in this case)
                
                if (stringEndPos == pos + 4)
                    it.setFile("");
                else
                    it.setFile(pStr.Get(), chan, mute);
                
                pos = stringEndPos;
            }
            
            LoadIRs();
            pos = UnserializeParams(chunk, pos);
    }
    
    // Stop wet/dry gain from ramping on preset recall
    
    mLastDryGain = mTargetDryGain;
    mLastWetGain = mTargetWetGain;
    
    return pos;
}

void HISSToolsConvolve::OnIdle()
{
    mIVUSender.UpdateControl(*this);
    mOVUSender.UpdateControl(*this);
    mILEDSender.UpdateControl(GetUI());
    mOLEDSender.UpdateControl(GetUI());
}
