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
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

// === Static Members ===

CComPtr<ISpVoice> CSpeechSynthesis::sSpVoice;
bool CSpeechSynthesis::sAvailable = false;
#ifdef __MULBERRY
cdmutex CSpeechSynthesis::_mutex;
#endif
cdstrqueue CSpeechSynthesis::sSpeakQueue;
bool CSpeechSynthesis::sTopDone = false;
cdstrvect CSpeechSynthesis::sItemList;

// === constants ===

const char* cSpeakPause = ", ";
const char* cSpeakEndSentence = ". ";

#pragma ARM_conform on

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
	CMenu* speak_menu = NULL;

	// Load COM
	CoInitialize(NULL);

	try
	{
		//  First check to see if the Speech Manager extension is present
		HRESULT result = sSpVoice.CoCreateInstance(CLSID_SpVoice);

		// Check state
		if (SUCCEEDED(result))
		{
			// Get descriptor strings
			for(short i = IDS_SPEAK_FROM1; i <= IDS_SPEAK_NONE; i++)
			{
				cdstring str;
				str.FromResource(i);
				sItemList.push_back(str);
			}

			sAvailable = true;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Not loaded - do nothing
		sAvailable = false;
	}

	return sAvailable;
}

// Remove speech
void CSpeechSynthesis::RemoveSpeechSynthesis()
{
	// Explicitly release COM object here rather than rely on static destructor to do it
	
	// Release COM object
	sSpVoice.Release();

	// Unload COM
	CoUninitialize();
}

// Create menu
void CSpeechSynthesis::InstallMenu(CMenu* menu)
{
}

bool CSpeechSynthesis::OnUpdateEditSpeak(UINT nID, CCmdUI* pCmdUI)
{
	bool handled = false;
	
	if (!Available())
	{
		pCmdUI->Enable(false);
		return false;
	}

#ifdef __MULBERRY
	switch(nID)
	{
	case IDM_EDIT_SPEAK:
		if (Speaking())
		{
			CString txt;
			txt.LoadString(IDS_SPEAK_STOPSPEAKING);
			OnUpdateMenuTitle(pCmdUI, txt);
			pCmdUI->Enable(true);	// Always
			handled = true;
		}
		break;
	default:;
	}	
#endif
	return handled;
}

bool CSpeechSynthesis::OnEditSpeak(UINT nID)
{
	bool handled = false;
	
#ifdef __MULBERRY
	switch(nID)
	{
	case IDM_EDIT_SPEAK:
		if (Speaking())
		{
			StopSpeaking();
			handled = true;
		}
		break;
	default:;
	}	
#endif
	return handled;
}

bool CSpeechSynthesis::Speaking(void)
{
	if (!Available())
		return false;

	SPVOICESTATUS status;
	HRESULT result = sSpVoice->GetStatus(&status, NULL);
	if (SUCCEEDED(result))
		return status.dwRunningState == SPRS_IS_SPEAKING;
	else
		return false;
}

void CSpeechSynthesis::SpeakText(char* txt, long length)
{
	if (!Available())
		return;

	auto_ptr<wchar_t> wtxt(new wchar_t[::strlen(txt) + 1]);
	const char* p = txt;
	wchar_t* q = wtxt.get();
	while(*p)
		*q++ = *p++;
	*q++ = *p++;
	sSpVoice->Speak(wtxt.get(), SPF_ASYNC | SPF_IS_NOT_XML, 0);
}

void CSpeechSynthesis::StopSpeaking(void)
{
	if (!Available())
		return;

	// Stop actual speech
	sSpVoice->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);

	// Protect queue against multi-thread access
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(_mutex);
#endif

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

	// Protect queue against multi-thread access
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(_mutex);
#endif

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
		SpeakText(spk, spk.length());
		sTopDone = true;
	}
}

// Speak specific text item
void CSpeechSynthesis::SpeakString(const cdstring& txt)
{
	if (!Available())
		return;

	cdstring spk = txt;

	// Add end of sentence
	spk += cSpeakEndSentence;

	// Protect queue against multi-thread access
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(_mutex);
#endif

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
			const char* body = nil;
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
			const char* body = nil;
			char* hdr = msg->GetHeader();
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

		// Protect queue against multi-thread access
#ifdef __MULBERRY
		cdmutex::lock_cdmutex _lock(_mutex);
#endif

		// Add item to speech queue
		sSpeakQueue.push(spk);
	}
}
