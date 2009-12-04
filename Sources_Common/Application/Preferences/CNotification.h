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


// CNotification.h

// Class to describe a mail notification style

#ifndef __CNOTIFICATION__MULBERRY__
#define __CNOTIFICATION__MULBERRY__

#include "CFutureItems.h"

#include "cdstring.h"

class CNotification
{
public:
		CNotification();
		CNotification(const CNotification& copy)			// Copy construct
			{ _copy(copy); }

	virtual ~CNotification() {}
		
		CNotification& operator=(const CNotification& copy)					// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }
		int operator==(const CNotification& test) const;					// Compare with same type

	// Do it
	void DoNotification(const char* rsrcid) const;

	// Getters/Setters
	bool DoShowAlert(void) const
		{ return mShowAlert; }
	void SetShowAlert(bool show_alert)
		{ mShowAlert = show_alert; }

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
	void SetTextToSpeak(const cdstring& text_to_speak)
		{ mTextToSpeak = text_to_speak; }

	// Serialize
	bool SetInfo(char_stream& info, NumVersion vers_prefs);			// Parse S-Expression element
	cdstring GetInfo(void) const;									// Create S_Expression element

private:
	bool			mShowAlert;
	bool			mPlaySound;
	cdstrmap		mSoundID;
	bool			mSpeakText;
	cdstring		mTextToSpeak;
	CFutureItems	mFuture;
	
	void _copy(const CNotification& copy);
};

#endif
