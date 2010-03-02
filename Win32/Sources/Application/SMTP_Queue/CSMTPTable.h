/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CMessage.h"

#include <set>

// Classes
class CMbox;
class CMessageList;
class CSMTPView;

class CSMTPTable : public CMailboxTable
{
	friend class CSMTPWindow;

	DECLARE_DYNCREATE(CSMTPTable)

public:
					CSMTPTable();
	virtual 		~CSMTPTable();

	virtual void	InitPos(void);							// Set initial position

	virtual BOOL	RedrawWindow(LPCRECT lpRectUpdate = NULL, CRgn* prgnUpdate = NULL, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );
	virtual void	RefreshSelection(void);										// Update button & captions as well
	virtual void	RefreshRow(TableIndexT row);								// Update button & captions as well

	virtual void	DoSelectionChanged(void);

	// Key/Mouse related

	// Common updaters

	// Command handlers
	virtual bool	OpenMailMessage(TableIndexT row, bool* option_key);		// Open a specified mail message
	afx_msg void	OnMessageDelete(void);

protected:	
	CSMTPView*			mSMTPView;							// Owning view
	bool				mTestSelectionAndHold;
	bool				mTestSelectionAndPriority;

	virtual void		GetDrawStyle(const CMessage* aMsg, COLORREF& color, short& style, bool& strike) const;
	virtual int			GetPlotFlag(const CMessage* aMsg) const;
	virtual bool		UsesBackground(const CMessage* aMsg) const;
	virtual COLORREF	GetBackground(const CMessage* aMsg) const;

	// Common updaters
	afx_msg void	OnUpdateSMTPHoldMessage(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateSMTPPriority(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateSMTPEnable(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnSMTPHoldMessage();
	afx_msg void	OnSMTPPriority();
	afx_msg void	OnSMTPDelete();
	afx_msg void	OnAccountsPopup(UINT nID);
	afx_msg void	OnSMTPAccountPopup(long index);
	afx_msg void	OnSMTPEnableQueue();
	
	virtual void	UpdateItems(void);							// Update after possible change
	virtual void	UpdateState(void);							// Update button/title state
	virtual void	UpdateCaptions(void);						// Update captions

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
