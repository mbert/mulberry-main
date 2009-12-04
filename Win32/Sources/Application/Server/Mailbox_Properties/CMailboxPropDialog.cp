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


// CMailboxPropDialog.cp : implementation file
//

#include "CMailboxPropDialog.h"

#include "CIMAPClient.h"
#include "CMboxList.h"
#include "CMulberryApp.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMailboxPropDialog dialog

IMPLEMENT_DYNAMIC(CMailboxPropDialog, CHelpPropertySheet)

CMailboxPropDialog::CMailboxPropDialog(CWnd* pParent /*=NULL*/)
	: CHelpPropertySheet(_T("Mailbox:"), pParent)
{
	mMboxList = nil;
	mMboxProtocol = nil;
}


BEGIN_MESSAGE_MAP(CMailboxPropDialog, CHelpPropertySheet)
	//{{AFX_MSG_MAP(CMailboxPropDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMailboxPropDialog::SetMboxList(CMboxList* mbox_list)
{
	mMboxList = mbox_list;
	mMboxProtocol = ((CMbox*) mbox_list->front())->GetProtocol();

	AddPage(&mPropMailboxOptions);
	mPropMailboxOptions.SetMboxList(mbox_list);

	AddPage(&mPropMailboxGeneral);
	mPropMailboxGeneral.SetMboxList(mbox_list);

	AddPage(&mPropMailboxServer);
	mPropMailboxServer.SetProtocol(mMboxProtocol);

	// See if only directories present
	bool only_dirs = true;
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		
		only_dirs &= mbox->IsDirectory();
	}

	// Only add those availble
	bool logged_in = mMboxProtocol->IsLoggedOn();
	if (!only_dirs && logged_in && mMboxProtocol->UsingACLs())
	{
		AddPage(&mPropMailboxACL);
		mPropMailboxACL.SetMboxList(mbox_list);
	}

	if (!only_dirs && logged_in && mMboxProtocol->UsingQuotas())
	{
		AddPage(&mPropMailboxQuota);
		mPropMailboxQuota.SetMboxList(mbox_list);
	}
}

void CMailboxPropDialog::SetProtocol(CMboxProtocol* proto)
{
	mMboxList = NULL;
	mMboxProtocol = proto;

	AddPage(&mPropMailboxServer);
	mPropMailboxServer.SetProtocol(mMboxProtocol);
}

#if 0
// Specially hacked to prevent sheet != page font problems
void CMailboxPropDialog::BuildPropPageArray()
{
	// delete existing prop page array
	delete[] (PROPSHEETPAGE*)m_psh.ppsp;
	m_psh.ppsp = NULL;

	// build new prop page array
	AFX_OLDPROPSHEETPAGE* ppsp = new AFX_OLDPROPSHEETPAGE[m_pages.GetSize()];
	m_psh.ppsp = (LPPROPSHEETPAGE)ppsp;
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		memcpy(&ppsp[i], &pPage->m_psp, sizeof(pPage->m_psp));

		LPCDLGTEMPLATE pTemplate = NULL;
		PROPSHEETPAGE& psp = (PROPSHEETPAGE&)ppsp[i];

		if (psp.dwFlags & PSP_DLGINDIRECT)
		{
			pTemplate = psp.pResource;
		}
		else
		{
			HRSRC hResource = ::FindResource(psp.hInstance,
				psp.pszTemplate, RT_DIALOG);
			HGLOBAL hTemplate = LoadResource(psp.hInstance,
				hResource);
			pTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
		}

		ASSERT(pTemplate != NULL);

		// WINBUG: Windows currently does not support DIALOGEX resources!
		// Assert that the template is *not* a DIALOGEX template.
		// DIALOGEX templates are not supported by the PropertySheet API.

		// To change a DIALOGEX template back to a DIALOG template,
		// remove the following:
		//  1. Extended styles on the dialog
		//  2. Help IDs on any control in the dialog
		//  3. Control IDs that are DWORDs
		//  4. Weight, italic, or charset attributes on the dialog's font

		ASSERT(((DLGTEMPLATEEX*)pTemplate)->signature != 0xFFFF); // See above!
/*																				<-- The big hack
#ifndef _AFX_NO_OCC_SUPPORT
		// if the dialog could contain OLE controls, deal with them now
		if (afxOccManager != NULL)
			pTemplate = pPage->InitDialogInfo(pTemplate);
#endif

#ifndef _MAC
		// set font of property page to same font used by property sheet
		HGLOBAL hTemplate = _ChangePropPageFont(pTemplate,
			(m_psh.dwFlags & PSH_WIZARD));

		if (pPage->m_hDialogTemplate != NULL)
		{
			GlobalFree(pPage->m_hDialogTemplate);
			pPage->m_hDialogTemplate = NULL;
		}

		if (hTemplate != NULL)
		{
			pTemplate = (LPCDLGTEMPLATE)hTemplate;
			pPage->m_hDialogTemplate = hTemplate;
		}
#endif //!_MAC
*/

		psp.pResource = pTemplate;
		psp.dwFlags |= PSP_DLGINDIRECT;
	}

	m_psh.nPages = m_pages.GetSize();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CMailboxPropDialog message handlers

BOOL CMailboxPropDialog::OnInitDialog()
{
	const short cBtnWidth = 98;
	const short cBtnHeight = 22;
	const short cBtnGap = 30;
	const short cBtnExtraGap = 20;

	BOOL result = CHelpPropertySheet::OnInitDialog();

	// Set flags
	m_psh.dwFlags &= ~PSH_HASHELP;			// Disable help button
	m_psh.dwFlags &= ~PSH_NOAPPLYNOW;		// Disable Apply Now button
	ModifyStyleEx(0, WS_EX_CONTEXTHELP);	// Add context help

	// Get width of OK button
	CWnd* btn = GetDlgItem(IDOK);
	CRect rect;
	btn->GetWindowRect(rect);
	ScreenToClient(rect);
	int offset = rect.Width();

	// Move OK
	btn->GetWindowRect(rect);
	ScreenToClient(rect);
	rect.OffsetRect(3*offset, 0);
	btn->MoveWindow(rect);

	// Hide Cancel/Apply/Help
	btn = GetDlgItem(IDCANCEL);
	btn->ShowWindow(SW_HIDE);
	btn = GetDlgItem(ID_APPLY_NOW);
	btn->ShowWindow(SW_HIDE);
	btn = GetDlgItem(IDHELP);
	btn->ShowWindow(SW_HIDE);

	// Load string title
	cdstring title;
	if (mMboxList)
	{
		title.FromResource((mMboxList->size() == 1 ? "UI::MailboxProp::Title" : "UI::MailboxProp::MultiTitle"));
		
		// Add name if single mailbox
		if (mMboxList->size() == 1)
			title += mMboxList->front()->GetName();
	}
	else if (mMboxProtocol)
	{
		title.FromResource("UI::MailboxProp::ServerTitle");
		
		// Add name of protocol
		title += mMboxProtocol->GetAccountName();
	}

	// Now set window title
	CUnicodeUtils::SetWindowTextUTF8(this, title);

	return result;
}

void CMailboxPropDialog::OnOK()
{
	// Do default OK action
	EndDialog(IDOK);
}

void CMailboxPropDialog::OnCancel()
{
	// Do default cancel action
	EndDialog(IDCANCEL);
}

bool CMailboxPropDialog::PoseDialog(CMboxList* mbox_list, CMboxProtocol* proto)
{
	CMailboxPropDialog	prop_dlog;

	if (mbox_list)
		prop_dlog.SetMboxList(mbox_list);
	else if (proto)
		prop_dlog.SetProtocol(proto);

	// Let DialogHandler process events
	prop_dlog.DoModal();

	return true;
}