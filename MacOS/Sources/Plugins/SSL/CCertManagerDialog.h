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

// Header for CCertManagerDialog class

#ifndef __CCERTMANAGERDIALOG__MULBERRY__
#define __CCERTMANAGERDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CCertificate.h"

// Constants

// Panes
const	PaneIDT		paneid_CertManagerDialog = 22110;
const	PaneIDT		paneid_CertManagerTabs = 'TABS';
const	PaneIDT		paneid_CertManagerList = 'LIST';
const	PaneIDT		paneid_CertManagerImportBtn = 'IMPO';
const	PaneIDT		paneid_CertManagerViewBtn = 'VIEW';
const	PaneIDT		paneid_CertManagerDeleteBtn = 'DELE';
const	PaneIDT		paneid_CertManagerTotal = 'TOTA';
const	PaneIDT		paneid_CertManagerSubject = 'SUBJ';
const	PaneIDT		paneid_CertManagerIssuer = 'ISSU';
const	PaneIDT		paneid_CertManagerNotBefore = 'BEFO';
const	PaneIDT		paneid_CertManagerNotAfter = 'AFTE';
const	PaneIDT		paneid_CertManagerFingerprint = 'FING';
const	PaneIDT		paneid_CertManagerLocation = 'LOCA';

// Mesages
const	MessageT	msg_CertManagerTabs = 'TABS';
const	MessageT	msg_CertManagerListSelection = 'LIST';
const	MessageT	msg_CertManagerListDblClick = 'LISD';
const	MessageT	msg_CertManagerImportBtn = 'IMPO';
const	MessageT	msg_CertManagerViewBtn = 'VIEW';
const	MessageT	msg_CertManagerDeleteBtn = 'DELE';
const	MessageT	msg_CertManagerReloadBtn = 'RELO';

// Resources
const	ResIDT		RidL_CCertManagerDialog = paneid_CertManagerDialog;

const	ResIDT	STRx_Certificates = 22101;
enum {
	str_ReallyDeleteCertificate = 1
};

// Type
class CTextTable;
class LPushButton;
class CStaticText;

class CCertManagerDialog : public LDialogBox
{
private:

public:
	enum { class_ID = 'Cmgr' };

					CCertManagerDialog();
					CCertManagerDialog(LStream *inStream);
	virtual 		~CCertManagerDialog();

	static void		PoseDialog();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	CTextTable*		mList;
	LPushButton*	mViewBtn;
	LPushButton*	mDeleteBtn;
	CStaticText*	mTotal;
	CStaticText*	mSubject;
	CStaticText*	mIssuer;
	CStaticText*	mNotBefore;
	CStaticText*	mNotAfter;
	CStaticText*	mFingerprint;
	CStaticText*	mLocation;
	
	unsigned long		mIndex;
	CCertificateList	mCerts;

			void	OnTabs(unsigned long index);
			
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
