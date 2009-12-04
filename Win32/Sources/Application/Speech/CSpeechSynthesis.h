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


// CSpeechSynthesis.h

// Class to handle speech sythesis

#ifndef __CSPEECHSYNTHESIS__MULBERRY__
#define __CSPEECHSYNTHESIS__MULBERRY__

//#pragma ARM_conform on
#include <sapi.h>
//#pragma ARM_conform off

#include "cdstring.h"
#include "cdmutex.h"

#include <deque>
#include <queue>
#include <vector>

// Classes

enum EMessageSpeakItem
{
	eMessageSpeakFrom1 = 1,
	eMessageSpeakFromAll,
	eMessageSpeakTo1,
	eMessageSpeakToAll,
	eMessageSpeakCC1,
	eMessageSpeakCCAll,
	eMessageSpeakDate,
	eMessageSpeakSubject,
	eMessageSpeakNumParts,
	eMessageSpeakBodyNoHdr,
	eMessageSpeakBodyHdr,
	eMessageSpeakNone
};

// Define vector

class CMessageSpeak;
typedef vector<CMessageSpeak> CMessageSpeakVector;

class CMessageSpeak
{
public:
	EMessageSpeakItem	mItem;
	cdstring			mItemText;
	
	int operator==(const CMessageSpeak& other) const			// Compare with same type
					{ return ((mItem == other.mItem) && (mItemText == other.mItemText)); }
	int operator!=(const CMessageSpeak& other) const			// Compare with same type
					{ return !(*this == other); }
};

class CMbox;
class CMessage;
class CAttachment;

class CSpeechSynthesis
{
public:
	enum ERate
	{
		eRateFast = 1,
		eRateNormal,
		eRateSlow
	};

	enum EPitch
	{
		ePitchHigh = 1,
		ePitchMid,
		ePitchLow
	};

	static cdstrvect sItemList;

	static bool	InstallSpeechSynthesis();				// Check for speech
	static void	RemoveSpeechSynthesis();				// Remove speech
	static void InstallMenu(CMenu* menu);				// Create menu
	
	static bool	Available(void)					// Is it available
		{ return sAvailable; }

	// Handle commands from menus
	static bool		OnUpdateEditSpeak(UINT nID, CCmdUI* pCmdUI);
	static bool		OnEditSpeak(UINT nID);

	static bool		Speaking(void);
	static void		StopSpeaking(void);

	static void		SpendTime(void);

	// Speak specific items
	static void		SpeakString(const cdstring& txt);
	static void		SpeakNewMessages(CMbox* mbox);
	static void		SpeakMessage(CMessage* msg, CAttachment* attach, bool letter);

protected:
			CSpeechSynthesis();
			~CSpeechSynthesis();

private:
    static CComPtr<ISpVoice> sSpVoice;
	static bool sAvailable;

#ifdef __MULBERRY
	static cdmutex _mutex;				// Protect queue against multi-thread access
#endif
	static cdstrqueue sSpeakQueue;
	static bool sTopDone;

	static void InstallVoices(void);
	static void ChangeVoice(short voiceSelection);

	static void	SpeakText(char* txt, long length);
};

#endif
