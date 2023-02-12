
#pragma once

#include <memory>
#include <type_traits>
#include <vector>
#include <wdlstring.h>
#include "HISSTools_ThreadSafety.hpp"

class FileList
{
public:

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
        
        void setFile(const char *filePath, int chan = 0, bool mute = false)
        {
            mLock.acquire();
            mFilePath.Set(filePath);
            mChan = chan;
            mMute = mute;
            mFrames = 0;
            mSampleRate = 0;
            mNumChans = 0;
            mDirty = true;
            mLock.release();
        }
        
        bool getFile(WDL_String& path, int *chan, bool *mute, bool clean = false) const
        {
            mLock.acquire();
            path.Set(mFilePath.Get());
            *chan = mChan;
            *mute = mMute;
            bool slotDirty = mDirty;
            mDirty = clean ? false : mDirty;
            mLock.release();
            
            return slotDirty;
        }
        
        bool flipMute()
        {
            if (mFilePath.GetLength())
            {
                mLock.acquire();
                
                if (mFilePath.GetLength())
                {
                    mMute = !mMute;
                    mDirty = true;
                    mLock.release();
                    
                    return true;
                }
                
                mLock.release();
            }
            
            return false;
        }
        
        bool incrementChan()
        {
            mLock.acquire();
                
            if (mNumChans > 1)
            {
                mChan = (mChan + 1) % mNumChans;
                mDirty = true;
                mLock.release();
                    
                return true;
            }
                
            mLock.release();
            
            return false;
        }
        
        void setInfo(int frames, int sampleRate, int numChans)
        {
            mFrames = frames;
            mSampleRate = sampleRate;
            mNumChans = numChans;
        }
        
        void getInfo(int *frames, int *sampleRate, int *numChans) const
        {
            *frames = mFrames;
            *sampleRate = mSampleRate;
            *numChans = mNumChans;
        }
	};
	
    template <typename FL, typename IT>
	class iterator_base : public IT
	{
        friend FL;
        
        iterator_base(FL& fileList, IT it)
        : IT (it)
        , mFileList(fileList)
        {}
        
	public:
		
		int getIn() const
		{	
			return static_cast<int>(*this - mFileList.begin()) / mFileList.mNumIns;
        }
		
		int getOut() const
		{	
			return static_cast<int>(*this - mFileList.begin()) % mFileList.mNumIns;
		}
    
	private:
		
        FL& mFileList;
	};
	
    using vector_type = std::vector<FileSlot>;
    using iterator = iterator_base<FileList, vector_type::iterator>;
    using const_iterator = iterator_base<const FileList, vector_type::const_iterator>;
    
	FileList(int numIns = 8, int numOuts = 8)
    : mNumIns(std::max(1, numIns))
    , mNumOuts(std::max(1, numOuts))
    , mFileSlots(mNumIns * mNumOuts)
	{
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
	
	FileSlot& get(int input, int output)
	{
		return mFileSlots[convertIOToIndex(input, output)];
	}
    
    const FileSlot& get(int input, int output) const
    {
        return mFileSlots[convertIOToIndex(input, output)];
    }
	
	void setFile(int input, int output, const char *filePath, int chan = 0, bool mute = false)
	{
        get(input, output).setFile(filePath, chan, mute);
	}
	
	bool getFile(int input, int output, WDL_String &path, int *chan, bool *mute, bool clean = false) const
	{
		return get(input, output).getFile(path, chan, mute, clean);
	}
	
	void setInfo(int input, int output, int frames, int sampleRate, int numChans)
	{
        get(input, output).setInfo(frames, sampleRate, numChans);
	}
	
	void getInfo(int input, int output, int *frames, int *sampleRate, int *numChans) const
	{
        get(input, output).getInfo(frames, sampleRate, numChans);
	}
	
	bool incrementChan(int input, int output)
	{
		return get(input, output).incrementChan();
	}
	
	bool flipMute(int input, int output)
	{
		return get(input, output).flipMute();
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
		return iterator(*this, mFileSlots.begin());
	}
	
	iterator end()
	{	
		return iterator(*this, mFileSlots.end());
	}
    
    const_iterator cbegin() const
    {
        return const_iterator(*this, mFileSlots.cbegin());
    }
    
    const_iterator cend() const
    {
        return const_iterator(*this, mFileSlots.cend());
    }
	
private:
	
	int convertIOToIndex(int input, int output) const
	{	
		return mNumIns * input + output; 
	}
	
	int mNumIns;
	int mNumOuts;
	
	std::vector<FileSlot> mFileSlots;
};
