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
	CWebDAVMultiResponseParser.cpp

	Author:			
	Description:	<describe the CWebDAVMultiResponseParser class here>
*/

#include "CWebDAVMultiResponseParser.h"

#include "CHTTPUtils.h"
#include "CWebDAVDefinitions.h"
#include "XMLNode.h"

using namespace http; 
using namespace webdav; 
using namespace xmllib; 

void CWebDAVMultiResponseParser::Parse(const XMLNode* multistatus_node)
{
	// Must have a node
	if (multistatus_node == NULL)
		return;
	
	// Verify that the node is the correct element <DAV:multistatus>
	if (!multistatus_node->CompareFullName(cElement_multistatus))
		return;
	
	// Node is the right type, so iterator over all child response nodes and process each one
	for(XMLNodeList::const_iterator iter = multistatus_node->Children().begin(); iter != multistatus_node->Children().end(); iter++)
		ParseResponse(*iter);
}
