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


// CCertManagerDialog.h : header file
//

#ifndef __CCERTMANAGERDIALOG__MULBERRY__
#define __CCERTMANAGERDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CCertificate.h"

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDialog dialog

class CStaticText;
class CTextTable;
class JXTabs;
class JXTextButton;

class CCertManagerDialog : public CDialogDirector
{
// Construction
public:
	CCertManagerDialog(JXDirector* supervisor);   // standard constructor

	static void		PoseDialog();

protected:
// begin JXLayout

    JXTabs*       mTabs;
    JXTextButton* mOKBtn;
    JXTextButton* mReloadBtn;
    JXTextButton* mImportBtn;
    JXTextButton* mViewBtn;
    JXTextButton* mDeleteBtn;
    CStaticText*  mTotal;
    CStaticText*  mSubject;
    CStaticText*  mIssuer;
    CStaticText*  mNotBefore;
    CStaticText*  mNotAfter;
    CStaticText*  mFingerprint;
    CStaticText*  mLocation;

// end JXLayout
	CTextTable*			mList;
	unsigned long		mIndex;
	CCertificateList	mCerts;

	void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	OnTabs(JIndex index);
			
			void	RefreshList();

			void	SelectionChange();
			void	DoubleClick();
			void	OnImportBtn();
			void	OnViewBtn();
			void	OnDeleteBtn();
			void	OnReloadBtn();

			void	SetCounter();
			void	ShowCertificate(const CCertificate* cert);
};
#endif
