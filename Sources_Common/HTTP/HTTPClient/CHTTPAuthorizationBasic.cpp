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
	CHTTPAuthorizationBasic.cpp

	Author:			
	Description:	<describe the CHTTPAuthorizationBasic class here>
*/

#include "CHTTPAuthorizationBasic.h"

#include "CHTTPDefinitions.h"

#include "base64.h"

#include <memory>

using namespace http; 

void CHTTPAuthorizationBasic::GenerateAuthorization(std::ostream& os, const CHTTPRequestResponse* request) const
{
	// Generate the string to base 64 encode
	cdstring encode(mUser);
	encode += ":";
	encode += mPswd;
	
	// Base64 encode it
	std::auto_ptr<char> base64(::base64_encode(reinterpret_cast<const unsigned char*>(encode.c_str()), encode.length()));

	// Generate header
	os << cHeaderAuthorization << cHeaderDelimiter << "Basic " << base64.get() << net_endl;
}
