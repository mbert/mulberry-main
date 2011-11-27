/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// CAddressFieldContainer.cp : implementation of the CAddressFieldContainer class
//

#include "CAddressFieldContainer.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CAdbkAddress.h"
#include "CAddressFieldText.h"
#include "CAddressFieldMultiLine.h"
#include "CAddressFieldSubContainer.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressFieldContainer

BEGIN_MESSAGE_MAP(CAddressFieldContainer, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressFieldContainer construction/destruction

CAddressFieldContainer::CAddressFieldContainer()
{
}

CAddressFieldContainer::~CAddressFieldContainer()
{
	// Always delete criteria items
	for(std::vector<CWnd*>::iterator iter = mFields.begin(); iter != mFields.end(); iter++)
		delete *iter;
}

// Get details of sub-panes
BOOL CAddressFieldContainer::Create(const CRect& rect, CWnd* pParentWnd)
{
	return CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, rect, pParentWnd, IDC_STATIC);
}

int CAddressFieldContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// need this to ensure tabbing into criteria panels
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

    CAddressFieldBase* name = AddField();
    CAddressFieldBase* nickname = AddField();
    CAddressFieldBase* organization = AddField();
    AddContainer(CAddressFieldSubContainer::eTel);
    AddContainer(CAddressFieldSubContainer::eEmail);
    AddContainer(CAddressFieldSubContainer::eIM);
    AddContainer(CAddressFieldSubContainer::eCalAddress);
    AddContainer(CAddressFieldSubContainer::eAddress);
    CAddressFieldBase* notes = AddField(true);

	return 0;
}

// Resize sub-views
void CAddressFieldContainer::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CWnd::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

void CAddressFieldContainer::SetAddress(const CAdbkAddress* addr)
{
    // Only called if top level
    static_cast<CAddressFieldBase*>(mFields[0])->SetDetails("Name:", 0, addr ? addr->GetName() : "");
    static_cast<CAddressFieldBase*>(mFields[1])->SetDetails("Nick-Name:", 0, addr ? addr->GetADL() : "");
    static_cast<CAddressFieldBase*>(mFields[2])->SetDetails("Organization:", 0, addr ? addr->GetCompany() : "");
    static_cast<CAddressFieldSubContainer*>(mFields[3])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[4])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[5])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[6])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[7])->SetAddress(addr);
    static_cast<CAddressFieldBase*>(mFields[8])->SetDetails("Note:", 0, addr ? addr->GetNotes() : "");
}

bool CAddressFieldContainer::GetAddress(CAdbkAddress* addr)
{
    bool changed = false;
    int newtype = 0;
    cdstring newdata;
    if (static_cast<CAddressFieldBase*>(mFields[0])->GetDetails(newtype, newdata))
    {
        addr->SetName(newdata);
        changed = true;
    }
    if (static_cast<CAddressFieldBase*>(mFields[1])->GetDetails(newtype, newdata))
    {
        addr->SetADL(newdata);
        changed = true;
    }
    if (static_cast<CAddressFieldBase*>(mFields[2])->GetDetails(newtype, newdata))
    {
        addr->SetCompany(newdata);
        changed = true;
    }
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[3])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[4])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[5])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[6])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[7])->GetAddress(addr);
    if (static_cast<CAddressFieldBase*>(mFields[8])->GetDetails(newtype, newdata))
    {
        addr->SetNotes(newdata);
        changed = true;
    }

    return changed;
}

bool CAddressFieldContainer::SetInitialFocus()
{
	for(std::vector<CWnd*>::iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
		if (dynamic_cast<CAddressFieldBase*>(*iter) != NULL && static_cast<CAddressFieldBase*>(*iter)->SetInitialFocus())
			return true;
		else if (dynamic_cast<CAddressFieldSubContainer*>(*iter)!= NULL && static_cast<CAddressFieldSubContainer*>(*iter)->SetInitialFocus())
			return true;
	}

	return false;
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 4;
const int cCriteriaHeight = 25;
const int cCriteriaMultiHeight = 65;
const int cCriteriaWidth = 496;

CAddressFieldBase* CAddressFieldContainer::AddField(bool multi)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int cCriteriaHeight1 = cCriteriaHeight + small_offset;
	const int cCriteriaMultiHeight1 = cCriteriaMultiHeight + 4 * small_offset;

	CRect r;
	r.left = 0;
	r.right = cCriteriaWidth;
	r.top = 0;
	r.bottom = multi ? cCriteriaMultiHeight1 : cCriteriaHeight1;

    CAddressFieldBase* field = multi ?
    		(CAddressFieldBase*) new CAddressFieldMultiLine :
    		(CAddressFieldBase*) new CAddressFieldText;
    field->Create(r, this);
    field->SetSingleInstance();

    AddView(field);

    return field;
}

CAddressFieldSubContainer* CAddressFieldContainer::AddContainer(int ctype)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int cCriteriaHeight1 = cCriteriaHeight + small_offset;

	CRect r;
	r.left = 0;
	r.right = cCriteriaWidth;
	r.top = 0;
	r.bottom = cCriteriaHeight1;

	CAddressFieldSubContainer* container = new CAddressFieldSubContainer;
    container->SetContainerType(static_cast<CAddressFieldSubContainer::EContainerType>(ctype));
    container->Create(r, this);

    AddView(container);

    return container;
}

void CAddressFieldContainer::AddView(CWnd* view)
{
    // Put inside panel
    mFields.push_back(view);

    // Now adjust sizes
	CRect size;
	view->GetWindowRect(size);

    // Resize groups so that width first inside criteria
    CRect gsize;
    GetWindowRect(gsize);
	::ResizeWindowBy(view, gsize.Width() - cCriteriaHWidthAdjust - size.Width(), 0, false);
	AddAlignment(new CWndAlignment(view, CWndAlignment::eAlign_TopWidth));

    // Position new sub-panel
    Layout();
    view->ShowWindow(SW_SHOW);
}

void CAddressFieldContainer::Layout()
{
    CPoint new_pos(0, cCriteriaVInitOffset);
    int total_height = cCriteriaVInitOffset;
	for(std::vector<CWnd*>::const_iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
        CWnd* child = *iter;
        CRect child_size;
        child->GetWindowRect(child_size);
    	child->SetWindowPos(nil, new_pos.x, new_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        new_pos.y += child_size.Height();
        total_height += child_size.Height();
	}

    CRect fsize;
    GetWindowRect(fsize);
    ::ResizeWindowTo(this, fsize.Width(), total_height + cCriteriaVInitOffset);

    //fsize = GetEnclosure()->GetBoundsGlobal();
    //static_cast<CBlankScrollable*>(GetEnclosure())->SetBounds(fsize.width(), total_height + cCriteriaVInitOffset);

	RedrawWindow();
}
