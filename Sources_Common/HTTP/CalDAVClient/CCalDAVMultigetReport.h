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
	CCalDAVMultigetReport.h

	Author:			
	Description:	<describe the CCalDAVMultigetReport class here>
*/

#ifndef CCalDAVMultigetReport_H
#define CCalDAVMultigetReport_H

#include "CWebDAVReport.h"

using namespace http; 
using namespace webdav; 

namespace http {

namespace caldav {

class CCalDAVMultigetReport: public CWebDAVReport
{
public:
	CCalDAVMultigetReport(CWebDAVSession* session, const cdstring& ruri, const cdstrvect& hrefs, bool old_style = false);
	virtual ~CCalDAVMultigetReport();

protected:
	bool	mOldStyle;

	void	InitRequestData(const cdstrvect& hrefs);
	
	void	GenerateXML(std::ostream& os, const cdstrvect& hrefs);
};

}	// namespace caldav

}	// namespace http

#endif	// CCalDAVMultigetReport_H
