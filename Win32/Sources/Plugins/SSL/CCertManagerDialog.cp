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

// CCertManagerDialog.cp : implementation file
//

#include "CCertManagerDialog.h"

#include "CCertificateManager.h"
#include "CCertificateStore.h"
#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CViewCertDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDialog dialog


CCertManagerDialog::CCertManagerDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCertManagerDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCertManagerDialog)
	//}}AFX_DATA_INIT
	mIndex = 0;
	mCerts.set_delete_data(false);
	mNotBeforeError = false;
	mNotAfterError = false;
}


void CCertManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCertManagerDialog)
	DDX_Control(pDX, IDC_CERTMGR_TABS, mTabs);
	DDX_Control(pDX, IDC_CERTMGR_VIEW, mViewBtn);
	DDX_Control(pDX, IDC_CERTMGR_DELETE, mDeleteBtn);
	DDX_Control(pDX, IDC_CERTMGR_TOTAL, mTotal);
	DDX_Control(pDX, IDC_CERTMGR_SUBJECT, mSubject);
	DDX_Control(pDX, IDC_CERTMGR_ISSUER, mIssuer);
	DDX_Control(pDX, IDC_CERTMGR_NOTBEFORE, mNotBefore);
	DDX_Control(pDX, IDC_CERTMGR_NOTAFTER, mNotAfter);
	DDX_Control(pDX, IDC_CERTMGR_FINGERPRINT, mFingerprint);
	DDX_Control(pDX, IDC_CERTMGR_LOCATION, mLocation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCertManagerDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCertManagerDialog)
	ON_WM_CTLCOLOR()

	ON_NOTIFY(TCN_SELCHANGE, IDC_CERTMGR_TABS, OnSelChangeTabs)
	ON_BN_CLICKED(IDC_CERTMGR_LISTSELCHANGE, OnListSelChange)
	ON_BN_CLICKED(IDC_CERTMGR_LISTDBLCLICK, OnListDblClick)
	ON_BN_CLICKED(IDC_CERTMGR_RELOAD, OnReload)
	ON_BN_CLICKED(IDC_CERTMGR_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_CERTMGR_VIEW, OnView)
	ON_BN_CLICKED(IDC_CERTMGR_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDialog message handlers

// Called during startup
BOOL CCertManagerDialog::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();

	// Set tabs
	CString s;
	TC_ITEM tabs;
	tabs.mask = TCIF_TEXT;
	s.LoadString(IDS_CERTIFICATES_PERSONAL);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*)s);
	mTabs.InsertItem(0, &tabs);
	s.LoadString(IDS_CERTIFICATES_USERS);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*)s);
	mTabs.InsertItem(1, &tabs);
	s.LoadString(IDS_CERTIFICATES_SERVERS);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*)s);
	mTabs.InsertItem(2, &tabs);
	s.LoadString(IDS_CERTIFICATES_AUTHORITIES);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*)s);
	mTabs.InsertItem(3, &tabs);
	mTabs.SetFont(CMulberryApp::sAppFont);

	mList.SubclassDlgItem(IDC_CERTMGR_LIST, this);
	mList.SetMsgTarget(this);
	mList.SetSelectionMsg(IDC_CERTMGR_LISTSELCHANGE);
	mList.SetDoubleClickMsg(IDC_CERTMGR_LISTDBLCLICK);

	// Do initial list
	RefreshList();
	OnListSelChange();

	return true;
}

HBRUSH CCertManagerDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// use helper in CWnd
	HBRUSH result = CHelpDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// Look for error items
	COLORREF red = RGB(0xDD,0x00,0x00);
	if (mNotBeforeError && (pWnd->m_hWnd == mNotBefore.m_hWnd))
		pDC->SetTextColor(red);
	if (mNotAfterError && (pWnd->m_hWnd == mNotAfter.m_hWnd))
		pDC->SetTextColor(red);
	
	return result;
}

void CCertManagerDialog::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (mIndex != mTabs.GetCurSel())
	{
		mIndex = mTabs.GetCurSel();
		ShowCertificate(NULL);
		RefreshList();
	}
}
			
void CCertManagerDialog::RefreshList()
{
	// Clear existing certs list
	mCerts.clear();
	
	// Determine type to show
	CCertificateManager::ECertificateType type = CCertificateManager::eNone;
	switch(mIndex)
	{
	case 0:
		type = CCertificateManager::ePersonalCertificates;
		break;
	case 1:
		type = CCertificateManager::eUserCertificates;
		break;
	case 2:
		type = CCertificateManager::eServerCertificates;
		break;
	case 3:
		type = CCertificateManager::eCACertificates;
		break;
	}
	
	// Get all certs of this type
	CCertificateManager::sCertificateManager->GetAllCertificatesInStores(type, mCerts);
	
	// Sort certificate list
	::sort(mCerts.begin(), mCerts.end(), CCertificate::SubjectCompare);

	// Get display data from all certs
	cdstrvect items;
	for(CCertificateList::const_iterator iter1 = mCerts.begin(); iter1 != mCerts.end(); iter1++)
	{
		cdstring text;
		switch(type)
		{
		case CCertificateManager::ePersonalCertificates:
		case CCertificateManager::eUserCertificates:
			for(cdstrvect::const_iterator iter2 = (*iter1)->GetEmail().begin(); iter2 != (*iter1)->GetEmail().end(); iter2++)
			{
				if (!text.empty())
					text += ", ";
				text += *iter2;
			}
			break;
		case CCertificateManager::eServerCertificates:
			for(cdstrvect::const_iterator iter2 = (*iter1)->GetDNS().begin(); iter2 != (*iter1)->GetDNS().end(); iter2++)
			{
				if (!text.empty())
					text += ", ";
				text += *iter2;
			}
			break;
		case CCertificateManager::eCACertificates:
		default:
			text = (*iter1)->GetName();
			break;
		}
		
		if (text.empty())
			text = (*iter1)->GetSubject();
		items.push_back(text);
	}

	mList.SetContents(items);
	
	SetCounter();
}

void CCertManagerDialog::OnListSelChange()
{
	if (mList.IsSelectionValid())
	{
		// Get selection in reverse to stay in sync
		STableCell aCell(0, 0);
		mList.GetFirstSelection(aCell);
		ShowCertificate(mCerts.at(aCell.row - 1));
		
		mViewBtn.EnableWindow(true);
		if (mCerts.at(aCell.row - 1)->GetStore()->CanWrite())
			mDeleteBtn.EnableWindow(true);
		else
			mDeleteBtn.EnableWindow(false);
	}
	else
	{
		// Clear items
		ShowCertificate(NULL);
		
		mViewBtn.EnableWindow(false);
		mDeleteBtn.EnableWindow(false);
	}
}

void CCertManagerDialog::OnListDblClick()
{
	OnView();
}

void CCertManagerDialog::OnImport()
{
	// Do standard open dialog prompting the user (with all document templates)
	CString filter = _T("All Files (*.*)|*.*||");
	CFileDialog dlgFile(true, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST, filter, this);

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		bool imported = false;

		POSITION pos = dlgFile.GetStartPosition();
		while(pos)
		{
			// Get next file path
			cdstring fpath(dlgFile.GetNextPathName(pos));

			// Determine type to import to
			CCertificateManager::ECertificateType type = CCertificateManager::eNone;
			switch(mIndex)
			{
			case 0:
				type = CCertificateManager::ePersonalCertificates;
				break;
			case 1:
				type = CCertificateManager::eUserCertificates;
				break;
			case 2:
				type = CCertificateManager::eServerCertificates;
				break;
			case 3:
				type = CCertificateManager::eCACertificates;
				break;
			}
			
			// Do import
			if (CCertificateManager::sCertificateManager->ImportCertificateFile(type, fpath))
				// Force list refresh
				imported = true;

		}
		
		// Refresh if any were added
		if (imported)
			RefreshList();
	}
}

void CCertManagerDialog::OnView()
{
	if (mList.IsSelectionValid())
	{
		// Get selection in reverse to stay in sync
		STableCell aCell(0, 0);
		mList.GetFirstSelection(aCell);
		cdstring text = mCerts.at(aCell.row - 1)->StringCert();
		CViewCertDialog::PoseDialog(text);
	}
}

void CCertManagerDialog::OnDelete()
{
	if (mList.IsSelectionValid())
	{
		// Warn of deletion
		if (CErrorHandler::PutCautionAlert(true, IDE_ReallyDeleteCertificate) == CErrorHandler::Ok)
		{
			STableCell aCell(0, 0);

			// Delete entire selection in reverse to stay in sync
			while(mList.GetPreviousSelectedCell(aCell))
			{
				CCertificate* cert = mCerts.at(aCell.row - 1);
				mList.RemoveRows(1, aCell.row, true);
				if (cert->GetStore()->DeleteCertificate(*cert))
					mCerts.erase(mCerts.begin() + (aCell.row - 1));
			}

			// Reset item counter
			SetCounter();
		}
	}
}

void CCertManagerDialog::OnReload()
{
	// Reload then refresh
	CCertificateManager::sCertificateManager->ReloadCertificates();
	RefreshList();
}

void CCertManagerDialog::SetCounter()
{
	CUnicodeUtils::SetWindowTextUTF8(mTotal, cdstring((unsigned long)mList.GetItemCount()));
}

void CCertManagerDialog::ShowCertificate(const CCertificate* cert)
{
	if (cert)
	{
		CUnicodeUtils::SetWindowTextUTF8(&mSubject, cert->GetSubject());
		CUnicodeUtils::SetWindowTextUTF8(&mIssuer, cert->GetIssuer());
		cdstrpair validity = cert->GetValidity();
		CUnicodeUtils::SetWindowTextUTF8(&mNotBefore, validity.first);
		mNotBeforeError = !cert->IsNotBeforeValid();
		mNotBefore.RedrawWindow();
		CUnicodeUtils::SetWindowTextUTF8(&mNotAfter, validity.second);
		mNotAfterError = !cert->IsNotAfterValid();
		mNotAfter.RedrawWindow();
		CUnicodeUtils::SetWindowTextUTF8(&mFingerprint, cert->GetFingerprint());
		CUnicodeUtils::SetWindowTextUTF8(&mLocation, cert->GetStoreLocation());
	}
	else
	{
		cdstring txt;
		CUnicodeUtils::SetWindowTextUTF8(&mSubject, txt);
		CUnicodeUtils::SetWindowTextUTF8(&mIssuer, txt);
		CUnicodeUtils::SetWindowTextUTF8(&mNotBefore, txt);
		CUnicodeUtils::SetWindowTextUTF8(&mNotAfter, txt);
		CUnicodeUtils::SetWindowTextUTF8(&mFingerprint, txt);
		CUnicodeUtils::SetWindowTextUTF8(&mLocation, txt);
	}
}

void CCertManagerDialog::PoseDialog()
{
	CCertManagerDialog dlog(CSDIFrame::GetAppTopWindow());

	dlog.DoModal();
}
