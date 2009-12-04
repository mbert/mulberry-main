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


// Header for CTextField class

#ifndef __CADDRESSDISPLAY__MULBERRY__
#define __CADDRESSDISPLAY__MULBERRY__

#include "CAddressText.h"

// Classes

class LDisclosureTriangle;
class CAddressList;

class CAddressDisplay : public CAddressText
{
public:
	enum { class_ID = 'AdDi' };

					CAddressDisplay(LStream *inStream);
	virtual 		~CAddressDisplay();

	virtual void	SetTwister(LDisclosureTriangle* twister)
						{ mTwister = twister; }

	virtual CAddressList*	GetAddresses(bool qualify = true);

	static void		AllowResolve(bool allow)
						{ sResolve = allow; }
						
public:
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual Boolean	HandleKeyPress(const EventRecord& inKeyEvent);	// Handle keys our way

	virtual void		DontBeTarget();

protected:
	virtual void	DoDragReceive(DragReference	inDragRef);				// Get multiple text items

private:
	LDisclosureTriangle*		mTwister;
	bool						mResolving;
	static bool	sResolve;

			void	ResolveAddresses(bool qualify = true);
			void	ExpandAddress();
			bool	ExpandAddressText(cdstring& expand, cdstrvect& results);
};

#endif
