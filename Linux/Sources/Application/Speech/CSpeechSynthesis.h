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


#include <Speech.h>

#include "cdstring.h"

#include <deque>
#include <queue>
#include <vector>

// Commands

const CommandT	cmd_SpeakSelection = 11000;
const CommandT	cmd_Rate = 11001;
const CommandT	cmd_Pitch = 11002;
const CommandT	cmd_Voice = 11003;

const CommandT	cmd_RateFast = 11100;
const CommandT	cmd_RateNormal = 11101;
const CommandT	cmd_RateSlow = 11102;

const CommandT	cmd_PitchHigh = 11200;
const CommandT	cmd_PitchMid = 11201;
const CommandT	cmd_PitchLow = 11202;

// Resources
const ResIDT	MENU_Speak = 135;

const ResIDT	MENU_Rate = 65;

const ResIDT	MENU_Pitch = 66;

const ResIDT	MENU_Voice = 67;

const ResIDT	STRx_Speech = 11100;
enum
{
	str_OldVersionForPPC = 1,
	str_SpeakMessage,
	str_SpeakSelection,
	str_SpeakStop,
	str_DefaultAlert,
	str_DefaultFrom,
	str_DefaultTo,
	str_DefaultSubject,
	str_DefaultBody,
	str_DefaultEnd,
	str_EmptyItem
};
const ResIDT	STRx_SpeechMessageItems = 11101;

// Classes

class LMenu;

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
typedef std::vector<CMessageSpeak> CMessageSpeakVector;

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

#ifndef MSIPL_PARTIAL_SPECIALIZATION
__MSL_FIX_ITERATORS__(CMessageSpeak);
__MSL_FIX_ITERATORS__(const CMessageSpeak);
#endif

class CMbox;
class CMessage;

class CSpeechSynthesis {

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

	static bool	InstallSpeechSynthesis(void);			// Check for speech and install menus etc
	
	static bool	Available(void)					// Is it available
						{ return sAvailable; }

	// Handle commands from menus
	static Boolean	ObeyCommand(CommandT inCommand, void *ioParam = nil);
	static Boolean	FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											Char16 &outMark,
											Str255 outName);

	static bool	Speaking(void)
						{ return ::SpeechBusy(); }
	static void		StopSpeaking(void);

	static void		SpendTime(void);

	// Speak specific items
	static void		SpeakString(const cdstring& txt);
	static void		SpeakNewMessages(CMbox* mbox);
	static void		SpeakMessage(CMessage* msg, unsigned long part, bool letter);

protected:
			CSpeechSynthesis();
			~CSpeechSynthesis();

private:

	static bool sAvailable;
	static short sCurrentVoice;
	static SpeechChannel sSpeechChan;
	static Fixed sDefaultRate;
	static Fixed sDefaultPitch;
	static LMenu* sRateMenu;
	static LMenu* sPitchMenu;
	static LMenu* sVoiceMenu;
	static ERate sCurrentRate;
	static EPitch sCurrentPitch;

	static cdstrqueue sSpeakQueue;
	static bool sTopDone;

	static void InstallVoices(void);
	static void ChangeVoice(short voiceSelection);

	static void		SpeakText(char* txt, long length);
};

#endif
