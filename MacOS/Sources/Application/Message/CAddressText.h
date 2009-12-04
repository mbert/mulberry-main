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


// Header for CAddressText class

#ifndef __CADDRESSTEXT__MULBERRY__
#define __CADDRESSTEXT__MULBERRY__

#include "CTextDisplay.h"

// Classes

class CAddressText : public CTextDisplay
{
public:
	enum { class_ID = 'AdTx' };

					CAddressText(LStream *inStream);
	virtual 		~CAddressText();

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
protected:
	virtual void		FinishCreateSelf();

private:
			void	CaptureAddress();
};

#endif
