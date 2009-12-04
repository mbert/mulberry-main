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


// CFutureItems : class to implement a favourite item

#include "CFutureItems.h"

#include "char_stream.h"

extern const char* cSpace;

#pragma mark ____________________________CFutureItems

// Write items
cdstring CFutureItems::GetInfo() const
{
	cdstring all;
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		cdstring temp(*iter);
		temp.quote();
		all += cSpace;	// Always add space as this is being added to other items
		all += temp;
	}

	return all;
}

// Read items
bool CFutureItems::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Empty first
	clear();

	while(!txt.test_end_sexpression())
	{
		cdstring temp;
		txt.get(temp);
		push_back(temp);
	}
	return true;
}
