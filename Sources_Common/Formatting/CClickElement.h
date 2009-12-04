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


#ifndef __CCLICKELEMENT__MULBERRY__
#define __CCLICKELEMENT__MULBERRY__

#include <map>

#include "cdstring.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#define CDisplayFormatter CFormattedTextDisplay
#endif
class CDisplayFormatter;

class CClickElement
{
public:
	CClickElement();
	CClickElement(int start, int stop);
	virtual ~CClickElement() {}

	void setStart(int start)
		{ mStart = start; }
	int getStart() const
		{ return mStart; }
	void setStop(int stop)
		{ mStop = stop; }
	int getStop() const
		{ return mStop; }

	virtual cdstring GetDescriptor() const = 0;

	CClickElement *next;

	bool findCursor(int offset) const;
	virtual bool Act(CDisplayFormatter* display) = 0;
	virtual bool IsAnchor() const;

protected:
	int mStart;
	int mStop;

private:
	void _copy(const CClickElement& copy)
		{ mStart = copy.mStart; mStop = copy.mStop; }
};

class CURLClickElement : public CClickElement
{
public:
	CURLClickElement();
	CURLClickElement(int start, int stop, char* scheme, char* data);

	virtual ~CURLClickElement() {}

	static void ParseMailto(const char* address, cdstring& to, cdstring& cc, cdstring& bcc,
							cdstring& subject, cdstring& body, cdstrvect& files);

	virtual cdstring GetDescriptor() const;
	virtual bool Act(CDisplayFormatter* display);
	virtual bool IsAnchor() const;

private:
	cdstring mScheme;
	cdstring mAddress;
};

class CAnchorClickElement : public CClickElement
{
public:
	CAnchorClickElement();
	CAnchorClickElement(int start, int stop, char* data);
	CAnchorClickElement(const CAnchorClickElement& copy)
		{ _copy(copy); }

	virtual ~CAnchorClickElement() {}

	CAnchorClickElement& operator=(const CAnchorClickElement& copy)
		{ if (this != &copy) _copy(copy); return *this; }


	virtual cdstring GetDescriptor() const;
	virtual bool Act(CDisplayFormatter* display);

private:
	cdstring mName;

	void _copy(const CAnchorClickElement& copy)
		{ mStart = copy.mStart; mStop = copy.mStop; mName = copy.mName; }
};

typedef std::map<cdstring, CAnchorClickElement> CAnchorElementMap;

#endif
