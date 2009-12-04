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


// CDSN : class to implement a favourite item

#include "CDSN.h"

#include "char_stream.h"
#include "CStringUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

const char* cSMTPNotify = "NOTIFY=";
unsigned long cSMTPNotifyLen = 7;
const char* cSMTPNotifySuccess = "SUCCESS";
const char* cSMTPNotifyFailure = "FAILURE";
const char* cSMTPNotifyDelay = "DELAY";
const char* cSMTPNotifyNever = "NEVER";
const char* cSMTPReturn = "RETURN=";
unsigned long cSMTPReturnLen = 7;
const char* cSMTPReturnFull = "FULL";
const char* cSMTPReturnHeaders = "HDRS";

#pragma mark ____________________________CDSN

// Constructor
CDSN::CDSN()
{
	mMDN = false;
	mRequest = false;
	mSuccess = false;
	mFailure = true;
	mDelay = false;
	mFull = false;
}

int CDSN::operator==(const CDSN& comp) const
{
	return (mMDN == comp.mMDN) &&
			(mRequest == comp.mRequest) &&
			(mSuccess == comp.mSuccess) &&
			(mFailure == comp.mFailure) &&
			(mDelay == comp.mDelay) &&
			(mFull == comp.mFull);
}

// Assignment with same type
void CDSN::_copy(const CDSN& copy)
{
	mMDN = copy.mMDN;
	mRequest = copy.mRequest;
	mSuccess = copy.mSuccess;
	mFailure = copy.mFailure;
	mDelay = copy.mDelay;
	mFull = copy.mFull;
}

void CDSN::WriteHeaderToStream(std::ostream& out) const
{
	out << cSMTPNotify;
	if (mSuccess || mFailure || mDelay)
	{
		bool done = false;
		if (mSuccess)
		{
			out << cSMTPNotifySuccess;
			done = true;
		}
		if (mFailure)
		{
			if (done)
				out << ",";
			out << cSMTPNotifyFailure;
			done = true;
		}
		if (mDelay)
		{
			if (done)
				out << ",";
			out << cSMTPNotifyDelay;
			done = true;
		}
	}
	else
		out << cSMTPNotifyNever;

	out << " " << cSMTPReturn;
	if (mFull)
		out << cSMTPReturnFull;
	else
		out << cSMTPReturnHeaders;
}

void CDSN::ReadHeader(char* txt)
{
	mSuccess = false;
	mFailure = false;
	mDelay = false;
	char* p = txt;
	while(*p == ' ') p++;
	if (*p && (::strncmpnocase(p, cSMTPNotify, cSMTPNotifyLen) == 0))
	{
		p += cSMTPNotifyLen;
		while(true)
		{
			char* token = p;
			while(*p && (*p != ',') && (*p != ' ')) p++;
			bool done = (!*p || (*p == ' '));
			if (*p) *p++ = 0;

			if (::strcmpnocase(token, cSMTPNotifySuccess) == 0)
				mSuccess = true;
			else if (::strcmpnocase(token, cSMTPNotifyFailure) == 0)
				mFailure = true;
			else if (::strcmpnocase(token, cSMTPNotifyDelay) == 0)
				mDelay = true;
			else if (::strcmpnocase(token, cSMTPNotifyNever) == 0)
			{
				mSuccess = false;
				mFailure = false;
				mDelay = false;
			}

			if (done) break;
		}
	}
	if (*p && (::strncmpnocase(p, cSMTPReturn, cSMTPReturnLen) == 0))
	{
		p += cSMTPReturnLen;
		char* token = p;
		while(*p && (*p != ' ')) p++;
		if (*p) *p = 0;
		if (::strcmpnocase(token, cSMTPReturnFull) == 0)
			mFull = true;
		else if (::strcmpnocase(token, cSMTPReturnHeaders) == 0)
			mFull = false;
	}

}

// Write prefs
cdstring CDSN::GetInfo(void) const
{
	cdstring all;
	all += (mRequest ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mSuccess ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mFailure ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mDelay ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mFull ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	return all;
}

// Read prefs
bool CDSN::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mRequest);
	txt.get(mSuccess);
	txt.get(mFailure);
	txt.get(mDelay);
	txt.get(mFull);

	return true;
}
