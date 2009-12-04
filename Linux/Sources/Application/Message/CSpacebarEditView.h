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


// CSpacebarEditView.h : header file
//

#ifndef __CSPACEBAREDITVIEW__MULBERRY__
#define __CSPACEBAREDITVIEW__MULBERRY__

#include "CAddressText.h"

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEditView window

class CMessageView;
class CMessageWindow;
class CMessage;

class CSpacebarEditView : public CAddressText
{
// Construction
public:
	CSpacebarEditView(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual ~CSpacebarEditView();

	virtual void OnCreate();

	virtual void SetMessageView(CMessageView* msgView)
					{ mMsgView = msgView; }
	virtual void SetMessageWindow(CMessageWindow* msgWindow)
					{ mMsgWindow = msgWindow; }

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual void Print(JPagePrinter& p);

protected:
	virtual CMessage* GetMessage() const;
	virtual void CleanMessage();
	virtual bool ChangePrintFont() const;
	virtual bool MessagePrint() const;

	virtual void	AddPrintSummary();						// Temporarily add header summary for printing
	virtual void	RemovePrintSummary();					// Remove temp header summary after printing

	virtual JCoordinate	GetPrintHeaderHeight(JPagePrinter& p) const;
	virtual JCoordinate	GetPrintFooterHeight(JPagePrinter& p) const;
	virtual void		DrawPrintHeader(JPagePrinter& p, const JCoordinate headerHeight);
	virtual void		DrawPrintFooter(JPagePrinter& p, const JCoordinate footerHeight);

private:
	CMessageView*	mMsgView;
	CMessageWindow*	mMsgWindow;

	JCoordinate GetCaptionHeight(JPagePrinter& p, const cdstring& caption) const;
	void		DrawCaption(JPagePrinter& p, const JRect& captionRect, const cdstring& caption, bool box);
};

/////////////////////////////////////////////////////////////////////////////

#endif
