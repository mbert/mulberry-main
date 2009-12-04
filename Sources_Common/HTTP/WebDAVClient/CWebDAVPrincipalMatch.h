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
	CWebDAVPrincipalMatch.h

	Author:			
	Description:	<describe the CWebDAVPrincipalMatch class here>
*/

#ifndef CWebDAVPrincipalMatch_H
#define CWebDAVPrincipalMatch_H

#include "CWebDAVPropFindBase.h"

#include "XMLName.h"

#include <vector>


using namespace http; 

namespace http {

namespace webdav {

class CWebDAVPrincipalMatch: public CWebDAVPropFindBase
{
public:
	CWebDAVPrincipalMatch(CWebDAVSession* session, const cdstring& ruri, const xmllib::XMLNameList& props);
	virtual ~CWebDAVPrincipalMatch();

protected:
	xmllib::XMLNameList	mProps;

	void	InitRequestData();
	
	void	GenerateXML(std::ostream& os);
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVPrincipalMatch_H
