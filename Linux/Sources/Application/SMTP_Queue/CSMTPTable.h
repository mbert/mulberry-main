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

#include "CSMTPWindow.h"

// Classes
class CSMTPView;

class CSMTPTable : public CMailboxTable
{
	friend class CSMTPWindow;

public:
	CSMTPTable(JXScrollbarSet* scrollbarSet, 
				JXContainer* enclosure,
				const HSizingOption hSizing, 
				const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual ~CSMTPTable();

private:
			void	InitSMTPTable(void);					// Do common init

protected:
	virtual void	OnCreate();								// Get details of sub-panes

public:
	virtual	void	InitPos();								// Set initial position
	
	virtual void	Refresh() const;						// Update button & captions as well
	virtual void	RefreshSelection();						// Update button & captions as well
	virtual void	RefreshRow(TableIndexT aRow);			// Update button & captions as well

	virtual void	DoSelectionChanged();

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:
	void	OnUpdateSMTPHoldMessage(CCmdUI* pCmdUI);
	void	OnUpdateSMTPPriority(CCmdUI* pCmdUI);
	void	OnUpdateSMTPEnable(CCmdUI* pCmdUI);

	void	OnSMTPHoldMessage();
	void	OnSMTPPriority();
	void	OnSMTPDelete();
	void	OnSMTPAccountPopup(long index);
	void	OnSMTPEnableQueue();
	
protected:
	CSMTPView*			mSMTPView;							// Owning view
	bool	mTestSelectionAndHold;
	bool	mTestSelectionAndPriority;

	virtual void		GetDrawStyle(const CMessage* aMsg, JRGB& color, short& style) const;
	virtual long		GetPlotFlag(const CMessage* aMsg) const;
	virtual bool		UsesBackground(const CMessage* aMsg) const;
	virtual JColorIndex	GetBackground(const CMessage* aMsg) const;

	virtual bool	OpenMailMessage(TableIndexT row, bool* option_key);	// Display a specified mail message

	virtual void	UpdateItems();							// Update after possible change

	void	UpdateState();						// Update toolbar buttons
	void	UpdateCaptions();					// Update number captions
};

#endif
