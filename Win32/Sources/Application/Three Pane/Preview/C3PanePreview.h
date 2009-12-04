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


// Header for C3PanePreview class

#ifndef __C3PANEPREVIEW__MULBERRY__
#define __C3PANEPREVIEW__MULBERRY__

#include "C3PaneParentPanel.h"
#include "CToolbarButton.h"

// Classes
class CAddressView;
class CEventPreview;
class CMessageView;
class C3PaneAddress;
class C3PaneMessage;
class C3PaneEvent;

class C3PanePreview : public C3PaneParentPanel
{
public:
					C3PanePreview();
	virtual 		~C3PanePreview();

	virtual void	ListenTo_Message(long msg, void* param);

	virtual bool	TestClose();
	virtual void	DoClose();

	virtual bool	IsSpecified() const;

	virtual void	ResetState();						// Reset state from prefs
	virtual void	SaveDefaultState();					// Save state in prefs
			
	virtual void	SetViewType(N3Pane::EViewType view);

			void	SetTitle(const cdstring& title);
			void	SetIcon(UINT icon);

	CMessageView*	GetMessageView() const;
	CAddressView*	GetAddressView() const;
	CEventPreview*	GetEventView() const;

protected:
	C3PaneMessage*		mMessage;
	C3PaneAddress*		mAddress;
	C3PaneEvent*		mEvent;

private:
			void	MakeMessage();
			void	MakeAddress();
			void	MakeEvent();

protected:
	// Generated message map functions
	//{{AFX_MSG(CEditIdentities)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
