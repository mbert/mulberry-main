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


// CXKeyScrollAttachment

// Extended key scroll attachment that allows opt/cmd-Arrow key combinations as navigators

#include "CXKeyScrollAttachment.h"

#include <LView.h>

#include <PP_KeyCodes.h>

CXKeyScrollAttachment::CXKeyScrollAttachment(LView *inViewToScroll)
		: LKeyScrollAttachment(inViewToScroll)
{
}


CXKeyScrollAttachment::CXKeyScrollAttachment(LStream *inStream)
		: LKeyScrollAttachment(inStream)
{
}


void CXKeyScrollAttachment::ExecuteSelf(MessageT inMessage, void* ioParam)
{
	mExecuteHost = false;		// We handle navigation keys
	Int16	theKey = ((EventRecord*) ioParam)->message & charCodeMask;
	bool optKeyDown = ((EventRecord*) ioParam)->modifiers & optionKey;
	bool cmdKeyDown = ((EventRecord*) ioParam)->modifiers & cmdKey;
	
	// Modify ioParam with extended keyboard equivs
	switch (theKey)
	{
		case char_UpArrow:
			if (optKeyDown && !cmdKeyDown)
				((EventRecord*) ioParam)->message = char_PageUp;
			else if (!optKeyDown && cmdKeyDown)
				((EventRecord*) ioParam)->message = char_Home;
			break;
			
		case char_DownArrow:
			if (optKeyDown && !cmdKeyDown)
				((EventRecord*) ioParam)->message = char_PageDown;
			else if (!optKeyDown && cmdKeyDown)
				((EventRecord*) ioParam)->message = char_End;
			break;
		default:
			break;
	}

	// Do inherited
	LKeyScrollAttachment::ExecuteSelf(inMessage, ioParam);
}
