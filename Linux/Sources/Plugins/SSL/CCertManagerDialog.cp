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
#include "CStaticText.h"
#include "CTableScrollbarSet.h"
#include "CTextDisplay.h"
#include "CTextTable.h"
#include "CViewCertDialog.h"

#include "JXTabs.h"

#include <JXChooseSaveFile.h>
#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <algorithm>
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CCertManagerDialog::CCertManagerDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mIndex = 1;
	mCerts.set_delete_data(false);
}

void CCertManagerDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,430, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,430);
    assert( obj1 != NULL );

    mTabs =
        new JXTabs(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 480,375);
    assert( mTabs != NULL );

    CScrollbarSet* sbs =
        new CScrollbarSet(mTabs,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 470,90);
    assert( sbs != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 395,395, 85,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mReloadBtn =
        new JXTextButton("Reload All Certificates", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,395, 145,25);
    assert( mReloadBtn != NULL );

    mImportBtn =
        new JXTextButton("Import...", mTabs,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 10,125, 70,25);
    assert( mImportBtn != NULL );

    mViewBtn =
        new JXTextButton("View...", mTabs,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 85,125, 70,25);
    assert( mViewBtn != NULL );

    mDeleteBtn =
        new JXTextButton("Delete", mTabs,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 160,125, 70,25);
    assert( mDeleteBtn != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Total:", mTabs,
                    JXWidget::kHElastic, JXWidget::kVElastic, 370,125, 40,20);
    assert( obj2 != NULL );
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    mTotal =
        new CStaticText("", mTabs,
                    JXWidget::kHElastic, JXWidget::kVElastic, 415,125, 55,20);
    assert( mTotal != NULL );

    JXDownRect* obj3 =
        new JXDownRect(mTabs,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,165, 470,205);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Certificate Details:", mTabs,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,158, 118,20);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Subject:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,15, 80,20);
    assert( obj5 != NULL );
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    mSubject =
        new CStaticText("", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,15, 375,45);
    assert( mSubject != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Issuer:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,65, 80,20);
    assert( obj6 != NULL );
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

    mIssuer =
        new CStaticText("", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,65, 375,45);
    assert( mIssuer != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Not Before:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,115, 80,20);
    assert( obj7 != NULL );
    const JFontStyle obj7_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj7->SetFontStyle(obj7_style);

    mNotBefore =
        new CStaticText("", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,115, 145,20);
    assert( mNotBefore != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Not After:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 245,115, 70,20);
    assert( obj8 != NULL );
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    mNotAfter =
        new CStaticText("", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 315,115, 145,20);
    assert( mNotAfter != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Fingerprint:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,140, 80,20);
    assert( obj9 != NULL );
    const JFontStyle obj9_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj9->SetFontStyle(obj9_style);

    mFingerprint =
        new CStaticText("", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,140, 375,20);
    assert( mFingerprint != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Location:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,165, 80,20);
    assert( obj10 != NULL );
    const JFontStyle obj10_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj10->SetFontStyle(obj10_style);

    mLocation =
        new CStaticText("", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,165, 375,30);
    assert( mLocation != NULL );

// end JXLayout

	mSubject->SetBreakCROnly(false);
	mIssuer->SetBreakCROnly(false);
	mLocation->SetBreakCROnly(false);

	cdstring s;
	mTabs->AppendCard(NULL, s.FromResource(IDS_CERTIFICATES_PERSONAL));
	mTabs->AppendCard(NULL, s.FromResource(IDS_CERTIFICATES_USERS));
	mTabs->AppendCard(NULL, s.FromResource(IDS_CERTIFICATES_SERVERS));
	mTabs->AppendCard(NULL, s.FromResource(IDS_CERTIFICATES_AUTHORITIES));

	mList = new CTextTable(sbs,sbs->GetScrollEnclosure(), JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 10, 10);
	mList->OnCreate();
	mList->SetSelectionMsg(true);
	mList->SetDoubleClickMsg(true);

	window->SetTitle("Certificates");
	SetButtons(mOKBtn, mOKBtn);

	ListenTo(mList);
	ListenTo(mTabs);
	ListenTo(mImportBtn);
	ListenTo(mViewBtn);
	ListenTo(mDeleteBtn);
	ListenTo(mReloadBtn);

	// Do initial list
	RefreshList();
	SelectionChange();
}

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDialog message handlers

void CCertManagerDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged) && (sender == mTabs))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		OnTabs(index);
		return;
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mImportBtn)
		{
			OnImportBtn();
			return;
		}
		else if (sender == mViewBtn)
		{
			OnViewBtn();
			return;
		}
		else if (sender == mDeleteBtn)
		{
			OnDeleteBtn();
			return;
		}
		else if (sender == mReloadBtn)
		{
			OnReloadBtn();
			return;
		}
	}
	else if (sender == mList)
	{
		if (message.Is(CTextTable::kSelectionChanged))
		{
			SelectionChange();
			return;
		}
		else if (message.Is(CTextTable::kLDblClickCell))
		{
			DoubleClick();
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

void CCertManagerDialog::OnTabs(JIndex index)
{
	if (mIndex != index)
	{
		mIndex = index;
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
		
		mViewBtn->Activate();
		if (mCerts.at(aCell.row - 1)->GetStore()->CanWrite())
			mDeleteBtn->Activate();
		else
			mDeleteBtn->Deactivate();
	}
	else
	{
		// Clear items
		ShowCertificate(NULL);
		
		mViewBtn->Deactivate();
		mDeleteBtn->Deactivate();
	}
}

void CCertManagerDialog::DoubleClick()
{
	OnViewBtn();
}

void CCertManagerDialog::OnImportBtn()
{
	// Do standard open dialog
    JPtrArray<JString> files(JPtrArrayT::kDeleteAll);
	if (JXGetChooseSaveFile()->ChooseFiles("Attachments", NULL, &files)) 
	{
		bool imported = false;
		JOrderedSetIterator<JString*>* filelist= files.NewIterator();
		JString* jfname;
		while (filelist->Next(&jfname))
		{
			cdstring fpath = jfname->GetCString();

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
		if (CErrorHandler::PutCautionAlert(true, IDE_ReallyDeleteCertificate) == CErrorHandler::Ok)
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
	    const JFontStyle black_style(kFalse, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
	    const JFontStyle red_style(kFalse, kFalse, 0, kFalse, (GetColormap())->GetRedColor());

		mSubject->SetText(cert->GetSubject());
		mIssuer->SetText(cert->GetIssuer());
		cdstrpair validity = cert->GetValidity();
		mNotBefore->SetText(validity.first);
		if (cert->IsNotBeforeValid())
	    	mNotBefore->SetFontStyle(black_style);
		else
	    	mNotBefore->SetFontStyle(red_style);
		mNotAfter->SetText(validity.second);
		if (cert->IsNotAfterValid())
	    	mNotAfter->SetFontStyle(black_style);
		else
	    	mNotAfter->SetFontStyle(red_style);
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
	CCertManagerDialog* dlog = new CCertManagerDialog(JXGetApplication());
	if (dlog->DoModal() != kDialogClosed_Cancel)
		dlog->Close();
}
