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


// CAdbkPropDialog.cp : implementation file
//

#include "CAdbkPropDialog.h"

#include "CAdbkProtocol.h"
#include "CMulberryApp.h"
#include "CRemoteAddressBook.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CAdbkPropDialog dialog

IMPLEMENT_DYNAMIC(CAdbkPropDialog, CHelpPropertySheet)

CAdbkPropDialog::CAdbkPropDialog(CWnd* pParent /*=NULL*/)
	: CHelpPropertySheet(_T("Address Book:"), pParent)
{
	mAdbkList = nil;

}


BEGIN_MESSAGE_MAP(CAdbkPropDialog, CHelpPropertySheet)
	//{{AFX_MSG_MAP(CAdbkPropDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAdbkPropDialog::SetAdbkList(CFlatAdbkList* adbk_list)
{
	mAdbkList = adbk_list;

	// Only add those availble
	AddPage(&mPropAdbkGeneral);
	mPropAdbkGeneral.SetAdbkList(adbk_list);

	CRemoteAddressBook* radbk = dynamic_cast<CRemoteAddressBook*>(mAdbkList->front());
	bool logged_in = radbk && radbk->GetProtocol()->IsLoggedOn();
	if (logged_in && radbk->GetProtocol()->UsingACLs())
	{
		AddPage(&mPropAdbkACL);
		mPropAdbkACL.SetAdbkList(adbk_list);
	}
}

// Specially hacked to prevent sheet != page font problems
#if 0
void CAdbkPropDialog::BuildPropPageArray()
{
	// delete existing prop page array
	free((void*)m_psh.ppsp);
	m_psh.ppsp = NULL;

	// determine size of PROPSHEETPAGE array
	int i;
	int nBytes = 0;
	for (i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		nBytes += pPage->m_psp.dwSize;
	}

	// build new prop page array
	PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*)malloc(nBytes);
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
// CAdbkPropDialog message handlers

BOOL CAdbkPropDialog::OnInitDialog()
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
	title.FromResource((mAdbkList->size() == 1 ? "UI::AdbkProps::Title" : "UI::AdbkProps::MultiTitle"));
	
	// Add name if single adbk
	if (mAdbkList->size() == 1)
		title += mAdbkList->front()->GetName();

	// Now set window title
	CUnicodeUtils::SetWindowTextUTF8(this, title);

	return result;
}

void CAdbkPropDialog::OnOK()
{
	// Do default OK action
	EndDialog(IDOK);
}

void CAdbkPropDialog::OnCancel()
{
	// Do default cancel action
	EndDialog(IDCANCEL);
}
