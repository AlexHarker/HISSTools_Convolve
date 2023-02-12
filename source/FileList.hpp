
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
		
		mutable HISSTools_SpinLock mLock;
        mutable bool mDirty;
	};

public:
	
    template <typename FL>
	class iterator_base
	{	
		
	public:
		
        iterator_base(FL *fileList, int startPos)
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
		
		bool operator ==(const iterator_base &rhs) const
		{	
			return (rhs.mFileList == mFileList && rhs.mPos == mPos);
		}
		
		bool operator !=(const iterator_base &rhs) const
		{	
			return !(*this == rhs);
		}
		
        iterator_base &operator ++()
		{	
			mPos++; 
			return *this; 
		}
		
        iterator_base &operator --()
		{
			mPos--; 
			return *this; 
		}
		
        iterator_base operator ++(int)
		{	
            iterator_base iResult(mFileList, mPos++);
			return iResult; 
		}
		
        iterator_base operator --(int)
		{	
            iterator_base iResult(mFileList, mPos--);
			return iResult; 
		}
		
		FileSlot *operator *()
		{
			if (mFileList->checkRange(getIn(), getOut()))
				return mFileList->mFileSlots + mPos;
			
			return nullptr;
		}
		
		void setFile(const char *filePath, int chan = 0, bool mute = false)
		{
			mFileList->setFile(mFileList->convertIndexToInput(mPos), mFileList->convertIndexToOutput(mPos), filePath, chan, mute);
		}
		
		bool getFile(const char **filePath, int *chan, bool *mute, bool clean = false)
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
		
        FL *mFileList;
		int mPos;
	};	
	
    using iterator = iterator_base<FileList>;
    using const_iterator = iterator_base<const FileList>;
    
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
			mFileSlots[i].mMute = false;
			mFileSlots[i].mChan = 0;
			
			// File Information
			
			mFileSlots[i].mSampleRate = 0;
			mFileSlots[i].mNumChans = 0;
			mFileSlots[i].mFrames = 0;
			
			// Locking and Dirty Info
			
			mFileSlots[i].mDirty = true;
		}
	}
	
	FileSlot *get(int input, int output)
	{
		if (checkRange(input, output))
			return mFileSlots + convertIOToIndex(input, output);
		
		return nullptr;
	}
    
    const FileSlot *get(int input, int output) const
    {
        if (checkRange(input, output))
            return mFileSlots + convertIOToIndex(input, output);
        
        return nullptr;
    }
	
	void setFile(int input, int output, const char *filePath, int chan = 0, bool mute = false)
	{
		setFile(get(input, output), filePath, chan, mute);
	}
	
	bool getFile(int input, int output, const char **filePath, int *chan, bool *mute, bool clean = false) const
	{
		return getFile(get(input, output), filePath, chan, mute, clean);
	}
	
	void setInfo(int input, int output, int frames, int sampleRate, int numChans)
	{
		setInfo(get(input, output), frames, sampleRate, numChans);
	}
	
	void getInfo(int input, int output, int *frames, int *sampleRate, int *numChans) const
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
    
    const_iterator cbegin() const
    {
        return const_iterator(this, 0);
    }
    
    const_iterator cend() const
    {
        return const_iterator(this, convertIOToIndex(mNumIns - 1, mNumOuts - 1) + 1);
    }
	
private:
	
	static void setFile(FileSlot *slot, const char *filePath, int chan, bool mute)
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
			slot->mDirty = true;
			slot->mLock.release();
		}
	}
	
	static bool getFile(const FileSlot *slot, const char **filePath, int *chan, bool *mute, bool clean)
	{
		bool slotDirty = false;
		
		if (slot)
		{
			slot->mLock.acquire();
			*filePath = slot->mFilePath.Get();	
			*chan = slot->mChan;
			*mute = slot->mMute;
			slotDirty = slot->mDirty;
			slot->mDirty = clean ? false : slot->mDirty;
			slot->mLock.release();
		}
		
		return slotDirty;
	}
	
	static bool flipMute(FileSlot *slot)
	{		
		if (slot && slot->mFilePath.GetLength())
		{
			slot->mLock.acquire();
			
			if (slot->mFilePath.GetLength())
			{
				slot->mMute = !slot->mMute;
				slot->mDirty = true;
				slot->mLock.release();
				
				return true;
			}
			
			slot->mLock.release();
		}
		
		return false;
	}
	
	static bool incrementChan(FileSlot *slot)
	{		
		if (slot)
		{
			slot->mLock.acquire();
			
			if (slot->mNumChans > 1)
			{
				slot->mChan = (slot->mChan + 1) % slot->mNumChans;
				slot->mDirty = true;
				slot->mLock.release();
				
				return true;
			}
			
			slot->mLock.release();
		}
		
		return false;
	}
	
    static void setInfo(FileSlot *slot, int frames, int sampleRate, int numChans)
	{
		if (slot)
		{
			slot->mFrames = frames;	
			slot->mSampleRate = sampleRate;
			slot->mNumChans = numChans;
		}
	}
	
	static void getInfo(const FileSlot *slot, int *frames, int *sampleRate, int *numChans)
	{
		if (slot)
		{
			*frames = slot->mFrames;	
			*sampleRate = slot->mSampleRate;
			*numChans = slot->mNumChans;
		}
	}
	
	bool checkRange(int input, int output) const
	{
        return (input >= 0 && input < mNumIns && output >= 0 && output < mNumOuts);
	}
	
	int convertIOToIndex(int input, int output) const
	{	
		return mNumIns * input + output; 
	}
	
	int convertIndexToInput(int idx) const
	{	
		return idx / mNumIns; 
	}
	
	int convertIndexToOutput(int idx) const
	{	
		return idx % mNumIns; 
	}
	
	int mNumIns;
	int mNumOuts;
	
	FileSlot *mFileSlots;
};
