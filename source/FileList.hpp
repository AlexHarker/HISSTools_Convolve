
#pragma once

#include <wdlstring.h>
#include "HISSTools_ThreadSafety.hpp"

class FileList
{	
	struct FileSlot
	{
		// Parameters
		
		WDL_String mFilePath;
		
		bool mMute;
		
		int mChan;
		
		// File Information
		
		int mFrames;
		int mSampleRate;
		int mNumChans;
		
		// Lock
		
		HISSTools_SpinLock mLock;
		bool mDirty;
	};

public:
	
	class iterator
	{	
		
	public:
		
		iterator(FileList *fileList, int startPos)
		{
			mPos = startPos;
			mFileList = fileList;
		}
		
		int getIn()		
		{	
			return mFileList->convertIndexToInput(mPos); 
		}
		
		int getOut()	
		{	
			return mFileList->convertIndexToOutput(mPos); 
		}
		
		bool operator ==(const iterator &rhs)	
		{	
			return (rhs.mFileList == mFileList && rhs.mPos == mPos) ? TRUE : FALSE; 
		}
		
		bool operator !=(const iterator &rhs)	
		{	
			return (*this == rhs) == TRUE ? FALSE : TRUE; 
		}
		
		iterator &operator ++() 
		{	
			mPos++; 
			return *this; 
		}
		
		iterator &operator --()
		{
			mPos--; 
			return *this; 
		}
		
		iterator operator ++(int)
		{	
			iterator iResult(mFileList, mPos++); 
			return iResult; 
		}
		
		iterator operator --(int)				
		{	
			iterator iResult(mFileList, mPos--);
			return iResult; 
		}
		
		FileSlot *operator *()
		{
			if (mFileList->checkRange(getIn(), getOut()))
				return mFileList->mFileSlots + mPos;
			
			return 0;
		}
		
		void setFile(const char *filePath, int chan = 0, bool mute = FALSE)
		{
			mFileList->setFile(mFileList->convertIndexToInput(mPos), mFileList->convertIndexToOutput(mPos), filePath, chan, mute);
		}
		
		bool getFile(const char **filePath, int *chan, bool *mute, bool clean = FALSE)
		{
			return mFileList->getFile(mFileList->convertIndexToInput(mPos), mFileList->convertIndexToOutput(mPos), filePath, chan, mute, clean);
		}
		
		void setInfo(int frames, int sampleRate, int numChans)
		{
			mFileList->setInfo(mFileList->convertIndexToInput(mPos), mFileList->convertIndexToOutput(mPos), frames, sampleRate, numChans);
		}
		
		void getInfo(int *frames, int *sampleRate, int *numChans)
		{
			mFileList->getInfo(mFileList->convertIndexToInput(mPos), mFileList->convertIndexToOutput(mPos), frames, sampleRate, numChans);
		}
		
	private:
		
		FileList *mFileList;
		int mPos;
	};	
	
	FileList(int numIns = 8, int numOuts = 8)
	{
		numIns = numIns < 1 ? 1 : numIns;
		mNumOuts = numOuts < 1 ? 1 : numOuts;
		
		mNumIns = numIns;
		mNumOuts = numOuts;
		
		mFileSlots = new FileSlot[numIns * numOuts];
		
		clear();
	}
	
	void clear()
	{
		for (int i = 0; i < mNumIns * mNumOuts; i++)
		{
			mFileSlots[i].mFilePath.Set("");
			mFileSlots[i].mMute = FALSE;
			mFileSlots[i].mChan = 0;
			
			// File Information
			
			mFileSlots[i].mSampleRate = 0;
			mFileSlots[i].mNumChans = 0;
			mFileSlots[i].mFrames = 0;
			
			// Locking and Dirty Info
			
			mFileSlots[i].mDirty = TRUE;
		}
	}
	
	FileSlot *get(int input, int output)
	{
		if (checkRange(input, output) == TRUE)
			return mFileSlots + convertIOToIndex(input, output);
		
		return 0;
	}
	
	void setFile(int input, int output, const char *filePath, int chan = 0, bool mute = FALSE)
	{
		setFile(get(input, output), filePath, chan, mute);
	}
	
	bool getFile(int input, int output, const char **filePath, int *chan, bool *mute, bool clean = FALSE)
	{
		return getFile(get(input, output), filePath, chan, mute, clean);
	}
	
	void setInfo(int input, int output, int frames, int sampleRate, int numChans)
	{
		setInfo(get(input, output), frames, sampleRate, numChans);
	}
	
	void getInfo(int input, int output, int *frames, int *sampleRate, int *numChans)
	{
		getInfo(get(input, output), frames, sampleRate, numChans);
	}
	
	bool incrementChan(int input, int output)
	{
		return incrementChan(get(input, output));
	}
	
	bool flipMute(int input, int output)
	{
		return flipMute(get(input, output));
	}
	
	int getNumIns()		
	{	
		return mNumIns; 
	}
	
	int getNumOuts()
	{	
		return mNumOuts; 
	}
	
	iterator begin()	
	{
		return iterator(this, 0);
	}
	
	iterator end()
	{	
		return iterator(this, convertIOToIndex(mNumIns - 1, mNumOuts - 1) + 1); 
	}
	
private:
	
	void setFile(FileSlot *slot, const char *filePath, int chan, bool mute)
	{
		if (slot)
		{
			slot->mLock.acquire();
			slot->mFilePath.Set(filePath);	
			slot->mChan = chan;
			slot->mMute = mute;
			slot->mFrames = 0;	
			slot->mSampleRate = 0;
			slot->mNumChans = 0;
			slot->mDirty = TRUE;
			slot->mLock.release();
		}
	}
	
	bool getFile(FileSlot *slot, const char **filePath, int *chan, bool *mute, bool clean)
	{
		bool slotDirty = FALSE;
		
		if (slot)
		{
			slot->mLock.acquire();
			*filePath = slot->mFilePath.Get();	
			*chan = slot->mChan;
			*mute = slot->mMute;
			slotDirty = slot->mDirty;
			slot->mDirty = (clean == TRUE) ? FALSE : slot->mDirty;
			slot->mLock.release();
		}
		
		return slotDirty;
	}
	
	bool flipMute(FileSlot *slot)
	{		
		if (slot && slot->mFilePath.GetLength())
		{
			slot->mLock.acquire();
			
			if (slot->mFilePath.GetLength())
			{
				slot->mMute = (slot->mMute == TRUE) ? FALSE : TRUE;
				slot->mDirty = TRUE;
				slot->mLock.release();
				
				return TRUE;
				
			}
			
			slot->mLock.release();
		}
		
		return FALSE;
	}
	
	bool incrementChan(FileSlot *slot)
	{		
		if (slot)
		{
			slot->mLock.acquire();
			
			if (slot->mNumChans > 1)
			{
				slot->mChan = (slot->mChan + 1) % slot->mNumChans;
				slot->mDirty = TRUE;
				slot->mLock.release();
				
				return TRUE;
			}
			
			slot->mLock.release();
		}
		
		return FALSE;
	}
	
	void setInfo(FileSlot *slot, int frames, int sampleRate, int numChans)
	{
		if (slot)
		{
			slot->mFrames = frames;	
			slot->mSampleRate = sampleRate;
			slot->mNumChans = numChans;
		}
	}
	
	void getInfo(FileSlot *slot, int *frames, int *sampleRate, int *numChans)
	{
		if (slot)
		{
			*frames = slot->mFrames;	
			*sampleRate = slot->mSampleRate;
			*numChans = slot->mNumChans;
		}
	}
	
	bool checkRange(int input, int output)
	{
		if (input >= 0 && input < mNumIns && output >= 0 && output < mNumOuts)
			return TRUE;
		
		return FALSE;
	}
	
	int convertIOToIndex(int input, int output) 
	{	
		return mNumIns * input + output; 
	}
	
	int convertIndexToInput(int idx)			
	{	
		return idx / mNumIns; 
	}
	
	int convertIndexToOutput(int idx)			
	{	
		return idx % mNumIns; 
	}
	
	int mNumIns;
	int mNumOuts;
	
	FileSlot *mFileSlots;
};
