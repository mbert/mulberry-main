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

/*
	CCalendarStoreWebcal.h

	Author:
	Description:	maintains state for a publish/subscribe webcal
*/

#ifndef CCalendarStoreWebcal_H
#define CCalendarStoreWebcal_H

#include "cdstring.h"


namespace xmllib 
{
class XMLDocument;
class XMLNode;
};

namespace calstore {

class CCalendarStoreWebcal
{
public:
	CCalendarStoreWebcal(const cdstring& url = cdstring::null_str)
	{
		mURL = url;
		mPeriodicRefresh = false;
		mRefreshInterval = 30;		// 30 minutes
		mAutoPublish = false;
		mReadOnly = false;
	}
	CCalendarStoreWebcal(const CCalendarStoreWebcal& copy)
	{
		_copy_CCalendarStoreWebcal(copy);
	}
	virtual ~CCalendarStoreWebcal() {}

	CCalendarStoreWebcal& operator=(const CCalendarStoreWebcal& copy)
	{
		if (this != &copy)
		{
			_copy_CCalendarStoreWebcal(copy);
		}
		return *this;
	}
	
	const cdstring& GetURL() const
	{
		return mURL;
	}
	void SetURL(const cdstring& url)
	{
		mURL = url;
	}

	bool GetPeriodicRefresh() const
	{
		return mPeriodicRefresh;
	}
	void SetPeriodicRefresh(bool refresh)
	{
		mPeriodicRefresh = refresh;
	}

	uint32_t GetRefreshInterval() const
	{
		return mRefreshInterval;
	}
	void SetRefreshInterval(uint32_t interval)
	{
		mRefreshInterval = interval;
	}

	bool GetAutoPublish() const
	{
		return mAutoPublish;
	}
	void SetAutoPublish(bool publish)
	{
		mAutoPublish = publish;
	}

	bool GetReadOnly() const
	{
		return mReadOnly;
	}
	void SetReadOnly(bool readonly)
	{
		mReadOnly = readonly;
	}

	void WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent) const;
	void ReadXML(const xmllib::XMLNode* node);

protected:
	cdstring					mURL;
	bool						mPeriodicRefresh;
	uint32_t					mRefreshInterval;
	bool						mAutoPublish;
	bool						mReadOnly;

private:
	void	_copy_CCalendarStoreWebcal(const CCalendarStoreWebcal& copy)
	{
		mURL = copy.mURL; mPeriodicRefresh = copy.mPeriodicRefresh; mRefreshInterval = copy.mRefreshInterval; mAutoPublish = copy.mAutoPublish; mReadOnly = copy.mReadOnly;
	}
};

}	// namespace iCal

#endif	// CCalendarStoreWebcal_H
