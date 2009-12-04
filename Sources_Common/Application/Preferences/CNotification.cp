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


// CNotification.cp

// Class to describe a mail notification style

#include "CNotification.h"

#include "CErrorHandler.h"
#include "CMulberryCommon.h"
#include "CPreferenceVersions.h"
#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif
#include "CUtils.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;
extern const char* cOSKey;

#pragma mark ____________________________CNotification

CNotification::CNotification()
{
	mShowAlert = true;
	mPlaySound = false;
	mSpeakText = false;
}

// Copy construct
void CNotification::_copy(const CNotification& copy)
{
	mShowAlert = copy.mShowAlert;
	mPlaySound = copy.mPlaySound;
	mSoundID = copy.mSoundID;
	mSpeakText = copy.mSpeakText;
	mTextToSpeak = copy.mTextToSpeak;
	mFuture = copy.mFuture;
}

// Compare with same type
int CNotification::operator==(const CNotification& test) const
{
	return (mShowAlert == test.mShowAlert) &&
			(mPlaySound == test.mPlaySound) &&
			(mSoundID == test.mSoundID) &&
			(mSpeakText == test.mSpeakText) &&
			(mTextToSpeak == test.mTextToSpeak);
}

void CNotification::DoNotification(const char* rsrcid) const
{
	// Do attachment notification
	if (DoPlaySound())
		::PlayNamedSound(GetSoundID());
#ifdef __use_speech
	if (DoSpeakText())
		CSpeechSynthesis::SpeakString(GetTextToSpeak());
#endif
	if (DoShowAlert())
		CErrorHandler::PutNoteAlertRsrc(rsrcid, !DoPlaySound());
}

const cdstring& CNotification::GetSoundID(void) const
{
	// Look for OS key
	cdstrmap::const_iterator found = mSoundID.find(cOSKey);
	if (found == mSoundID.end())
	{
		std::pair<cdstrmap::iterator, bool> result = const_cast<cdstrmap&>(mSoundID).insert(cdstrmap::value_type(cOSKey, cdstring::null_str));
		found = result.first;
	}

	return (*found).second;
}

void CNotification::SetSoundID(const cdstring& sound_id)
{
	// Try insert or replace
	std::pair<cdstrmap::iterator, bool> found = mSoundID.insert(cdstrmap::value_type(cOSKey, sound_id));
	if (!found.second)
		(*found.first).second = sound_id;
}


// Parse S-Expression element
bool CNotification::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	txt.get(mShowAlert);
	txt.get(mPlaySound);
	// >= v2.0.0a4
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_4) >= 0)
		cdstring::ParseSExpression(txt, mSoundID, true);
	else
	{
		// Read in single string value and insert in OS-string map
		cdstring temp;
		txt.get(temp);
		SetSoundID(temp);
	}
	txt.get(mSpeakText);
	txt.get(mTextToSpeak, true);

	mFuture.SetInfo(txt, vers_prefs);

	return result;
}

// Create S_Expression element
cdstring CNotification::GetInfo(void) const
{
	cdstring all = (mShowAlert ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mPlaySound ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring sexpression;
	sexpression.CreateSExpression(mSoundID);
	sexpression.ConvertFromOS();
	all += sexpression;
	all += cSpace;

	all += (mSpeakText ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring item = mTextToSpeak;
	item.quote();
	item.ConvertFromOS();
	all += item;

	all += mFuture.GetInfo();

	return all;
}
