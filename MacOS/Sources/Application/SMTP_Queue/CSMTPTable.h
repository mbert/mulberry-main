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


// Header for CSMTPTable class

#ifndef __CSMTPTABLE__MULBERRY__
#define __CSMTPTABLE__MULBERRY__

#include "CMailboxTable.h"

// Panes

// Messages

// Resources
const	ResIDT		RidL_CSMTPBtns = 1500;

// Classes
class CMessage;
class CSMTPView;

class CSMTPTable : public CMailboxTable
{
	friend class CSMTPWindow;

private:
	CSMTPView*			mSMTPView;							// Owning view
	bool				mTestSelectionAndHold;
	bool				mTestSelectionAndPriority;

public:
	enum { class_ID = 'MbSm' };

					CSMTPTable();
					CSMTPTable(LStream *inStream);
	virtual 		~CSMTPTable();

private:
			void	InitSMTPTable(void);					// Do common init

protected:
	virtual void	FinishCreateSelf(void);					// Get details of sub-panes
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

public:
	virtual	void	InitPos(void);								// Set initial position
	
	virtual void	Refresh();								// Update button & captions as well
	virtual void	RefreshSelection();						// Update button & captions as well
	virtual void	RefreshRow(const TableIndexT aRow);		// Update button & captions as well

	virtual void	DoSelectionChanged(void);

protected:
	void	OnSMTPHoldMessage();
	void	OnSMTPPriority();
	void	OnSMTPDelete();
	void	OnSMTPAccountPopup(long index);
	void	OnSMTPEnableQueue();
	
protected:
	virtual void	GetDrawStyle(const CMessage* aMsg, RGBColor& color, Style& style, bool& strike) const;
	virtual ResIDT	GetPlotFlag(const CMessage* aMsg) const;
	virtual bool	UsesBackground(const CMessage* aMsg) const;
	virtual RGBColor GetBackground(const CMessage* aMsg) const;

	virtual bool	OpenMailMessage(TableIndexT row, bool* option_key);	// Display a specified mail message

	virtual void	UpdateItems(void);							// Update after possible change

	void	UpdateState(void);						// Update buttons/title
	void	UpdateCaptions(void);					// Update number captions
};

#endif
