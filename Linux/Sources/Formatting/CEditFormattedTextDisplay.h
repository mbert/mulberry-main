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

class CFontNameMenu;
class CFontSizeMenu;
class CStyleToolbar;

class CEditFormattedTextDisplay : public CFormattedTextDisplay
{
public:
	CEditFormattedTextDisplay(JXContainer* enclosure,
			const HSizingOption hSizing, const VSizingOption vSizing,
			const JCoordinate x, const JCoordinate y,
			const JCoordinate w, const JCoordinate h);
	virtual ~CEditFormattedTextDisplay();

	void SetToolbar(CStyleToolbar *toolbar);
	void ShowToolbar(bool show);

	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	void DoFormattingStatus(unsigned long cmd, CCmdUI* cmdui);			
	bool GetFormattingStatus(unsigned long inCommand);
	void DoFormattingCommand(unsigned long inCommand);

	void DoAlignmentStatus(unsigned long cmd, CCmdUI* cmdui);
	void DoAlignmentCommand(unsigned long inCommand);

	void DoColorStatus(unsigned long cmd, CCmdUI* cmdui);
	void DoColorCommand(unsigned long inCommand);
	void DoColorItem(JIndex choice);

	void DoFontStatus(CFontNameMenu* menu);
	void DoFontItem(CFontNameMenu* menu);

	void DoSizeStatus(CFontSizeMenu* menu);
	void DoSizeItem(CFontSizeMenu* menu);

	virtual void Activate();
	virtual void Deactivate();

			void StyledToolbarListen(JXWidget* listen_to);

	virtual void DoQuotation(void) {} 	// Do nothing: quotation never displayed in draft

protected:
	CStyleToolbar* mEnriched;
	
	JColorIndex		mBlackIndex;
	JColorIndex		mRedIndex;
	JColorIndex		mGreenIndex;
	JColorIndex		mBlueIndex;
	JColorIndex		mYellowIndex;
	JColorIndex		mCyanIndex;
	JColorIndex		mMagentaIndex;
	JColorIndex		mMulberryIndex;
	JColorIndex		mWhiteIndex;
	
	void	UpdateStyledToolbar();
};

#endif
