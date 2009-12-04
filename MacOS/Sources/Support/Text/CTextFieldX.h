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


// Header for CTextFieldX class

#ifndef __CTEXTFIELDX__MULBERRY__
#define __CTEXTFIELDX__MULBERRY__

#include "CTextBase.h"
#include <LBroadcaster.h>

// Constants
//const MessageT msg_TextFieldChanged = 950;

// Classes
class CTextFieldX : public CTextBase,
					public LBroadcaster
{

public:
	enum { class_ID = 'TXFi' };


					CTextFieldX(LStream *inStream);
	virtual 		~CTextFieldX();
	
	void	SetBroadcastReturn(bool broadcast)
		{ mBroadcastReturn = broadcast; }

	virtual void		CalcRevealedRect();
	
	bool				CalcPortStructureRect( Rect& outRect ) const;
	
	virtual Boolean		CalcPortExposedRect(
								Rect&		outRect,
								bool		inOKIfHidden = false) const;

	virtual Boolean	HandleKeyPress(const EventRecord &inKeyEvent);	// Handle non-editable case
	virtual	Boolean ObeyCommand(CommandT inCommand,
								void* ioParam);							// Handle commands our way

			void	SetNumber(long num);						// Number in
			long	GetNumber() const;							// Number out

	virtual void	SetTextTraits(const TextTraitsRecord& aTextTrait);	// Set text traits for general text

protected:
	MessageT		mValueMessage;
	bool			mBroadcastAll;
	bool			mBroadcastReturn;
	bool			mNumbers;

	TextTraitsRecord		mTextTraits;								// Text traits for general text

	virtual void	FinishCreateSelf();

	virtual void	DrawSelf();

	virtual void		ActivateSelf();
	virtual void		DeactivateSelf();

	virtual void		EnableSelf();
	virtual void		DisableSelf();

	virtual void		BeTarget();
	virtual void		DontBeTarget();

	virtual void		UserChangedText();

};

class CTextFieldMultiX : public CTextFieldX
{
public:
	enum { class_ID = 'TXFm' };


					CTextFieldMultiX(LStream *inStream);
	virtual 		~CTextFieldMultiX();
};

#endif
