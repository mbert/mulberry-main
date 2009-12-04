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


// Header for CGroupPreview class

#ifndef __CGROUPPREVIEW__MULBERRY__
#define __CGROUPPREVIEW__MULBERRY__

#include "CAddressPreviewBase.h"

#include "CGrayBackground.h"
#include "CIconWnd.h"
#include "CToolbarButton.h"

// Classes
class CGroup;

class CGroupPreview : public CAddressPreviewBase
{
public:
					CGroupPreview();
	virtual 		~CGroupPreview();

	virtual void	Close();
	virtual void	Focus();

	void	SetGroup(CGroup* grp);
	void	ClearGroup();
	CGroup*	GetGroup() const
		{ return mGroup; }

// Dialog Data
	//{{AFX_DATA(CEditGroupDialog)
	enum { IDD = IDD_GROUPPREVIEW };
	cdstring	mNickName;
	cdstring	mGroupName;
	cdstring	mAddressList;
	CEdit	mAddressListCtrl;
	//}}AFX_DATA
	CGrayBackground		mHeader;				// Header for parts table
	CIconWnd			mIcon;					// Icon
	CStatic				mDescriptor;			// Caption details
	CToolbarButton		mZoom;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditGroupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	CGroup*			mGroup;

			void	SetFields(const CGroup* grp);			// Set fields in dialog
			bool	GetFields(CGroup* grp);					// Get fields from dialog

	virtual void InitDialog();

	// message handlers
	afx_msg void OnGroupEditSort();

	afx_msg void OnZoomPane(void);

	DECLARE_MESSAGE_MAP()
};

#endif
