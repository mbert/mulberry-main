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


// CSpeechSynthesis.cp

// Class to handle speech sythesis

#include "CSpeechSynthesis.h"

#include "CAddressList.h"
#include "CCommands.h"
#include "CErrorHandler.h"
#include "CMessage.h"
#include "CPreferences.h"

// === Static Members ===

bool CSpeechSynthesis::sAvailable = false;
short CSpeechSynthesis::sCurrentVoice = 0;
SpeechChannel CSpeechSynthesis::sSpeechChan = nil;
Fixed CSpeechSynthesis::sDefaultRate = 0.0;
Fixed CSpeechSynthesis::sDefaultPitch = 0.0;
cdstrqueue CSpeechSynthesis::sSpeakQueue;
bool CSpeechSynthesis::sTopDone = false;
cdstrvect CSpeechSynthesis::sItemList;

// === constants ===

const char* cSpeakPause = ", ";
const char* cSpeakEndSentence = ". ";

// __________________________________________________________________________________________________
// C L A S S __ C A B O U T D I A L O G
// __________________________________________________________________________________________________

CSpeechSynthesis::CSpeechSynthesis()
{
}

CSpeechSynthesis::~CSpeechSynthesis()
{
}

// Check for speech and install menus etc
bool CSpeechSynthesis::InstallSpeechSynthesis()
{
	try
	{
		//  First check to see if the Speech Manager extension is present.
		ThrowIf_(!UEnvironment::HasGestaltAttribute(gestaltSpeechAttr, gestaltSpeechMgrPresent));
#if PP_Target_Classic
		// Check for suitable PPC glue on PPC machines
#ifdef __powerc
		if (!UEnvironment::HasGestaltAttribute(gestaltSpeechAttr, gestaltSpeechHasPPCGlue))
		{
			CErrorHandler::PutStopAlert(STRx_Speech, str_OldVersionForPPC);
			ThrowIf_(true);
		}
#endif
#endif
		InstallVoices();

		// Get descriptor strings
		for(short i = eMessageSpeakFrom1; i <= eMessageSpeakNone; i++)
		{
			LStr255 pstr(STRx_SpeechMessageItems, i);
			cdstring str = pstr;
			sItemList.push_back(str);
		}

		// Success!
		sAvailable = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Delete menu items
		ResIDT res;
		MenuHandle menuH;
		SInt16 item;
		LMenuBar::GetCurrentMenuBar()->FindMenuItem(cmd_SpeakSelection, res, menuH, item);
		LMenuBar::GetCurrentMenuBar()->FetchMenu(res)->RemoveItem(item);
		LMenuBar::GetCurrentMenuBar()->FetchMenu(res)->RemoveItem(item);

		// Not loaded - do nothing
		sAvailable = false;
	}

	return sAvailable;
}

// Install voice list in menu
void CSpeechSynthesis::InstallVoices(void)
{
	short				voiceCount;
	short				i;
	VoiceSpec			voice;
	VoiceDescription	desc;
	VoiceDescription	defaultDesc;

	// Get the description for the system default voice
	defaultDesc.length = sizeof(VoiceDescription);
	ThrowIfOSErr_(::GetVoiceDescription(nil, &defaultDesc, defaultDesc.length));

	// Count the number of available voices (for all synthesizers)
	ThrowIfOSErr_(::CountVoices(&voiceCount));

	// Install each voice in the Voice menu and make the system default the active voice
	for (i = 1; i <= voiceCount; ++i)
	{
		// get the next indexed voice
		ThrowIfOSErr_(::GetIndVoice(voiceCount - i + 1, &voice));

		desc.length = sizeof (VoiceDescription);
		ThrowIfOSErr_(::GetVoiceDescription(&voice, &desc, desc.length));

		// Check for default voice
		if ((desc.voice.id == defaultDesc.voice.id)
			&& (desc.language == defaultDesc.language))
			// Change to default voice
			ChangeVoice(i);
	}
}

/*
**	Respond to Voice Change selection.
*/
void CSpeechSynthesis::ChangeVoice(short voiceSelection)
{
	OSErr err = noErr;
	// Only change if different
	if (voiceSelection == sCurrentVoice)
		return;

	short voiceCount;
	VoiceSpec	voice;

	ThrowIfOSErr_(::CountVoices(&voiceCount));

	// Get the requested voice spec
	ThrowIfOSErr_(::GetIndVoice (voiceCount - voiceSelection + 1, &voice));

	// Get rid of the old speech channel
	if (sCurrentVoice)
		// Release the channel
		ThrowIfOSErr_(::DisposeSpeechChannel(sSpeechChan));

	// Allocate a new speech channel
	ThrowIfOSErr_(::NewSpeechChannel(&voice, &sSpeechChan));

	/* Set the default speaking rate and pitch */
	ThrowIfOSErr_(::GetSpeechRate(sSpeechChan, &sDefaultRate));
	ThrowIfOSErr_(::GetSpeechPitch(sSpeechChan, &sDefaultPitch));

	// Assign new value
	sCurrentVoice = voiceSelection;  /* Assign the global voice selection */
}

// Handle commands from menus
Boolean CSpeechSynthesis::ObeyCommand(CommandT inCommand, void *ioParam)
{
	if (!Available()) return false;

	bool cmdHandled = false;

	switch (inCommand)
	{
	case cmd_SpeakSelection:
		if (Speaking())
		{
			StopSpeaking();
			cmdHandled = true;

			// Force command status update to refresh menu item
			LCommander::SetUpdateCommandStatus(true);
		}
		break;

	default:;
	}

	return cmdHandled;
}

Boolean CSpeechSynthesis::FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName)
{
	if (!Available()) return false;

	outUsesMark = false;
	bool handled = false;

	switch (inCommand)
	{
	case cmd_SpeakSelection:
		// Handle if speaking
		if (Speaking())
		{
			outEnabled = true;
			::PLstrcpy(outName, LStr255(STRx_Speech, str_SpeakStop));
			handled = true;
		}
		break;

	default:;
	}

	return handled;
}

void CSpeechSynthesis::SpeakText(char* txt, long length)
{
	if (!Available())
		return;

	ThrowIfOSErr_(::SpeakText(sSpeechChan, txt, length));
}

void CSpeechSynthesis::StopSpeaking(void)
{
	if (!Available())
		return;

	// Stop actual speech
	ThrowIfOSErr_(::StopSpeech(sSpeechChan));

	// Clear all from queue
	while(!sSpeakQueue.empty())
		sSpeakQueue.pop();
	sTopDone = false;
}

// Speak items from queue (remember that Speech Manager 'captures' text buffer while speaking)
void CSpeechSynthesis::SpendTime(void)
{
	if (!Available())
		return;

	// Ignore if speaking or empty queue
	if (sSpeakQueue.empty() || Speaking())
		return;

	// Pop off top item if done
	if (sTopDone)
	{
		sSpeakQueue.pop();
		sTopDone = false;
	}

	// Check for next item
	if (!sSpeakQueue.empty())
	{
		cdstring& spk = sSpeakQueue.front();
		try
		{
			SpeakText(spk, spk.length());
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Ignore errors
		}
		sTopDone = true;
	}

	// Force command status update to refresh menu item
	LCommander::SetUpdateCommandStatus(true);
}

// Speak specific text item
void CSpeechSynthesis::SpeakString(const cdstring& txt)
{
	if (!Available())
		return;

	cdstring spk = txt;

	// Add end of sentence
	spk += cSpeakEndSentence;

	// Add item to speech queue
	sSpeakQueue.push(spk);

}

void CSpeechSynthesis::SpeakNewMessages(CMbox* mbox)
{
	if (!Available())
		return;
}

void CSpeechSynthesis::SpeakMessage(CMessage* msg, CAttachment* attach, bool letter)
{
	if (!Available())
		return;

	if (!msg)
		return;

	CEnvelope* env = msg->GetEnvelope();
	if (!env)
		return;
	
	cdstring spk;
	const CMessageSpeakVector* items;

	// Adjust for first text part
	if (!attach)
		attach = msg->FirstDisplayPart();

	// Got appropriate item array
	if (letter)
		items = &CPreferences::sPrefs->mSpeakLetterItems.GetValue();
	else
		items = &CPreferences::sPrefs->mSpeakMessageItems.GetValue();

	// Assume text read in
	for(CMessageSpeakVector::const_iterator iter = items->begin(); iter != items->end(); iter++)
	{
		// Add user item to text and add pause after
		spk = (*iter).mItemText;
		spk += cSpeakPause;

		switch((*iter).mItem)
		{
		case eMessageSpeakFrom1:	// Speak single from address
			if (!env->GetFrom()->empty())
				spk += env->GetFrom()->front()->GetNamedAddress();
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakFromAll:	// Speak all from addresses
			if (!env->GetFrom()->empty())
			{
				for(CAddressList::const_iterator iter2 = env->GetFrom()->begin(); iter2 < env->GetFrom()->end(); iter2++)
				{
					spk += (*iter2)->GetNamedAddress();
					spk += cSpeakPause;
				}
			}
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakTo1:
			if (!env->GetTo()->empty())	// Speak single to address
				spk += env->GetTo()->front()->GetNamedAddress();
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakToAll:	// Speak all to addresses
			if (!env->GetTo()->empty())
			{
				for(CAddressList::const_iterator iter2 = env->GetTo()->begin(); iter2 < env->GetTo()->end(); iter2++)
				{
					spk += (*iter2)->GetNamedAddress();
					spk += cSpeakPause;
				}
			}
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakCC1:	// Speak single CC address
			if (!env->GetCC()->empty())
				spk += env->GetCC()->front()->GetNamedAddress();
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakCCAll:	// Speak all CC addresses
			if (!env->GetCC()->empty())
			{
				for(CAddressList::const_iterator iter2 = env->GetCC()->begin(); iter2 < env->GetCC()->end(); iter2++)
				{
					spk += (*iter2)->GetNamedAddress();
					spk += cSpeakPause;
				}
			}
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakDate:	// Speak date
			if (env->GetTextDate(true, true).length())
				spk += env->GetTextDate(true, true);
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakSubject:	// Speak subject
			if (env->GetSubject().length())
				spk += env->GetSubject();
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;

		case eMessageSpeakNumParts:	// Speak number of parts
			spk += cdstring(msg->GetBody()->CountParts());
			break;

		case eMessageSpeakBodyNoHdr:	// Speak message text without header
		{
			const char* body = NULL;
			try
			{
				if (attach)
					body = attach->ReadPart(msg);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}
			if (body && ::strlen(body))
			{
				// Limit size to user specified maximum
				size_t len = ::strlen(body);
				if (len > CPreferences::sPrefs->mSpeakMessageMaxLength.GetValue())
					len = CPreferences::sPrefs->mSpeakMessageMaxLength.GetValue();
				spk += cdstring(body, len);
			}
			else
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			break;
		}

		case eMessageSpeakBodyHdr:	// Speak message text with header
		{
			char* hdr = msg->GetHeader();
			const char* body = NULL;
			try
			{
				if (attach)
					body = attach->ReadPart(msg);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}
			if ((!hdr || !::strlen(hdr)) && (!body || !::strlen(body)))
				spk += CPreferences::sPrefs->mSpeakMessageEmptyItem.GetValue();
			else
			{
				if (hdr)
					spk += cdstring(hdr, ::strlen(hdr));
				spk += cSpeakPause;
				if (body)
				{
					// Limit size to user specified maximum
					size_t len = ::strlen(body);
					if (len > CPreferences::sPrefs->mSpeakMessageMaxLength.GetValue())
						len = CPreferences::sPrefs->mSpeakMessageMaxLength.GetValue();
					spk += cdstring(body, len);
				}
			}
			break;
		}

		case eMessageSpeakNone:	// Do not speak any extra text
		default:
			break;

		}

		// Add end of sentence
		spk += cSpeakEndSentence;

		// Add item to speech queue
		sSpeakQueue.push(spk);
	}
}
