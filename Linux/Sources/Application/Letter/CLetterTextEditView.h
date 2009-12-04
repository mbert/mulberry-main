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


// CLetterTextEditView.h : header file
//

#ifndef __CLETTERTEXTEDITVIEW__MULBERRY__
#define __CLETTERTEXTEDITVIEW__MULBERRY__

#include "CEditFormattedTextDisplay.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterTextEditView window

class CLetterWindow;

class CLetterTextEditView : public CEditFormattedTextDisplay
{
	friend class CLetterWindow;

public:
	CLetterTextEditView(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);


	virtual ~CLetterTextEditView();

	virtual void SetLetterWindow(CLetterWindow* ltrWindow)
		{ mLtrWindow = ltrWindow; }

	// Command handlers
	virtual void	OnEditPaste();

protected:
	virtual JBoolean	TEGetExternalClipboard(JString16* text, JRunArray<Font>* style) const;

	// Printing
	virtual CMessage* GetMessage() const;
	virtual void CleanMessage();
	virtual bool ChangePrintFont() const;
	virtual bool MessagePrint() const;

	virtual void	AddPrintSummary();						// Temporarily add header summary for printing
	virtual void	RemovePrintSummary();					// Remove temp header summary after printing

	virtual void OnCreate();

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);

	virtual void	AddDropFlavor(unsigned int theFlavor) // Set its drop flavor
						{ mDropFlavors.push_back(theFlavor); }

	virtual JBoolean	TEXWillAcceptDrop(const JArray<Atom>& typeList,
										  const Atom action, const Time time,
										  const JXWidget* source);
	virtual JBoolean	TEXConvertDropData(const JArray<Atom>& typeList,
										   const Atom action, const Time time,
										   JString16* text, JRunArray<Font>* style);

private:
	CLetterWindow*		mLtrWindow;
	CDisplayFormatter*	mFormatter;
	mutable CMessage*	mPrintMessage;
	typedef std::vector<Atom> CFlavorsList;
  	CFlavorsList		mDropFlavors;				// List of flavors to accept

};

/////////////////////////////////////////////////////////////////////////////

#endif
