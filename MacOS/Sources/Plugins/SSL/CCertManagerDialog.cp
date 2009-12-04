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

// Source for CCertManagerDialog class

#include "CCertManagerDialog.h"

#include "CBalloonDialog.h"
#include "CCertificateManager.h"
#include "CCertificateStore.h"
#include "CErrorHandler.h"
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#include "CStaticText.h"
#include "CTextTable.h"
#include "CViewCertDialog.h"

#include <LPushButton.h>
#include <UStandardDialogs.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCertManagerDialog::CCertManagerDialog()
{
}

// Constructor from stream
CCertManagerDialog::CCertManagerDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mIndex = 1;
	mCerts.set_delete_data(false);
}

// Default destructor
CCertManagerDialog::~CCertManagerDialog()
{
	mIndex = 1;
	mCerts.set_delete_data(false);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCertManagerDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mList = (CTextTable*) FindPaneByID(paneid_CertManagerList);
	mList->AddListener(this);
	mViewBtn = (LPushButton*) FindPaneByID(paneid_CertManagerViewBtn);
	mDeleteBtn = (LPushButton*) FindPaneByID(paneid_CertManagerDeleteBtn);
	mTotal = (CStaticText*) FindPaneByID(paneid_CertManagerTotal);
	mSubject = (CStaticText*) FindPaneByID(paneid_CertManagerSubject);
	mIssuer = (CStaticText*) FindPaneByID(paneid_CertManagerIssuer);
	mNotBefore = (CStaticText*) FindPaneByID(paneid_CertManagerNotBefore);
	mNotAfter = (CStaticText*) FindPaneByID(paneid_CertManagerNotAfter);
	mFingerprint = (CStaticText*) FindPaneByID(paneid_CertManagerFingerprint);
	mLocation = (CStaticText*) FindPaneByID(paneid_CertManagerLocation);

	// Do initial list
	RefreshList();

	// Link broadcasters to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CCertManagerDialog);
}

// Handle OK button
void CCertManagerDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_CertManagerTabs:
		OnTabs(*(unsigned long*) ioParam);
		break;

	case msg_CertManagerListSelection:
		SelectionChange();
		break;

	case msg_CertManagerListDblClick:
		DoubleClick();
		break;

	case msg_CertManagerImportBtn:
		OnImportBtn();
		break;

	case msg_CertManagerViewBtn:
		OnViewBtn();
		break;

	case msg_CertManagerDeleteBtn:
		OnDeleteBtn();
		break;

	case msg_CertManagerReloadBtn:
		OnReloadBtn();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

void CCertManagerDialog::OnTabs(unsigned long index)
{
	if (mIndex != index)
	{
		mIndex = index;
		ShowCertificate(NULL);
		RefreshList();
		Refresh();
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
	case 1:
		type = CCertificateManager::ePersonalCertificates;
		break;
	case 2:
		type = CCertificateManager::eUserCertificates;
		break;
	case 3:
		type = CCertificateManager::eServerCertificates;
		break;
	case 4:
		type = CCertificateManager::eCACertificates;
		break;
	}
	
	// Get all certs of this type
	CCertificateManager::sCertificateManager->GetAllCertificatesInStores(type, mCerts);
	
	// Sort certificate list
	std::sort(mCerts.begin(), mCerts.end(), CCertificate::SubjectCompare);

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

	mList->SetContents(items);
	
	SetCounter();
}

void CCertManagerDialog::SelectionChange()
{
	if (mList->IsSelectionValid())
	{
		// Get selection in reverse to stay in sync
		STableCell aCell(0, 0);
		mList->GetFirstSelection(aCell);
		ShowCertificate(mCerts.at(aCell.row - 1));
		
		mViewBtn->Enable();
		if (mCerts.at(aCell.row - 1)->GetStore()->CanWrite())
			mDeleteBtn->Enable();
		else
			mDeleteBtn->Disable();
	}
	else
	{
		// Clear items
		ShowCertificate(NULL);
		
		mViewBtn->Disable();
		mDeleteBtn->Disable();
	}
}

void CCertManagerDialog::DoubleClick()
{
	OnViewBtn();
}

void CCertManagerDialog::OnImportBtn()
{
	LFileTypeList fileTypes((NavTypeList**) NULL);
	PP_StandardDialogs::LFileChooser chooser;
	
	chooser.GetDialogOptions()->optionFlags = kNavDefaultNavDlogOptions;		// Can select multiple files
	
	if (chooser.AskOpenFile(fileTypes))
	{
		bool imported = false;
		for(int i = 1; i <= chooser.GetNumberOfFiles(); i++)
		{
			PPx::FSObject fspec;
			chooser.GetFileSpec(i, fspec);

			cdstring fpath(fspec.GetPath());
			if (!fpath.empty())
			{
				// Determine type to import to
				CCertificateManager::ECertificateType type = CCertificateManager::eNone;
				switch(mIndex)
				{
				case 1:
					type = CCertificateManager::ePersonalCertificates;
					break;
				case 2:
					type = CCertificateManager::eUserCertificates;
					break;
				case 3:
					type = CCertificateManager::eServerCertificates;
					break;
				case 4:
					type = CCertificateManager::eCACertificates;
					break;
				}
				
				// Do import
				if (CCertificateManager::sCertificateManager->ImportCertificateFile(type, fpath))
					// Force list refresh
					imported = true;
			}
		}
		
		// Refresh if any were added
		if (imported)
			RefreshList();
	}
}

void CCertManagerDialog::OnViewBtn()
{
	if (mList->IsSelectionValid())
	{
		// Get selection in reverse to stay in sync
		STableCell aCell(0, 0);
		mList->GetFirstSelection(aCell);
		cdstring text = mCerts.at(aCell.row - 1)->StringCert();
		CViewCertDialog::PoseDialog(text);
	}
}

void CCertManagerDialog::OnDeleteBtn()
{
	if (mList->IsSelectionValid())
	{
		// Warn of deletion
		if (CErrorHandler::PutCautionAlert(true, STRx_Certificates, str_ReallyDeleteCertificate) == CErrorHandler::Ok)
		{
			STableCell aCell(0, 0);

			// Delete entire selection in reverse to stay in sync
			while(mList->GetPreviousSelectedCell(aCell))
			{
				CCertificate* cert = mCerts.at(aCell.row - 1);
				mList->RemoveRows(1, aCell.row, true);
				if (cert->GetStore()->DeleteCertificate(*cert))
					mCerts.erase(mCerts.begin() + (aCell.row - 1));
			}

			// Reset item counter
			SetCounter();
		}
	}
}

void CCertManagerDialog::OnReloadBtn()
{
	// Reload then refresh
	CCertificateManager::sCertificateManager->ReloadCertificates();
	RefreshList();
}

void CCertManagerDialog::SetCounter()
{
	mTotal->SetText(cdstring(mList->GetItemCount()));
}

void CCertManagerDialog::ShowCertificate(const CCertificate* cert)
{
	if (cert)
	{
		mSubject->SetText(cert->GetSubject());
		mIssuer->SetText(cert->GetIssuer());
		cdstrpair validity = cert->GetValidity();
		mNotBefore->SetText(validity.first);
		if (cert->IsNotBeforeValid())
			mNotBefore->SetTextTraitsID(261);
		else
			mNotBefore->SetTextTraitsID(278);
		mNotAfter->SetText(validity.second);
		if (cert->IsNotAfterValid())
			mNotAfter->SetTextTraitsID(261);
		else
			mNotAfter->SetTextTraitsID(278);
		mFingerprint->SetText(cert->GetFingerprint());
		mLocation->SetText(cert->GetStoreLocation());
	}
	else
	{
		cdstring txt;
		mSubject->SetText(txt);
		mIssuer->SetText(txt);
		mNotBefore->SetText(txt);
		mNotAfter->SetText(txt);
		mFingerprint->SetText(txt);
		mLocation->SetText(txt);
	}
}

void CCertManagerDialog::PoseDialog()
{
	CBalloonDialog	theHandler(paneid_CertManagerDialog, CMulberryApp::sApp);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{					
		MessageT hitMessage = theHandler.DoDialog();
		
		if (hitMessage == msg_OK)
			break;
	}
}
