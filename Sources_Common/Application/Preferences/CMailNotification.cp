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


// CMailNotification.cp

// Class to describe a mail notification style

#include "CMailNotification.h"

#include "char_stream.h"
#include "CMailAccountManager.h"
#include "CPreferences.h"
#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;
extern const char* cOSKey;

#pragma mark ____________________________CMailNotification

CMailNotification::CMailNotification()
{
	mEnabled = true;
	mCheckInterval = 5;
	mLastCheck = 0;
	mCheckPending = false;

	mShowAlertForeground = true;
	mShowAlertBackground = false;
	mFlashIcon = false;
	mPlaySound = false;
	mSpeakText = false;
	
	mSpeakCombined = false;
	mSpeakSeparate = false;

	mOpenMbox = false;
	mCheckOnce = false;
	mCheckNew = true;
}

// Copy construct
void CMailNotification::_copy(const CMailNotification& copy)
{
	mName = copy.mName;
	mFavouriteID = copy.mFavouriteID;
	mEnabled = copy.mEnabled;
	mCheckInterval = copy.mCheckInterval;
	mLastCheck = copy.mLastCheck;
	mCheckPending = false;

	mShowAlertForeground = copy.mShowAlertForeground;
	mShowAlertBackground = copy.mShowAlertBackground;
	mFlashIcon = copy.mFlashIcon;
	mPlaySound = copy.mPlaySound;
	mSoundID = copy.mSoundID;
	mSpeakText = copy.mSpeakText;
	mTextToSpeak = copy.mTextToSpeak;

	mSpeakCombined = copy.mSpeakCombined;
	mSpeakSeparate = copy.mSpeakSeparate;

	mOpenMbox = copy.mOpenMbox;
	mCheckOnce = copy.mCheckOnce;
	mCheckNew = copy.mCheckNew;
	
	mFuture = copy.mFuture;
}

// Compare with same type
int CMailNotification::operator==(const CMailNotification& test) const
{
	return (mName == test.mName) &&
			(mFavouriteID == test.mFavouriteID) &&
			(mEnabled == test.mEnabled) &&
			(mCheckInterval == test.mCheckInterval) &&
			(mShowAlertForeground == test.mShowAlertForeground) &&
			(mShowAlertBackground == test.mShowAlertBackground) &&
			(mFlashIcon == test.mFlashIcon) &&
			(mPlaySound == test.mPlaySound) &&
			(mSoundID == test.mSoundID) &&
			(mSpeakText == test.mSpeakText) &&
			(mTextToSpeak == test.mTextToSpeak) &&
			(mSpeakCombined == test.mSpeakCombined) &&
			(mSpeakSeparate == test.mSpeakSeparate) &&
			(mOpenMbox == test.mOpenMbox) &&
			(mCheckOnce == test.mCheckOnce) &&
			(mCheckNew == test.mCheckNew);
}

// Get index into preferences favourites
unsigned long CMailNotification::GetFavouriteIndex(const CPreferences* prefs) const
{
	// NB DO NOT user sMailAccountManager here as it may not be setup
	
	cdstring name = GetFavouriteID().c_str() + 1;
	bool user = (*GetFavouriteID().c_str() == '@');
	unsigned long index = 0;
	if (user)
	{
		// Scan user items for name match
		index = CMailAccountManager::eFavouriteOthers;
		bool found = false;
		for(CFavouriteItemList::const_iterator iter = prefs->mFavourites.GetValue().begin() + CMailAccountManager::eFavouriteOthers;
				iter != prefs->mFavourites.GetValue().end(); iter++, index++)
		{
			if ((*iter).GetName() == name)
			{
				found = true;
				break;
			}
		}
		if (!found)
			index = 0;
	}
	else
		index = ::atoi(name);
	
	return index;
}

const cdstring& CMailNotification::GetSoundID(void) const
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

void CMailNotification::SetSoundID(const cdstring& sound_id)
{
	// Try insert or replace
	std::pair<cdstrmap::iterator, bool> found = mSoundID.insert(cdstrmap::value_type(cOSKey, sound_id));
	if (!found.second)
		(*found.first).second = sound_id;
}

void CMailNotification::SetTextToSpeak(const cdstring& text_to_speak)
{
	mTextToSpeak = text_to_speak;
	if (!mTextToSpeak.empty())
	{
		mSpeakCombined = ::strstr(mTextToSpeak.c_str(), "##") || ::strstr(mTextToSpeak.c_str(), "**");
		mSpeakSeparate = !((!::strchr(mTextToSpeak.c_str(), '#') && !::strchr(mTextToSpeak.c_str(), '*')) || mSpeakCombined);
	}
	else
	{
		mSpeakCombined = false;
		mSpeakSeparate = false;
	}
}

// Parse S-Expression element
bool CMailNotification::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = 1;

	txt.get(mName, true);
	// >= v2.0.0a4
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_4) >= 0)
		txt.get(mFavouriteID, true);
	txt.get(mEnabled);
	txt.get(mCheckInterval);
	txt.get(mShowAlertForeground);
	txt.get(mShowAlertBackground);
	txt.get(mFlashIcon);
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
	{
		// Set it by hand to ensure cached values get set
		cdstring temp;
		txt.get(temp, true);
		SetTextToSpeak(temp);
	}
	txt.get(mOpenMbox);
	txt.get(mCheckOnce);
	// >= v2.0.0a4
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_4) >= 0)
		txt.get(mCheckNew);

	mFuture.SetInfo(txt, vers_prefs);

	return result;
}

// Create S_Expression element
cdstring CMailNotification::GetInfo(void) const
{
	cdstring all;
	cdstring temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	temp = mFavouriteID;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += (mEnabled ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	temp = mCheckInterval;
	all += temp;
	all += cSpace;

	all += (mShowAlertForeground ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mShowAlertBackground ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mFlashIcon ? cValueBoolTrue : cValueBoolFalse);
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

	temp = mTextToSpeak;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += (mOpenMbox ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mCheckOnce ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mCheckNew ? cValueBoolTrue : cValueBoolFalse);

	all += mFuture.GetInfo();

	return all;
}
