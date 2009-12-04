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


#include "CClickElement.h"
#include "CMulberryApp.h"
#include "cdstring.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CStringUtils.h"
#include "CURL.h"

#if 0
#include <UProfiler.h>
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#include "CFormattedTextDisplay.h"
#elif __dest_os == __win32_os
#include "CDisplayFormatter.h"
#endif

#pragma mark ____________________________CClickElement

CClickElement::CClickElement()
{
	mStart = -1;
	mStop = -1;
	next = NULL;
}

CClickElement::CClickElement(int start, int stop)
{
	mStart = start;
	mStop = stop;
	next = NULL;
}

bool CClickElement::findCursor(int offset) const
{
	return (offset >= mStart && offset <= mStop);
}

bool CClickElement::IsAnchor() const
{
	return false;
}

#pragma mark ____________________________CURLClickElement

CURLClickElement::CURLClickElement()
{
}

CURLClickElement::CURLClickElement(int start, int stop, char *s, char *data) : CClickElement(start, stop)
{
	mScheme.steal(s);
	mAddress.steal(data);
}

const char cMailtoURL_StartExtended = '?';
const char cMailtoURL_NextExtended = '&';
const char cMailtoURL_FieldEnd = '=';

void CURLClickElement::ParseMailto(const char *address, cdstring& myTo, cdstring& myCc, cdstring& myBcc,
									cdstring& mySubject, cdstring& myBody, cdstrvect& files)
{
	// Do not process empty URL data
	if (!address || !*address)
		return;

	cdstring part;

	while(*address && (*address != cMailtoURL_StartExtended))
		myTo += *address++;

	if (*address) address++;

	while(*address)
	{
		part = cdstring::null_str;
		while(*address && (*address != cMailtoURL_FieldEnd))
			part += *address++;

		if (*address) address++;

		if(strcmpnocase(part.c_str(), "to") == 0)
		{
			if (!myTo.empty())
				myTo += ", ";
			while(*address && (*address != cMailtoURL_NextExtended))
				myTo += *address++;
		}
		else if(strcmpnocase(part.c_str(), "cc") == 0)
		{
			if (!myCc.empty())
				myCc += ", ";
			while(*address && (*address != cMailtoURL_NextExtended))
				myCc += *address++;
		}
		else if(strcmpnocase(part.c_str(), "bcc") == 0)
		{
			if (!myBcc.empty())
				myBcc += ", ";
			while(*address && (*address != cMailtoURL_NextExtended))
				myBcc += *address++;
		}
		else if(strcmpnocase(part.c_str(), "body") == 0)
		{
			while(*address && (*address != cMailtoURL_NextExtended))
				myBody += *address++;
		}
		else if(strcmpnocase(part.c_str(), "subject") == 0)
		{
			while(*address && (*address != cMailtoURL_NextExtended))
				mySubject += *address++;
		}
		else if(strcmpnocase(part.c_str(), "x-mulberry-file") == 0)
		{
			cdstring temp;
			while(*address && (*address != cMailtoURL_NextExtended))
				temp += *address++;
			if (!temp.empty())
			{
				temp.DecodeURL();
				files.push_back(temp);
			}
		}
		else
		{
			while(*address && (*address != cMailtoURL_NextExtended))
				address++;
		}
		

		if (*address) address++;
	}

	// Decode escapes from URLs
	myTo.DecodeURL();
	CRFC822::TextFrom1522(myTo);
	myCc.DecodeURL();
	CRFC822::TextFrom1522(myCc);
	myBcc.DecodeURL();
	CRFC822::TextFrom1522(myBcc);
	myBody.DecodeURL();
	mySubject.DecodeURL();
	CRFC822::TextFrom1522(mySubject);
}

cdstring CURLClickElement::GetDescriptor() const
{
	return mScheme + mAddress;
}

bool CURLClickElement::Act(CDisplayFormatter *display)
{
	if (IsAnchor())
		return display->DoAnchor(this);
	else if (mScheme.compare(cMailtoURLScheme, true) == 0)
	{
		CMulberryApp::ProcessMailto(mAddress);
		return true;
	}
	else if ((mScheme.compare("webcal:", true) == 0) && (CPreferences::sPrefs->mHandleICS.GetValue()))
	{
		CMulberryApp::ProcessWebcal(mAddress);
		return true;
	}
	else
	{
		cdstring st = mScheme + mAddress;

		return display->LaunchURL(st);
	}
}

bool CURLClickElement::IsAnchor() const
{
	return mScheme.empty() && (mAddress[cdstring::size_type(0)] == '#');
}

#pragma mark ____________________________CAnchorClickElement

CAnchorClickElement::CAnchorClickElement()
{
}

CAnchorClickElement::CAnchorClickElement(int start, int stop, char *data) : CClickElement(start, stop)
{
	mName.steal(data);
}

cdstring CAnchorClickElement::GetDescriptor() const
{
	return mName;
}

bool CAnchorClickElement::Act(CDisplayFormatter *display)
{
	return true;
}
