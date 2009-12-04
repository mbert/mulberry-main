/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// CMailNotification.h

// Class to describe a mail notification style

#ifndef __CMAILNOTIFICATION__MULBERRY__
#define __CMAILNOTIFICATION__MULBERRY__

#include "CFutureItems.h"

#include "cdstring.h"
#include "prefsvector.h"

#include <time.h>

class CPreferences;

class CMailNotification
{
public:
		CMailNotification();
		CMailNotification(const CMailNotification& copy)			// Copy construct
			{ _copy(copy); }
	virtual ~CMailNotification() {}
		
		CMailNotification& operator=(const CMailNotification& copy)				// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }
		int operator==(const CMailNotification& test) const;					// Compare with same type

	// Getters/Setters
	const cdstring& GetName(void) const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }
	
	const cdstring& GetFavouriteID(void) const
		{ return mFavouriteID; }
	void SetFavouriteID(const cdstring& id)
		{ mFavouriteID = id; }
	unsigned long GetFavouriteIndex(const CPreferences* prefs) const;

	bool IsEnabled(void) const
		{ return mEnabled; }
	void Enable(bool enable)
		{ mEnabled = enable; }
	
	long GetCheckInterval(void) const
		{ return mCheckInterval; }
	void SetCheckInterval(long check_interval)
		{ mCheckInterval = check_interval; }
	
	time_t GetLastCheck(void) const
		{ return mLastCheck; }
	void SetLastCheck(time_t last_check)
		{ mLastCheck = last_check; }
	
	bool GetCheckPending(void) const
		{ return mCheckPending; }
	void SetCheckPending(bool check)
		{ mCheckPending = check; }

	bool DoShowAlertForeground(void) const
		{ return mShowAlertForeground; }
	void SetShowAlertForeground(bool show_alert)
		{ mShowAlertForeground = show_alert; }

	bool DoShowAlertBackground(void) const
		{ return mShowAlertBackground; }
	void SetShowAlertBackground(bool show_alert)
		{ mShowAlertBackground = show_alert; }

	bool DoFlashIcon(void) const
		{ return mFlashIcon; }
	void SetFlashIcon(bool flash_icon)
		{ mFlashIcon = flash_icon; }

	bool DoPlaySound(void) const
		{ return mPlaySound; }
	void SetPlaySound(bool play_sound)
		{ mPlaySound = play_sound; }

	const cdstring& GetSoundID(void) const;
	void SetSoundID(const cdstring& sound_id);

	bool DoSpeakText(void) const
		{ return mSpeakText; }
	void SetSpeakText(bool speak_text)
		{ mSpeakText = speak_text; }

	const cdstring& GetTextToSpeak(void) const
		{ return mTextToSpeak; }
	void SetTextToSpeak(const cdstring& text_to_speak);
	bool SpeakCombined() const
		{ return mSpeakCombined; }
	bool SpeakSeparate() const
		{ return mSpeakSeparate; }

	bool GetOpenMbox(void) const
		{ return mOpenMbox; }
	void SetOpenMbox(bool open_mbox)
		{ mOpenMbox = open_mbox; }

	bool GetCheckOnce(void) const
		{ return mCheckOnce; }
	void SetCheckOnce(bool check_once)
		{ mCheckOnce = check_once; }

	bool GetCheckNew(void) const
		{ return mCheckNew; }
	void SetCheckNew(bool check_new)
		{ mCheckNew = check_new; }

	// Serialize
	bool SetInfo(char_stream& info, NumVersion vers_prefs);		// Parse S-Expression element
	cdstring GetInfo(void) const;								// Create S_Expression element

private:
	cdstring	mName;
	cdstring	mFavouriteID;
	bool		mEnabled;
	long		mCheckInterval;
	time_t		mLastCheck;			// Last time it was checked
	bool		mCheckPending;		// Check is currently pending

	bool		mShowAlertForeground;
	bool		mShowAlertBackground;
	bool		mFlashIcon;
	bool		mPlaySound;
	cdstrmap	mSoundID;
	bool		mSpeakText;
	cdstring	mTextToSpeak;
	bool		mSpeakCombined;
	bool		mSpeakSeparate;
	
	bool		mOpenMbox;
	bool		mCheckOnce;
	bool		mCheckNew;

	CFutureItems	mFuture;
	
	void _copy(const CMailNotification& copy);
};

typedef prefsvector<CMailNotification> CMailNotificationList;

#endif
