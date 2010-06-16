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
 CWebDAVSyncReportParser.h
 
 Author:			
 Description:	<describe the CWebDAVSyncReportParser class here>
 */

#ifndef CWebDAVSyncReportParser_H
#define CWebDAVSyncReportParser_H

#include "CWebDAVMultiResponseParser.h"

#include "cdstring.h"

using namespace http; 
using namespace webdav; 

namespace http {
	
	namespace webdav {
		
		class CWebDAVSyncReportParser: public CWebDAVMultiResponseParser
		{
		public:
			CWebDAVSyncReportParser(cdstrmap& changed, cdstrset& removed, cdstring& sync_token);
			virtual ~CWebDAVSyncReportParser();
			
			
		protected:
			cdstrmap&	mChanged;
			cdstrset&	mRemoved;
			cdstring&	mSyncToken;
			
			// Parse the response element down to the properties
			
			virtual void ParseResponse(const xmllib::XMLNode* response);
			virtual void ParseStatus(const xmllib::XMLNode* response, const cdstring& href);
			virtual void ParsePropStat(const xmllib::XMLNode* response, const cdstring& href);
			virtual void ParseProp(const xmllib::XMLNode* response, const cdstring& href);
			
			// Parsing of property elements
			virtual void ParsePropElement(const xmllib::XMLNode* response, const cdstring& href);
			
		};
		
	}	// namespace webdav
	
}	// namespace http

#endif	// CWebDAVSyncReportParser_H
