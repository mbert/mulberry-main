/*
 Copyright (c) 2010 Cyrus Daboo. All rights reserved.
 
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

#ifndef CWebDAVSyncReport_H
#define CWebDAVSyncReport_H

#include "CWebDAVReport.h"

using namespace http; 
using namespace webdav; 

namespace http {
	
namespace webdav {
	
	class CWebDAVSyncReport: public CWebDAVReport
	{
	public:
		CWebDAVSyncReport(CWebDAVSession* session, const cdstring& ruri, const cdstring& sync_token);
		virtual ~CWebDAVSyncReport();
		
	protected:		
		void	InitRequestData(const cdstring& sync_token);

		virtual void WriteHeaderToStream(std::ostream& os);
		
		void	GenerateXML(std::ostream& os, const cdstring& sync_token);
	};
	
}	// namespace webdav

}	// namespace http

#endif	// CWebDAVSyncReport_H
