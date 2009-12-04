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


// Header for tree node abstract base class

#include "CTreeNode.h"

// Copy construct
CTreeNode::CTreeNode(const CTreeNode& copy)
{
	mName = copy.mName;
	mWDLevel = copy.mWDLevel;
	mDirDelim = copy.mDirDelim;
}

// Assignment with same type
CTreeNode& CTreeNode::operator=(const CTreeNode& copy)
{
	if (this != &copy)
	{
		mName = copy.mName;
		mWDLevel = copy.mWDLevel;
		mDirDelim = copy.mDirDelim;
	}

	return *this;
}

// Set pointer to short name
void CTreeNode::SetShortName()
{
	// Determine last directory break
	const char* p = nil;
	if (mDirDelim && ((p = ::strrchr(mName.c_str(), mDirDelim)) != nil))
		mShortName = ++p;
	else
		mShortName = mName.c_str();

} // CTreeNode::SetShortName
