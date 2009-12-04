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


#include <SpeechSynthesis.h>

#include "cdstring.h"

#include <deque>
#include <queue>
#include <vector>

// Resources
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

class CMbox;
class CMessage;
class CAttachment;

class CSpeechSynthesis {

public:
	static cdstrvect sItemList;

	static bool	InstallSpeechSynthesis(void);			// Check for speech and install menus etc
	
	static bool	Available(void)					// Is it available
						{ return sAvailable; }

	// Handle commands from menus
	static Boolean	ObeyCommand(CommandT inCommand, void *ioParam = nil);
	static Boolean	FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	static bool	Speaking(void)
						{ return ::SpeechBusy(); }
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

	static bool sAvailable;
	static short sCurrentVoice;
	static SpeechChannel sSpeechChan;
	static Fixed sDefaultRate;
	static Fixed sDefaultPitch;

	static cdstrqueue sSpeakQueue;
	static bool sTopDone;

	static void InstallVoices(void);
	static void ChangeVoice(short voiceSelection);

	static void		SpeakText(char* txt, long length);
};

#endif
