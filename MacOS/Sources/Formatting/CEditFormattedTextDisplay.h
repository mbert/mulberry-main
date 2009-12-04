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


#ifndef __CEDITFORMATTEDTEXTDISPLAY__MULBERRY__
#define __CEDITFORMATTEDTEXTDISPLAY__MULBERRY__

#include "CFormattedTextDisplay.h"
#include "CStyleToolbar.h"

const 	MessageT	msg_Bold = 3010;
const 	MessageT	msg_Italic = 3011;
const 	MessageT	msg_Underline = 3012;

const 	MessageT	msg_AlignLeft = 3013;
const 	MessageT	msg_AlignCenter = 3014;
const 	MessageT	msg_AlignRight = 3015;
const 	MessageT	msg_AlignJustify = 3016;

const 	MessageT	msg_Font = 3017;
const 	MessageT	msg_Size = 3018;
const 	MessageT	msg_Color = 3019;


const	ResIDT		FONTS = 212;
const 	ResIDT		SIZES = 213;


enum EColor
{
	eblack = 1,
	ered,
	egreen,
	eblue,
	eyellow,
	ecyan,
	emagenta,
	emulberry,
	ewhite,
	eother
};


class CFormattedTextDisplay;
class CStyleToolbar;
class LBroadcaster;


class CEditFormattedTextDisplay : public CFormattedTextDisplay, public LListener
{
public:
	enum { class_ID = 'EFTD' };
	
	CEditFormattedTextDisplay(LStream *inStream);
	virtual ~CEditFormattedTextDisplay();

	void SetToolbar(CStyleToolbar *toolbar);
	void ListenToMessage(MessageT inMessage,void *ioParam);

	virtual	Boolean ObeyCommand(CommandT inCommand,
							void* ioParam);							// Handle commands our way
	virtual void FindCommandStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);			// Handle menus our way

	virtual void DoPaste();
	
	void doFormattingStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);			
	bool getFormattingStatus(CommandT inCommand);
	void doFormattingCommand(CommandT inCommand);

	void doAlignmentStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);
	void doAlignment(CommandT inCommand);

	void doColorStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);
	void doColor(CommandT inCommand);
	void doColorMessage(SInt32 color);

	void doFontCommand();
	void doFontStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);

	void doSizeCommand();
	void doSizeStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);

	virtual void ActivateSelf();
	virtual void DeactivateSelf();

	virtual void DoQuotation(void) {} 	// Do nothing: quotation never displayed in draft

protected:
	virtual void	BeTarget(void);
	virtual void	DontBeTarget(void);
	

private:
	CStyleToolbar* mEnriched;
	
};

#endif
