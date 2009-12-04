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


// CPrefsIdentity.cpp : implementation file
//


#include "CPrefsIdentity.h"

#include "CErrorHandler.h"
#include "CEditIdentities.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsIdentity property page

IMPLEMENT_DYNCREATE(CPrefsIdentity, CPrefsPanel)

CPrefsIdentity::CPrefsIdentity() : CPrefsPanel(CPrefsIdentity::IDD)
{
	//{{AFX_DATA_INIT(CPrefsIdentity)
	mSmartAddressSet = _T("");
	//}}AFX_DATA_INIT
}

CPrefsIdentity::~CPrefsIdentity()
{
}

void CPrefsIdentity::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsIdentity)
	//DDX_Control(pDX, IDC_IDENTITIES, mIdentitiesCtrl);
	DDX_Control(pDX, IDC_CHANGEIDENTITY, mChangeIdentityBtn);
	DDX_Control(pDX, IDC_DUPLICATEIDENTITY, mDuplicateIdentityBtn);
	DDX_Control(pDX, IDC_DELETEIDENTITY, mDeleteIdentityBtn);
	DDX_Check(pDX, IDC_CONTEXTTIED, mContextTied);
	DDX_Check(pDX, IDC_MSGTIED, mMsgTied);
	DDX_Check(pDX, IDC_MBOXINHERITTIED, mTiedMboxInherit);
	DDX_Control(pDX, IDC_OUTGOINGADDRESSES, mSmartAddressList);
	DDX_UTF8Text(pDX, IDC_OUTGOINGADDRESSES, mSmartAddressSet);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsIdentity, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsIdentity)
	ON_COMMAND(IDC_IDENTITIES, OnSelchangeIdentities)
	ON_COMMAND(IDC_IDENTITIES_SELECT, OnChangeIdentity)
	ON_BN_CLICKED(IDC_ADDIDENTITY, OnAddIdentity)
	ON_BN_CLICKED(IDC_CHANGEIDENTITY, OnChangeIdentity)
	ON_BN_CLICKED(IDC_DUPLICATEIDENTITY, OnDuplicateIdentity)
	ON_BN_CLICKED(IDC_DELETEIDENTITY, OnDeleteIdentity)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPrefsIdentity::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTextTable::eBroadcast_Drag:
		{
			CTextTable::SBroadcastDrag* data = static_cast<CTextTable::SBroadcastDrag*>(param);
			if (data->mTable == &mIdentitiesCtrl)
				// Do drag move action
				DoMoveIdentities(data->mDragged, data->mDropped);
		}
		break;
	default:;
	}
}

// Set up params for DDX
void CPrefsIdentity::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	// Set values
	mList = &prefs->mIdentities;
	mContextTied = prefs->mContextTied.GetValue();
	mMsgTied = prefs->mMsgTied.GetValue();
	mTiedMboxInherit = prefs->mTiedMboxInherit.GetValue();
	mSmartAddressSet.clear();
	for(short i = 0; i < prefs->mSmartAddressList.GetValue().size(); i++)
		mSmartAddressSet += prefs->mSmartAddressList.GetValue()[i] + "\r\n";
}

// Get params from DDX
void CPrefsIdentity::UpdatePrefs(CPreferences* prefs)
{
	// Identity list kept in sync by button actions
	
	// Get values
	prefs->mContextTied.SetValue(mContextTied);
	prefs->mMsgTied.SetValue(mMsgTied);
	prefs->mTiedMboxInherit.SetValue(mTiedMboxInherit);
	cdstrvect accumulate;
	cdstring temp(mSmartAddressSet);
	char* p = ::strtok(temp.c_str_mod(), "\r\n");
	while(p)
	{
		accumulate.push_back(p);
		p = ::strtok(NULL, "\r\n");
	}	
	prefs->mSmartAddressList.SetValue(accumulate);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsIdentity message handlers

void CPrefsIdentity::InitControls(void)
{
	// Create identities list
	mIdentitiesCtrl.SubclassDlgItem(IDC_IDENTITIES, this);
	mIdentitiesCtrl.EnableDragAndDrop();
	mIdentitiesCtrl.Add_Listener(this);
	mIdentitiesCtrl.SetMsgTarget(this);
	mIdentitiesCtrl.SetSelectionMsg(IDC_IDENTITIES);
	mIdentitiesCtrl.SetDoubleClickMsg(IDC_IDENTITIES_SELECT);
}

void CPrefsIdentity::SetControls(void)
{
	// Put identities into list
	InitIdentitiesList();

	// Disable buttons when no selection
	mChangeIdentityBtn.EnableWindow(false);
	mDuplicateIdentityBtn.EnableWindow(false);
	mDeleteIdentityBtn.EnableWindow(false);
}

// Initialise the list
void CPrefsIdentity::InitIdentitiesList(void)
{
	cdstrvect items;
	for(CIdentityList::const_iterator iter = mList->GetValue().begin(); iter != mList->GetValue().end(); iter++)
		items.push_back((*iter).GetIdentity());

	mIdentitiesCtrl.SetContents(items);
}

void CPrefsIdentity::OnSelchangeIdentities()
{
	// Disable buttons when no selection
	mChangeIdentityBtn.EnableWindow(mIdentitiesCtrl.IsSelectionValid());
	mDuplicateIdentityBtn.EnableWindow(mIdentitiesCtrl.IsSelectionValid());
	mDeleteIdentityBtn.EnableWindow(mIdentitiesCtrl.IsSelectionValid());
}

void CPrefsIdentity::OnAddIdentity()
{
	while(true)
	{
		// Create the dialog
		CEditIdentities dlog(CSDIFrame::GetAppTopWindow());
		CIdentity new_identity;
		dlog.SetIdentity(mCopyPrefs, &new_identity);

		// Let Dialog process events
		if (dlog.DoModal() == IDOK)
		{
			// Copy changed identity
			dlog.GetIdentity(&new_identity);
			
			// Check validity
			if (new_identity.GetIdentity().empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
				continue;
			}

			// Add to prefs and popup
			((CIdentityList&) mList->GetValue()).push_back(new_identity);
			mList->SetDirty();

			// Insert item
			mIdentitiesCtrl.AddItem(new_identity.GetIdentity());
			break;
		}
		else
			break;
	}
}

void CPrefsIdentity::OnChangeIdentity()
{
	// Get selected items
	ulvector selected;
	mIdentitiesCtrl.GetSelectedRows(selected);
	for(ulvector::iterator iter = selected.begin(); iter != selected.end(); iter++)
	{
		while(true)
		{
			// Create the dialog
			CEditIdentities dlog(CSDIFrame::GetAppTopWindow());
			CIdentity& old_identity = (CIdentity&) mList->GetValue()[*iter - 1];
			dlog.SetIdentity(mCopyPrefs, &old_identity);

			// Let Dialog process events
			if (dlog.DoModal() == IDOK)
			{
				// Copy changed identity - init to old one to copy over fields not edited by the dialog
				CIdentity edit_identity(old_identity);
				dlog.GetIdentity(&edit_identity);

				// Check validity
				if (edit_identity.GetIdentity().empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
					continue;
				}

				// Carry on
				mCopyPrefs->ReplaceIdentity(old_identity, edit_identity);

				// Replace item
				STableCell aCell(*iter, 1);
				const cdstring& txt = edit_identity.GetIdentity();
				mIdentitiesCtrl.SetCellData(aCell, txt, txt.length() + 1);
				mIdentitiesCtrl.RefreshRow(aCell.row);
				break;
			}
			else
				break;
		}
	}
	
	// Reset selection
	mIdentitiesCtrl.UnselectAllCells();
	for(ulvector::iterator iter = selected.begin(); iter != selected.end(); iter++)
		mIdentitiesCtrl.SelectRow(*iter);
}

void CPrefsIdentity::OnDuplicateIdentity()
{
	// Get selected items
	STableCell aCell(0, 0);

	// Duplicate entire selection
	while(mIdentitiesCtrl.GetNextSelectedCell(aCell))
	{
		// Create the dialog
		CIdentity new_identity(mList->GetValue()[aCell.row - 1]);
		new_identity.SetIdentity(new_identity.GetIdentity() + " copy");

		// Add to prefs and popup
		((CIdentityList&) mList->GetValue()).push_back(new_identity);
		mList->SetDirty();

		// Insert item
		const cdstring& txt = new_identity.GetIdentity();
		mIdentitiesCtrl.InsertRows(1, mCopyPrefs->mIdentities.GetValue().size(), txt, txt.length() + 1, true);
	}
}

void CPrefsIdentity::OnDeleteIdentity()
{
	STableCell aCell(0, 0);

	// Delete entire selection in reverse to stay in sync
	while(mIdentitiesCtrl.GetPreviousSelectedCell(aCell))
	{
		mIdentitiesCtrl.RemoveRows(1, aCell.row, true);

		// Remove item from prefs
		mCopyPrefs->DeleteIdentity(&(*(mCopyPrefs->mIdentities.Value().begin() + (aCell.row - 1))));
	}
}

// Move identities
void CPrefsIdentity::DoMoveIdentities(const ulvector& from, unsigned long to)
{
	// Move items in prefs
	mCopyPrefs->MoveIdentity(from, to);
	
	// Redo list display
	InitIdentitiesList();
}
