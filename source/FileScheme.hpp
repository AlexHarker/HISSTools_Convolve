
#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "FileList.hpp"
#include <AudioFile/IAudioFile.h>

class FileScheme
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////// Scheme Structures ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	struct Token
	{	
		std::vector <WDL_String> mStrings;
		
		Token(const char *str)
		{
			mStrings.push_back(WDL_String(str));
		}
	};
	
	struct Slot
	{	
		int mIChan;
		int mOChan;
		
		int mChan;
		
		Token *mSuffix1; 
		Token *mSuffix2;
		Token *mSeparator;
		
		bool mOptional;
		
		Slot (int iChan, int oChan, int chan, Token *suffix1, Token *suffix2, Token *separator, bool optional) : 
		mIChan(iChan), mOChan(oChan), mChan(chan), mSuffix1(suffix1), mSuffix2(suffix2), mSeparator(separator), mOptional(optional) {}
	};
	
	struct Scheme
	{	
		WDL_String mID;
		
		int mNumIChan;
		int mNumOChan;
		
		bool mMinIChan;
		bool mMinOChan;
		
		std::vector <Slot> mSlots;
		
		Scheme() : mID(""), mNumIChan(-1), mNumOChan(-1), mMinIChan(FALSE), mMinOChan(FALSE) {}
	};
	
	struct List
	{
		WDL_String mName;
		WDL_String mID;
		
		std::vector <Scheme *> mSchemes;
		
		List () : mName(""), mID("") {}
	};
	
	struct Folder
	{
		WDL_String mName;
		
		std::vector <List *> mLists;
		
		Folder () : mName("") {}
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Convenient Vector Iterators ///////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	typedef std::vector <Token *>::iterator		TokenPtrIterator;
	typedef std::vector <WDL_String>::iterator	StringIterator;
	typedef std::vector <Folder *>::iterator	FolderPtrIterator;
	typedef std::vector <List *>::iterator		ListPtrIterator;
	typedef std::vector <Scheme *>::iterator	SchemePtrIterator;
	typedef std::vector <Slot>::iterator		SlotIterator;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////// Enums ///////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	enum Keyword {
		kKeywordNone, kKeywordEmpty,
		kKeywordTop, kKeywordFolder, kKeywordList, kKeywordScheme, kKeywordSlot, kKeywordChannels, 
		kKeywordName, kKeywordID, kKeywordIncludeList, kKeywordIncludeScheme, kKeywordSynonym
	};

    enum ParseErrorCode {
		kErrNone,
		kErrFolderHasName, kErrListHasName, kErrUnexpectedName, kErrNameNoValue,
		kErrListHasID, kErrSchemeHasID, kErrUnexpectedID, kErrListIDExists, kErrSchemeIDExists, kErrIDNoValue,
		kErrListIncludeIDNotGiven, kErrListIncludeNoOpenFolder, kErrListIncludeIDNotFound,
		kErrSchemeIncludeIDNotGiven, kErrSchemeIncludeNoOpenList, kErrSchemeIncludeIDNotFound,
		kErrClosedFolderEmpty, kErrClosedListEmpty, kErrClosedListNoID, kErrClosedSchemeEmpty, kErrClosedSchemeNoID,
		kErrSlotNoOpenScheme, kErrSlotNotGiven, kErrSlotInvalidSuffix, kErrSlotAlreadyDefined,
		kErrChannelsNoOpenScheme, kErrChannelsNotGiven, kErrChannelsAlreadyDefined,
		kErrUnexpectedNonKeyword,
	};
		
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// Variables /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Suffix Token Lists
	
	std::vector <Token *> mSuffixTokens;
	std::vector <Token *> mSourceTokens;
	std::vector <Token *> mReceiverTokens;
	std::vector <Token *> mSeparatorTokens;
	
	// Object Lists (for freeing)
	
	std::vector <Folder *> mFolders;
	std::vector <List *> mLists;
	std::vector <Scheme *> mSchemes;

	// Scheme Hierarchy
	
	Folder		*mOpenFolder;
	List		*mOpenList;
	Scheme		*mOpenScheme;
	
	// Parsing Variables
	
	std::ifstream *mFileStream;

	const char *mCurrentReadPoint;

	WDL_String mCurrentLineString;
	WDL_String mCurrentString;
	
	int mLine;
	
	int mOpenSlotInput;
	int mOpenSlotOutput;

	bool mProgress;
	
	// TEMP
	
	List *mList;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Constructor and Destructor ////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
	
	FileScheme()
	{
		mOpenFolder = 0;
		mOpenList = 0;
		mOpenScheme = 0;

		mLine = 0;
		mFileStream = 0;
		mCurrentReadPoint = 0;
		mCurrentString.Set("");
		
		if (parse("/Volumes/Macintosh HD/Users/alexharker/Scheme.txt") == FALSE)
		{
			// Basic default loading scheme
			
			openFolder();
			openList();
			
			openScheme();
			setChannels(0, 0, FALSE, FALSE);
			
			// FIX - take numbers for here from somewhere....
			
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					char iN[8];
					char oN[8];
					
					sprintf(iN, "%d", i + 1);
					sprintf(oN, "%d", j + 1);
					addSlot(i, j, 0, iN, oN, "-", FALSE);
				}
			}
			
			closeFolder();
		}
		
		mList = mFolders.back()->mLists.back();
/*
		openFolder();
		openList();

		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Centre", "binauralL", "-", FALSE);
		addSlot(0, 1, 0, "Centre", "binauralR", "-", FALSE);
		addSlot(1, 0, 0, "Centre", "binauralL", "-", FALSE);
		addSlot(1, 1, 0, "Centre", "binauralR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Centre", "ortfL", "-", FALSE);
		addSlot(0, 1, 0, "Centre", "ortfR", "-", FALSE);
		addSlot(1, 0, 0, "Centre", "ortfL", "-", FALSE);
		addSlot(1, 1, 0, "Centre", "ortfR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Centre", "L", "-", FALSE);
		addSlot(0, 1, 0, "Centre", "R", "-", FALSE);
		addSlot(1, 0, 0, "Centre", "L", "-", FALSE);
		addSlot(1, 1, 0, "Centre", "R", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Centre", "Ls", "-", FALSE);
		addSlot(0, 1, 0, "Centre", "Rs", "-", FALSE);
		addSlot(1, 0, 0, "Centre", "Ls", "-", FALSE);
		addSlot(1, 1, 0, "Centre", "Rs", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Centre", "omniL", "-", FALSE);
		addSlot(0, 1, 0, "Centre", "omniR", "-", FALSE);
		addSlot(1, 0, 0, "Centre", "omniL", "-", FALSE);
		addSlot(1, 1, 0, "Centre", "omniR", "-", FALSE);
		
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FCentre", "binauralL", "-", FALSE);
		addSlot(0, 1, 0, "FCentre", "binauralR", "-", FALSE);
		addSlot(1, 0, 0, "FCentre", "binauralL", "-", FALSE);
		addSlot(1, 1, 0, "FCentre", "binauralR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FCentre", "ortfL", "-", FALSE);
		addSlot(0, 1, 0, "FCentre", "ortfR", "-", FALSE);
		addSlot(1, 0, 0, "FCentre", "ortfL", "-", FALSE);
		addSlot(1, 1, 0, "FCentre", "ortfR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FCentre", "L", "-", FALSE);
		addSlot(0, 1, 0, "FCentre", "R", "-", FALSE);
		addSlot(1, 0, 0, "FCentre", "L", "-", FALSE);
		addSlot(1, 1, 0, "FCentre", "R", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FCentre", "Ls", "-", FALSE);
		addSlot(0, 1, 0, "FCentre", "Rs", "-", FALSE);
		addSlot(1, 0, 0, "FCentre", "Ls", "-", FALSE);
		addSlot(1, 1, 0, "FCentre", "Rs", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FCentre", "omniL", "-", FALSE);
		addSlot(0, 1, 0, "FCentre", "omniR", "-", FALSE);
		addSlot(1, 0, 0, "FCentre", "omniL", "-", FALSE);
		addSlot(1, 1, 0, "FCentre", "omniR", "-", FALSE);
		
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Left", "binauralL", "-", FALSE);
		addSlot(0, 1, 0, "Left", "binauralR", "-", FALSE);
		addSlot(1, 0, 0, "Right", "binauralL", "-", FALSE);
		addSlot(1, 1, 0, "Right", "binauralR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Left", "ortfL", "-", FALSE);
		addSlot(0, 1, 0, "Left", "ortfR", "-", FALSE);
		addSlot(1, 0, 0, "Right", "ortfL", "-", FALSE);
		addSlot(1, 1, 0, "Right", "ortfR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Left", "L", "-", FALSE);
		addSlot(0, 1, 0, "Left", "R", "-", FALSE);
		addSlot(1, 0, 0, "Right", "L", "-", FALSE);
		addSlot(1, 1, 0, "Right", "R", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Left", "Ls", "-", FALSE);
		addSlot(0, 1, 0, "Left", "Rs", "-", FALSE);
		addSlot(1, 0, 0, "Right", "Ls", "-", FALSE);
		addSlot(1, 1, 0, "Right", "Rs", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "Left", "omniL", "-", FALSE);
		addSlot(0, 1, 0, "Left", "omniR", "-", FALSE);
		addSlot(1, 0, 0, "Right", "omniL", "-", FALSE);
		addSlot(1, 1, 0, "Right", "omniR", "-", FALSE);
		
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FLeft", "binauralL", "-", FALSE);
		addSlot(0, 1, 0, "FLeft", "binauralR", "-", FALSE);
		addSlot(1, 0, 0, "FRight", "binauralL", "-", FALSE);
		addSlot(1, 1, 0, "FRight", "binauralR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FLeft", "ortfL", "-", FALSE);
		addSlot(0, 1, 0, "FLeft", "ortfR", "-", FALSE);
		addSlot(1, 0, 0, "FRight", "ortfL", "-", FALSE);
		addSlot(1, 1, 0, "FRight", "ortfR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FLeft", "L", "-", FALSE);
		addSlot(0, 1, 0, "FLeft", "R", "-", FALSE);
		addSlot(1, 0, 0, "FRight", "L", "-", FALSE);
		addSlot(1, 1, 0, "FRight", "R", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FLeft", "Ls", "-", FALSE);
		addSlot(0, 1, 0, "FLeft", "Rs", "-", FALSE);
		addSlot(1, 0, 0, "FRight", "Ls", "-", FALSE);
		addSlot(1, 1, 0, "FRight", "Rs", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "FLeft", "omniL", "-", FALSE);
		addSlot(0, 1, 0, "FLeft", "omniR", "-", FALSE);
		addSlot(1, 0, 0, "FRight", "omniL", "-", FALSE);
		addSlot(1, 1, 0, "FRight", "omniR", "-", FALSE);

		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "NLeft", "binauralL", "-", FALSE);
		addSlot(0, 1, 0, "NLeft", "binauralR", "-", FALSE);
		addSlot(1, 0, 0, "NRight", "binauralL", "-", FALSE);
		addSlot(1, 1, 0, "NRight", "binauralR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "NLeft", "ortfL", "-", FALSE);
		addSlot(0, 1, 0, "NLeft", "ortfR", "-", FALSE);
		addSlot(1, 0, 0, "NRight", "ortfL", "-", FALSE);
		addSlot(1, 1, 0, "NRight", "ortfR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "NLeft", "L", "-", FALSE);
		addSlot(0, 1, 0, "NLeft", "R", "-", FALSE);
		addSlot(1, 0, 0, "NRight", "L", "-", FALSE);
		addSlot(1, 1, 0, "NRight", "R", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "NLeft", "Ls", "-", FALSE);
		addSlot(0, 1, 0, "NLeft", "Rs", "-", FALSE);
		addSlot(1, 0, 0, "NRight", "Ls", "-", FALSE);
		addSlot(1, 1, 0, "NRight", "Rs", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "NLeft", "omniL", "-", FALSE);
		addSlot(0, 1, 0, "NLeft", "omniR", "-", FALSE);
		addSlot(1, 0, 0, "NRight", "omniL", "-", FALSE);
		addSlot(1, 1, 0, "NRight", "omniR", "-", FALSE);

		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "RLeft", "binauralL", "-", FALSE);
		addSlot(0, 1, 0, "RLeft", "binauralR", "-", FALSE);
		addSlot(1, 0, 0, "RRight", "binauralL", "-", FALSE);
		addSlot(1, 1, 0, "RRight", "binauralR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "RLeft", "ortfL", "-", FALSE);
		addSlot(0, 1, 0, "RLeft", "ortfR", "-", FALSE);
		addSlot(1, 0, 0, "RRight", "ortfL", "-", FALSE);
		addSlot(1, 1, 0, "RRight", "ortfR", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "RLeft", "L", "-", FALSE);
		addSlot(0, 1, 0, "RLeft", "R", "-", FALSE);
		addSlot(1, 0, 0, "RRight", "L", "-", FALSE);
		addSlot(1, 1, 0, "RRight", "R", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "RLeft", "Ls", "-", FALSE);
		addSlot(0, 1, 0, "RLeft", "Rs", "-", FALSE);
		addSlot(1, 0, 0, "RRight", "Ls", "-", FALSE);
		addSlot(1, 1, 0, "RRight", "Rs", "-", FALSE);
		openScheme();
		setChannels(2, 2, FALSE, FALSE);
		addSlot(0, 0, 0, "RLeft", "omniL", "-", FALSE);
		addSlot(0, 1, 0, "RLeft", "omniR", "-", FALSE);
		addSlot(1, 0, 0, "RRight", "omniL", "-", FALSE);
		addSlot(1, 1, 0, "RRight", "omniR", "-", FALSE);

		closeFolder();
		
		mList = mFolders.back()->mLists.back();
 */

	}

	~FileScheme()
	{
		for (TokenPtrIterator it = mSuffixTokens.begin(); it != mSuffixTokens.end(); it++)
			delete *it;
		for (TokenPtrIterator it = mSourceTokens.begin(); it != mSourceTokens.end(); it++)
			delete *it;
		for (TokenPtrIterator it = mReceiverTokens.begin(); it != mReceiverTokens.end(); it++)
			delete *it;
		for (TokenPtrIterator it = mSeparatorTokens.begin(); it != mSeparatorTokens.end(); it++)
			delete *it;
		for (FolderPtrIterator it = mFolders.begin(); it != mFolders.end(); it++)
			delete *it;
		for (ListPtrIterator it = mLists.begin(); it != mLists.end(); it++)
			delete *it;
		for (SchemePtrIterator it = mSchemes.begin(); it != mSchemes.end(); it++)
			delete *it;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////// Suffix Tokens ////////////////////////////////////////////// 
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:	

	bool matchToken(Token *token, const char *str)
	{		
		for (StringIterator it = token->mStrings.begin(); it != token->mStrings.end(); it++)
			if (strcmp(str, it->Get()) == 0)
				return TRUE;
		
		return FALSE;
	}
	
	// Figure out synonyms later..... (there is a chance that two tokens now combine through a third token....
	
	void newSynonymTokenFromString(std::vector <Token *> *tokens, const char *str, const char *synonym)
	{
	}
	
	
	Token *newTokenFromString(std::vector <Token *> *tokens, const char *str)
	{		
		Token *theToken = new Token(str);
		
		tokens->push_back(theToken);
		
		return theToken;
	}
	
	Token *findTokenFromString(std::vector <Token *> *tokens, const char *str, bool addIfNotFound = FALSE)
	{		
		if (!str || !strlen(str))
			return 0;
		
		for (TokenPtrIterator it = tokens->begin(); it != tokens->end(); it++)
			if ((matchToken(*it, str) == TRUE))
				return *it;
		
		if (addIfNotFound == TRUE)
			return newTokenFromString(tokens, str);
		else
			return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////// Suffix Parsing //////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	bool findSeparator (const char **separatorBeg, const char **separatorEnd, const char *suffix)
	{
		*separatorBeg = strpbrk(suffix + 1, ".-");
		*separatorEnd = *separatorBeg + 1;
		
		// Basic sanity and disallowed character check
		
		if (!*separatorBeg || strlen(*separatorBeg) < 2 || strpbrk((*separatorBeg) + 1, ".-") || strpbrk(suffix, " _"))
			return FALSE;
		
		// Modify for multi-character separators
		
		if (**separatorBeg == '-')
		{
			if (**separatorEnd == '>' && strlen(*separatorBeg) > 2)
				separatorEnd++;
			else 
				if (**separatorEnd == 't' && strlen(*separatorBeg) > 4 && *(*(separatorEnd) + 1) == 'o' && *(*(separatorEnd) + 2) == '-')
					*separatorEnd += 3;
		}
		
		return TRUE;
	}
	
	bool parseSuffix(const char *suffix, WDL_String *suffix1, WDL_String *suffix2, WDL_String *separator, bool alwaysValid = FALSE)
	{
		WDL_String suffixTemp;
		
		const char *separatorBeg;
		const char *separatorEnd;
		
		// Basic Sanity
		
		if (strlen(suffix) == 0)
			return FALSE;
		
		if (findSeparator(&separatorBeg, &separatorEnd, suffix) == FALSE)
		{
			if (findTokenFromString(&mSuffixTokens, suffix, alwaysValid))
			{
				suffix1->Set(suffix);
				suffix2->Set("");
				separator->Set("");
				return TRUE;
			}
		}
		else
		{				
			suffixTemp.Set(suffix, separatorBeg - suffix);
			
			if (findTokenFromString(&mSourceTokens, suffixTemp.Get(), alwaysValid) && findTokenFromString(&mReceiverTokens, separatorEnd, alwaysValid))
			{	
				suffix1->Set(suffix, separatorBeg - suffix);
				suffix2->Set(separatorEnd);
				separator->Set(separatorBeg, separatorEnd - separatorBeg);
				return TRUE;
			}
		}
		
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////// Matching for Lexical Analysis ///////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool matchSlot(const char *str)
	{
		const char *dash = strchr(str, '-');
		int i;
		
		// Check that the string is long enough, and only has one dash not in an end position
		
		if (strlen(str) < 3 || !dash || dash == str || strchr(dash + 1, '-') || dash == strrchr(str, 0))
			return FALSE;
		
		// Check that all other characters are numerical digits
		
		for (i = 0; str[i]; i++)
			if (str[i] != '-' && !isdigit(str[i]))
				return FALSE;
		
		for (i = 0, mOpenSlotInput = 0; str + i != dash; i++)
			mOpenSlotInput = (mOpenSlotInput * 10) + (str[i] - 48);
		
		for (i = dash - str + 1, mOpenSlotOutput = 0; str[i]; i++)
			mOpenSlotOutput = (mOpenSlotOutput * 10) + (str[i] - 48);
		
		// Reference to zero
		
		mOpenSlotInput--;
		mOpenSlotOutput--;
		
		return TRUE;
	}
	
	Keyword matchKeyword(const char *str, bool slotIsKeyword = TRUE)
	{
		// Stream is Empty
		
		if(!str || !*str)
			return kKeywordEmpty;
		
		// Synonyms
		
		if (strcmp(str, "synonym:") == 0)	
			return kKeywordSynonym;

		// Hierarchy
		
		if (strcmp(str, "top:") == 0)
			return kKeywordTop;
		if (strcmp(str, "folder:") == 0)
			return kKeywordFolder;
		if (strcmp(str, "list:") == 0)
			return kKeywordList;
		if (strcmp(str, "scheme:") == 0)
			return kKeywordScheme;		
		
		// Scheme items
		
		if (strcmp(str, "channels:") == 0)
			return kKeywordChannels;
		if ((slotIsKeyword == TRUE) && matchSlot(str))
			return kKeywordSlot;
		
		// Parameters
		
		if (strcmp(str, "name:") == 0)
			return kKeywordName;
		if (strcmp(str, "id:") == 0)
			return kKeywordID;
		
		// Inclusions
		
		if (strcmp(str, "include_list:") == 0)
			return kKeywordIncludeList;
		if (strcmp(str, "include_scheme:") == 0)
			return kKeywordIncludeScheme;
		
		return kKeywordNone;
	}
		
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// File Stream ///////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool openStream(const char *loadingSchemeFilePath)
	{
		mFileStream = new std::ifstream;
		mFileStream->open(loadingSchemeFilePath);
		mCurrentReadPoint = 0;
		mCurrentLineString.Set("");
		mCurrentString.Set("");
		mProgress = TRUE;
		mLine = 0;
		
		return mFileStream->is_open();
	}
	
	void closeStream()
	{
		delete mFileStream;
		mFileStream = NULL;
	}	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// Basic File Parsing (retrieve strings / ignore comments) /////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	bool matchTwoChar(const char *str, char char1, char char2)
	{
		return ((str && strlen(str) > 1 && *str == char1 && *(str + 1) == char2)) ? TRUE : FALSE;
	}
	
	bool updateLine()
	{		
		// Check whether this line still has characters
		
		if (mCurrentReadPoint && *mCurrentReadPoint)
			return TRUE;
		
		// Reset current string and get next line

		mCurrentString.Set("");

		std::string tempString;
		getline (*mFileStream, tempString);
		
		// File is done
		
		if (mFileStream->eof())
		{
			mCurrentLineString.Set("");
			mCurrentReadPoint = 0;
			mLine = 0;
			return FALSE;
		}
		
		// Copy current line
		
		mCurrentLineString.Set(tempString.c_str());
		mCurrentReadPoint = mCurrentLineString.Get();
		mLine++;
		
		return TRUE;
	}
	
	const char *commentBlock()
	{		
		mCurrentReadPoint += 2;
		
		while (TRUE)
		{
			if (updateLine() == FALSE)
				return 0;
		
			for ( ; *mCurrentReadPoint; mCurrentReadPoint++)
				if (matchTwoChar(mCurrentReadPoint, '*', '/') == TRUE)
					return mCurrentReadPoint + 2;
		}
	}
	
	bool quotedString()
	{
		const char *nextReadPoint = strchr(mCurrentReadPoint + 1, '"');
		
		if (nextReadPoint)
		{
			mCurrentString.Set(mCurrentReadPoint + 1, nextReadPoint - (mCurrentReadPoint + 1));
			mCurrentReadPoint = nextReadPoint + 1;
			return TRUE;
		}
		
		// Ignore single quotation mark (except as delimiter)
			
		mCurrentReadPoint += 1;
		return FALSE;
	}
	
	const char *nextDelimiter ()
	{
		const char *delimiter = strpbrk(mCurrentReadPoint, " \"/\t\n\v\f\r");
		
		// Ignore single forward slashes
		
		while (delimiter && *delimiter == '/' && *(delimiter + 1) != '/' && *(delimiter + 1) != '*')
			delimiter = strpbrk(mCurrentReadPoint + 1, " \"/\t\n\v\f\r");
		
		return delimiter ? delimiter : strrchr(mCurrentReadPoint, 0);
	}
		
	void nextString()
	{
		const char *nextReadPoint = mCurrentReadPoint;
		
		while (!mCurrentReadPoint || !(*mCurrentReadPoint) || nextReadPoint == mCurrentReadPoint)
		{
			// Load next line if necessary
			
			if (updateLine() == FALSE)
				return;
			
			// Check for comment block
			
			if (matchTwoChar(mCurrentReadPoint, '/', '*') == TRUE) 
				mCurrentReadPoint = commentBlock();
			
			// Check for line comment
			
			if (matchTwoChar(mCurrentReadPoint, '/', '/') == TRUE)
				mCurrentReadPoint = 0;

			// Check for quoted string
			
			if (mCurrentReadPoint && *mCurrentReadPoint == '"')
				if (quotedString() == TRUE)
					return;
			
			// Skip whitespace
			
			while (mCurrentReadPoint && isspace(*mCurrentReadPoint))
				mCurrentReadPoint++;
			
			nextReadPoint = nextDelimiter();
		}
		
		// Copy up until the delimiter
		
		mCurrentString.Set(mCurrentReadPoint, nextReadPoint - mCurrentReadPoint);
		mCurrentReadPoint = nextReadPoint;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Retrieve Specified Types of String /////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	const char *getString (bool progressAfter)
	{				
		if (mProgress == TRUE)
			nextString();
		
		mProgress = progressAfter;
		
		return mCurrentString.Get();
	}
	
	Keyword getKeyword()
	{	
		return matchKeyword(getString(TRUE));
	}
	
	const char *getNonKeyword(bool slotIsKeyword = TRUE)
	{		
		if (matchKeyword(getString(FALSE), slotIsKeyword) == kKeywordNone)
			mProgress = TRUE;
		 
		return (mProgress == TRUE) ? mCurrentString.Get() : 0;
	}
	
	bool getSpecifiedString(const char *str)
	{		
		if (strcmp(getString(FALSE), str) == 0)
			mProgress = TRUE;
		
		return mProgress;
	}
	
	int getInt()
	{
		const char *intString = getString(FALSE);
		int intVal = 0;
		int i;
		
		for (i = 0; intString[i] && isdigit(intString[i]); i++)
			intVal = (intVal * 10) + (intString[i] - 48);

		if (!*intString || intString[i])
			return -1;
		
		mProgress = TRUE;
		return intVal;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Report Parsing Errors Items ///////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void parseError(ParseErrorCode err)
	{
		const char *errString;
		
		switch (err)
		{
			case kErrNone:				
				return;
				
			case kErrFolderHasName:					errString = "name: Folder already has name";			break;
			case kErrListHasName:					errString = "name: List already has name";				break;
			case kErrUnexpectedName:				errString = "name: Unexpected name";					break;
			case kErrNameNoValue:					errString = "name: keyword with no value";				break;
			case kErrListHasID:						errString = "id: List already has ID";					break;
			case kErrSchemeHasID:					errString = "id: Scheme already has ID";				break;
			case kErrUnexpectedID:					errString = "id: Unexpected ID";						break;
			case kErrListIDExists:					errString = "id: List ID already exists";				break;
			case kErrSchemeIDExists:				errString = "id: Scheme ID already exists";				break;
			case kErrIDNoValue:						errString = "id: keyword with no value";				break;
			case kErrListIncludeIDNotGiven:			errString = "include_list: keyword with no value";		break;
			case kErrListIncludeNoOpenFolder:		errString = "include_list: no open folder";				break;
			case kErrListIncludeIDNotFound:			errString = "include_list: ID not found";				break;
			case kErrSchemeIncludeIDNotGiven:		errString = "include_scheme: keyword with no value";	break;
			case kErrSchemeIncludeNoOpenList:		errString = "include_scheme: no open list";				break;
			case kErrSchemeIncludeIDNotFound:		errString = "include_scheme: ID not found";				break;
			case kErrClosedFolderEmpty:				errString = "closed folder is empty";					break;
			case kErrClosedListEmpty:				errString = "closed list is empty";						break;
			case kErrClosedListNoID:				errString = "closed list not in folder without ID";		break;
			case kErrClosedSchemeEmpty:				errString = "closed scheme is empty";					break;
			case kErrClosedSchemeNoID:				errString = "closed scheme not in list without ID";		break;
			case kErrSlotNoOpenScheme:				errString = "slot: no open scheme";						break;
			case kErrSlotNotGiven:					errString = "slot: no values given";					break;
			case kErrSlotInvalidSuffix:				errString = "slot: invalid suffix";						break;
			case kErrSlotAlreadyDefined:			errString = "slot: slot already defined";				break;
			case kErrChannelsNoOpenScheme:			errString = "channels: no open scheme";					break;
			case kErrChannelsNotGiven:				errString = "channels: no values given";				break;
			case kErrChannelsAlreadyDefined:		errString = "channels: already defined";				break;
			case kErrUnexpectedNonKeyword:			errString = "parsing - unexpected non keyword";			break;
 		}
		
		// FIX - different outputs
		
		printf("Parsing error @ string \"%s\" - line %d - %s\n", mCurrentString.Get(), mLine, errString);
		fflush(stdout);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// Close Items ///////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void closeScheme()
	{
		ParseErrorCode err = kErrNone;
		
		if (!mOpenScheme)
			return;
		
		if (!mOpenList && !mOpenScheme->mID.GetLength())
			err = kErrClosedSchemeNoID;
		if (mOpenScheme->mSlots.size() == 0)
			err = kErrClosedSchemeEmpty;
				
		if (err == kErrNone)
		{
			mSchemes.push_back(mOpenScheme);
			mOpenList->mSchemes.push_back(mOpenScheme);
		}
		else 
			delete mOpenScheme;
		
		parseError(err);
		mOpenScheme = 0;
	}
	
	void closeList()
	{
		ParseErrorCode err = kErrNone;
		
		closeScheme();
		
		if (!mOpenList)
			return;
		
		if (!mOpenFolder && !mOpenList->mID.GetLength())
			err = kErrClosedListNoID;
		if (mOpenList->mSchemes.size() == 0)
			err = kErrClosedListEmpty;
		
		if (err == kErrNone)
		{
			mLists.push_back(mOpenList);
			mOpenFolder->mLists.push_back(mOpenList);
		}
		else
			delete mOpenList;	
		
		parseError(err);
		mOpenList = 0;
	}
	
	void closeFolder()
	{
		closeList();
		
		ParseErrorCode err = kErrNone;
		
		if (!mOpenFolder) 
			return;
		
		if (mOpenFolder->mLists.size() == 0)
			err  = kErrClosedFolderEmpty;
		
		if (err == kErrNone)
			mFolders.push_back(mOpenFolder);
		else
			delete mOpenFolder;
		
		parseError(err);
		mOpenFolder = 0;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// Open Items ////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void openFolder()
	{
		closeFolder();
		mOpenFolder = new Folder;
	}
	
	void openList()
	{
		closeList();
		mOpenList = new List;
	}
	
	void openScheme()
	{
		closeScheme();
		mOpenScheme = new Scheme;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////// Scheme Item - Slot ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	ParseErrorCode addSlot(int iChan, int oChan, int chan, const char *suffix1, const char *suffix2, const char *separator, bool optional)
	{
		if (!mOpenScheme)
			return kErrSlotNoOpenScheme;
		
		for (SlotIterator currentSlot = mOpenScheme->mSlots.begin(); currentSlot != mOpenScheme->mSlots.end(); currentSlot++)
			if ((*currentSlot).mIChan == iChan && (*currentSlot).mOChan == oChan)
				return kErrSlotAlreadyDefined;
				
		Token *suffix1Token = findTokenFromString(&mSourceTokens, suffix1, TRUE);
		Token *suffix2Token = findTokenFromString(&mReceiverTokens, suffix2, TRUE);
		Token *separatorToken = findTokenFromString(&mSeparatorTokens, separator, TRUE);
		
		mOpenScheme->mSlots.push_back(Slot(iChan, oChan, chan, suffix1Token, suffix2Token, separatorToken, optional));
		
		return kErrNone;
	}
	
	ParseErrorCode parseSlot()
	{
		ParseErrorCode err = kErrNone;
		
		const char *suffix = getNonKeyword(FALSE);
		
		WDL_String suffix1;
		WDL_String suffix2;
		WDL_String separator;
		
		int iChan = mOpenSlotInput;
		int oChan = mOpenSlotOutput;
		
		int chan = 0;
		bool optional;
		
		if (!suffix)
			return kErrSlotNotGiven;
		
		if (parseSuffix(suffix, &suffix1, &suffix2, &separator, TRUE) == FALSE)
			err = kErrSlotInvalidSuffix;
		
		// Reference channel to zero
		
		chan = getInt() - 1;
		chan = chan < 0 ? 0 : chan;
		
		optional = getSpecifiedString("?");
		
		if (err == kErrNone)
			return addSlot(iChan, oChan, chan, suffix1.Get(), suffix2.Get(), separator.Get(), optional);
		else 
			return err;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////// Scheme Item - Channels //////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	ParseErrorCode setChannels(int iChan, int oChan, bool minIChan, bool minOChan)
	{
		if (!mOpenScheme)
			return kErrChannelsNoOpenScheme;
		
		if (mOpenScheme->mNumIChan != -1 && mOpenScheme->mNumOChan != -1)
			return kErrChannelsAlreadyDefined;
		
		mOpenScheme->mNumIChan = iChan;
		mOpenScheme->mNumOChan = oChan;
		mOpenScheme->mMinIChan = minIChan;
		mOpenScheme->mMinOChan = minOChan;
		
		return kErrNone;
	}
	
	bool parseMinimum()
	{		
		if (getSpecifiedString("min") == TRUE)
			return TRUE;
		
		getSpecifiedString("req");
		return FALSE;
	}
	
	ParseErrorCode parseChannels()
	{		
		int iChan;
		int oChan;
		
		bool minIChan = FALSE;
		bool minOChan = FALSE;
		
		const char *minString;
		
		iChan = getInt();
		oChan = getInt();
		minString = getNonKeyword();
		minIChan = parseMinimum();
		minOChan = minIChan;
		
		if (iChan == -1)
			return kErrChannelsNotGiven;
		
		if (oChan == -1)
		{
			oChan = getInt();
			minString = getNonKeyword();
			minOChan = parseMinimum();
		}
		
		if (oChan == -1)
		{
			oChan = iChan;
			minOChan = minIChan;
		}
		
		return setChannels(iChan, oChan, minIChan, minOChan);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// Parameters ////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	ParseErrorCode setName(const char *str)
	{		
		if (!str)
			return kErrNameNoValue;
		
		if (mOpenList)
		{
			if (mOpenList->mName.GetLength())
				return kErrListHasName;
			
			mOpenList->mName.Set(str);
			return kErrNone;
		}
		
		if (mOpenFolder)
		{
			if (mOpenFolder->mName.GetLength())
				return kErrFolderHasName;
				
			mOpenFolder->mName.Set(str);
			return kErrNone;
		}
		
		return kErrUnexpectedName;
	}
	
	ParseErrorCode setID(const char *str)
	{
		if (!str)
			return kErrIDNoValue;

		if (mOpenScheme)
		{
			for (SchemePtrIterator it = mSchemes.begin(); it !=  mSchemes.end(); it++)
				if (strcmp(str, (*it)->mID.Get()) == 0)
					return kErrSchemeIDExists;
			
			if (mOpenScheme->mID.GetLength())
				return kErrSchemeHasID;
			
			mOpenScheme->mID.Set(str);
			return kErrNone;
		}
		
		if (mOpenList)
		{			
			for (ListPtrIterator it = mLists.begin(); it !=  mLists.end(); it++)
				if (strcmp(str, (*it)->mID.Get()) == 0)
					return kErrListIDExists;
			
			if (mOpenList->mID.GetLength())
				return kErrListHasID;
			
			mOpenList->mID.Set(str);
			return kErrNone;
		}
		
		return kErrUnexpectedID;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// Inclusions ////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	ParseErrorCode includeList(const char *str)
	{		
		closeList();
		
		if (!str)
			return kErrListIncludeIDNotGiven;
		
		if (!mOpenFolder) 
			return kErrListIncludeNoOpenFolder;
		
		for (ListPtrIterator it = mLists.begin(); it !=  mLists.end(); it++)
		{
			if (strcmp(str, (*it)->mID.Get()) == 0)
			{
				mOpenFolder->mLists.push_back(*it);
				return kErrNone;
			}
		}
		
		return kErrListIncludeIDNotFound;
	}
	
	ParseErrorCode includeScheme(const char *str)
	{		
		closeScheme();
		
		if (!str)
			return kErrSchemeIncludeIDNotGiven;
		
		if (!mOpenList) 
			return kErrSchemeIncludeNoOpenList;
		
		for (SchemePtrIterator it = mSchemes.begin(); it !=  mSchemes.end(); it++)
		{
			if (strcmp(str, (*it)->mID.Get()) == 0)
			{
				mOpenList->mSchemes.push_back(*it);
				return kErrNone;
			}
		}
		
		return kErrSchemeIncludeIDNotFound;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Main file parsing routine for scheme definitions /////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool parse(const char *loadingSchemeFilePath)
	{
		Keyword nextKeyword = kKeywordNone;
		
		if (!openStream(loadingSchemeFilePath))
		{
			closeStream();
			return FALSE;
		}
		
		// FIX - First do synonyms......

		while (nextKeyword != kKeywordEmpty)
		{
			ParseErrorCode err = kErrNone;
			
			switch (nextKeyword = getKeyword())
			{
					// Hierarchy
					
				case kKeywordTop:				closeFolder();							break;
				case kKeywordFolder:			openFolder();							break;
				case kKeywordList:				openList();								break;
				case kKeywordScheme:			openScheme();							break;
					
					// Scheme items
					
				case kKeywordSlot:				err = parseSlot();						break;
				case kKeywordChannels:			err = parseChannels();					break;
					
					// Parameters
					
				case kKeywordName:				err = setName(getNonKeyword());			break;
				case kKeywordID:				err = setID(getNonKeyword());			break;
					
					// Inclusions
					
				case kKeywordIncludeList:		err = includeList(getNonKeyword());		break;
				case kKeywordIncludeScheme:		err = includeScheme(getNonKeyword());	break;
					
					// Items to ignore
					
				case kKeywordNone:
                
					err = kErrUnexpectedNonKeyword;
					
				case kKeywordSynonym:
					
					while (getNonKeyword());
					break;
                    
                case kKeywordEmpty:
                    break;
			}
			
			parseError(err);
		}
		
		closeFolder();
		closeStream();
		
		return TRUE;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////// Basic Path / Name Operations ///////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
	
	void getExtension(WDL_String *extension, WDL_String *path)
	{
		const char *pathCString = path->Get();
		const char *extCString = strrchr(pathCString, '.');
		
		if (extCString)
			extension->Set(extCString);
		else
			extension->Set("");
	}
	
	void removeExtension(WDL_String *extensionRemoved, WDL_String *path)
	{  
		const char *pathCString = path->Get();
		const char *extCString = strrchr(pathCString, '.');
		
		if (extCString)
			extensionRemoved->Set(pathCString, extCString - pathCString);
		else
			extensionRemoved->Set(path);
	}

	void getFileFromPath(WDL_String *fileName, WDL_String *path)
	{  
		const char *pathCString = path->Get();
		const char *fileNameCString = strrchr(pathCString, '/');
		
		if (fileNameCString)
			fileName->Set(fileNameCString + 1);
		else
			fileName->Set(path);
	}
		
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////// Base Name / Path /////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool getBaseName(WDL_String *baseName, WDL_String *filePath)
	{
		WDL_String fileName;
		WDL_String suffix1;
		WDL_String suffix2;
		WDL_String separator;
		bool success;
		
		getFileFromPath(&fileName, filePath);	
		success = getBasePath(baseName, &suffix1, &suffix2, &separator, &fileName);
		baseName->Append(" . . .");
		
		return success;
	}
	
	bool getBasePath(WDL_String *baseName, WDL_String *suffix1, WDL_String *suffix2, WDL_String *separator, WDL_String *path)
	{
		WDL_String extensionRemoved;
		
		removeExtension(&extensionRemoved, path);
		
		const char *file = extensionRemoved.Get();
		const char *suffix = 0;
		const char *suffixTest;
		const char *prevDot = 0;
						
		for (suffixTest = strpbrk(file, " ._"); suffixTest; suffixTest = strpbrk(suffixTest + 1, " ._"))
		{
			
			if (!suffixTest)
				break;
			
			// Store suffix point (accounting correctly for strings with two or more dots)
			
			if (*suffixTest == '.')
			{
				if (prevDot)
					suffix = prevDot;
				else
					suffix = suffixTest;
			
				prevDot = suffixTest;
			}
			else
				suffix = suffixTest;
		}
					
		if (!suffix || parseSuffix(suffix + 1, suffix1, suffix2, separator) == FALSE)
		{
			baseName->Set(file);
			return FALSE;
		}
		
		baseName->Set(file, suffix - file + 1);
		return TRUE;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////// Scheme Based Matching //////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
private:
	
	bool schemeChannelCheck(Scheme *scheme, int activeIns, int activeOuts)
	{
		bool check = TRUE;
		
		if (scheme->mNumIChan > 0)
		{
			if (scheme->mMinIChan == TRUE)
				check = (activeIns < scheme->mNumIChan) ? FALSE : check;
			else
				check = (activeIns != scheme->mNumIChan) ? FALSE : check;
		}
		
		if (scheme->mNumOChan > 0)
		{
			if (scheme->mMinOChan == TRUE)
				check = (activeOuts < scheme->mNumOChan) ? FALSE : check;
			else
				check = (activeOuts != scheme->mNumOChan) ? FALSE : check;
		}
		
		return check;
	}
	
    bool checkFileExist(const char *path, unsigned int channel)
    {
        HISSTools::IAudioFile file(path);
        
        if (file.isOpen() && !file.getErrorFlags() && file.getChannels() > channel)
            return true;
        
        return false;
    }
    
	SlotIterator matchFileWithScheme(Scheme *scheme, WDL_String *path, WDL_String *basePath, WDL_String *suffix1, WDL_String *suffix2, WDL_String *separator, WDL_String *extension)
	{
		for (SlotIterator  currentSlot = scheme->mSlots.begin(); currentSlot != scheme->mSlots.end(); currentSlot++)
		{
			// Check that we have a match between the existence of a separator in the slot definition and the file path and that we have enough channels
			
			if ((((*currentSlot).mSeparator != 0) != (separator->GetLength() != 0)) || checkFileExist(path->Get(), (*currentSlot).mChan) == 0)
				continue;
			
			// Check that the first suffix part matches
			
			if (matchToken((*currentSlot).mSuffix1, suffix1->Get()) == FALSE)
				continue;
		
			// If there is no separator we have a match
			
			if (!(*currentSlot).mSeparator)
				return currentSlot;
			
			// Check that the separator matches
			
			if (matchToken((*currentSlot).mSeparator, separator->Get()) == FALSE)
				continue;
			
			// Check that the second suffix part matches
			
			if (matchToken((*currentSlot).mSuffix2, suffix2->Get()) == TRUE)
				return currentSlot;
		}					
	
		return scheme->mSlots.end();
	}
	
	bool findFileForSlot(WDL_String *returnedPath, Slot *slot, WDL_String *basePath, WDL_String *extension)
	{
		StringIterator sepIt;
		StringIterator suf1It;
		StringIterator suf2It;

		if (slot->mSeparator)
		{
			for (suf1It = slot->mSuffix1->mStrings.begin(); suf1It != slot->mSuffix1->mStrings.end(); suf1It++)
			{
				for (sepIt = slot->mSeparator->mStrings.begin(); sepIt != slot->mSeparator->mStrings.end(); sepIt++)
				{
					for (suf2It = slot->mSuffix2->mStrings.begin(); suf2It != slot->mSuffix2->mStrings.end(); suf2It++)
					{
						returnedPath->SetFormatted (basePath->GetLength() + strlen((*suf1It).Get()) + strlen((*sepIt).Get()) + strlen((*suf2It).Get()) + extension->GetLength(), 
												 "%s%s%s%s%s", basePath->Get(), (*suf1It).Get(), (*sepIt).Get(), (*suf2It).Get(), extension->Get());
						if (checkFileExist(returnedPath->Get(), slot->mChan))
							return TRUE;
					}
				}
			}
		}
		else 
		{
			for (suf1It = slot->mSuffix1->mStrings.begin(); suf1It != slot->mSuffix1->mStrings.end(); suf1It++)
			{
				returnedPath->SetFormatted(basePath->GetLength() + strlen((*suf1It).Get()) + extension->GetLength(), "%s%s%s", basePath->Get(), (*suf1It).Get(), extension->Get());
				if (checkFileExist(returnedPath->Get(), slot->mChan))
					return TRUE;
			}
		}
			 
		return FALSE;
	}
	
	bool slotOptionalByChannel(Scheme *scheme, int iChan, int oChan, int activeIns, int activeOuts)
	{		
		if (scheme->mNumIChan <= 0 || (scheme->mMinIChan == TRUE && iChan >= activeIns))
			return TRUE;
		
		if (scheme->mNumOChan <= 0 || (scheme->mMinOChan == TRUE && oChan >= activeOuts))
			return TRUE;
				
		return FALSE;
	}
	
public:
	
	bool loadWithScheme(WDL_String *path, FileList *fileList, int activeIns, int activeOuts)
	{
		WDL_String returnedPath;
		WDL_String basePath;
		WDL_String extension;
		WDL_String suffix1;
		WDL_String suffix2;
		WDL_String separator;
		
		List *currentList = mList;		
		
		SchemePtrIterator currentScheme;

		SlotIterator currentSlot;
		SlotIterator matchedSlot;

		// FIX - default behaviours....
		
		fileList->clear();
		
		if (getBasePath(&basePath, &suffix1, &suffix2, &separator, path) == FALSE)
			return FALSE;
				
		getExtension(&extension, path);
		
		// Loop through schemes for file matching
		
		for (currentScheme = currentList->mSchemes.begin(); currentScheme != currentList->mSchemes.end(); currentScheme++)
		{
			// Clear the list for the new scheme
			
			fileList->clear();
			
			// Check that the scheme matches the currently active channels
			
			if (schemeChannelCheck(*currentScheme, activeIns, activeOuts) == FALSE)
				continue;
			
			// Match the chosen file
							
			matchedSlot = matchFileWithScheme(*currentScheme, path, &basePath, &suffix1, &suffix2, &separator, &extension);
				
			if (matchedSlot == (*currentScheme)->mSlots.end())
				continue;
			
			// Check for the remaining files
			
			for (currentSlot = (*currentScheme)->mSlots.begin(); currentSlot != (*currentScheme)->mSlots.end(); currentSlot++)
			{				
				// If we have the matched slot then use the input path, otherwise attempt to find a relevant file
				
				if (currentSlot == matchedSlot)
					fileList->setFile((*currentSlot).mIChan, (*currentSlot).mOChan, path->Get(), (*currentSlot).mChan);
				else
				{
					if (findFileForSlot(&returnedPath, &(*currentSlot), &basePath, &extension) == TRUE)
						fileList->setFile((*currentSlot).mIChan, (*currentSlot).mOChan, returnedPath.Get(), (*currentSlot).mChan);
					else
					{
						// If the file is not present check if it is optional in the slot defintion or optional acording to the channel specification
					
						if (((*currentSlot).mOptional == FALSE) && slotOptionalByChannel(*currentScheme, (*currentSlot).mIChan, (*currentSlot).mOChan, activeIns, activeOuts) == FALSE)
							break;
					}
				}
			}
				 
			if (currentSlot == (*currentScheme)->mSlots.end())
				return TRUE;
		}
		
		return FALSE;
	}
};
