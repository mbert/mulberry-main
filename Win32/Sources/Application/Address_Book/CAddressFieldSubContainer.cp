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


// CAddressFieldSubContainer.cp : implementation of the CAddressFieldSubContainer class
//

#include "CAddressFieldSubContainer.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CAdbkAddress.h"
#include "CAddressFieldContainer.h"
#include "CAddressFieldText.h"
#include "CAddressFieldMultiLine.h"

#include "templs.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressFieldSubContainer

BEGIN_MESSAGE_MAP(CAddressFieldSubContainer, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressFieldSubContainer construction/destruction

CAddressFieldSubContainer::CAddressFieldSubContainer()
{
	mType = eTel;
    mDirty = false;
}

CAddressFieldSubContainer::~CAddressFieldSubContainer()
{
	// Always delete criteria items
	for(std::vector<CWnd*>::iterator iter = mFields.begin(); iter != mFields.end(); iter++)
		delete *iter;
}

// Get details of sub-panes
BOOL CAddressFieldSubContainer::Create(const CRect& rect, CWnd* pParentWnd)
{
	return CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, rect, pParentWnd, IDC_STATIC);
}

int CAddressFieldSubContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// need this to ensure tabbing into criteria panels
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	CString s;
	s.LoadString(IDS_SEARCH_CRITERIATITLE);
	CRect r(0, 0, width, height);
	mTitle.CreateEx(WS_EX_CONTROLPARENT, _T("BUTTON"), s, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, r, this, IDC_STATIC);
	mTitle.SetFont(CMulberryApp::sAppFont);
	AddAlignment(new CWndAlignment(&mTitle, CWndAlignment::eAlign_WidthHeight));

    if (mType == eTel)
    	mTitle.SetWindowText(_T("Telephone"));
    else if (mType == eEmail)
    	mTitle.SetWindowText(_T("Email"));
    else if (mType == eIM)
    	mTitle.SetWindowText(_T("IM"));
    else if (mType == eCalAddress)
    	mTitle.SetWindowText(_T("Calendar"));
    else if (mType == eAddress)
    	mTitle.SetWindowText(_T("Address"));

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

    CAddressFieldBase* field = AddField();
    field->SetBottom();

	return 0;
}

// Resize sub-views
void CAddressFieldSubContainer::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CWnd::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

void CAddressFieldSubContainer::SetAddress(const CAdbkAddress* addr)
{
    // Remove all existing fields
	while(mFields.size() > 1)
	{
        CWnd* child = mFields.front();
        delete child;
        mFields.erase(mFields.begin());
	}

    if (addr != NULL)
    {
        if (mType == eTel)
        {
            CAdbkAddress::phonemap phones = addr->GetPhones();
            for(CAdbkAddress::phonemap::const_iterator iter = phones.begin(); iter != phones.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eEmail)
        {
            CAdbkAddress::emailmap emails = addr->GetEmails();
            for(CAdbkAddress::emailmap::const_iterator iter = emails.begin(); iter != emails.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eAddress)
        {
            CAdbkAddress::addrmap addrs = addr->GetAddresses();
            for(CAdbkAddress::addrmap::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField(true));
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eIM)
        {
            CAdbkAddress::immap ims = addr->GetIMs();
            for(CAdbkAddress::immap::const_iterator iter = ims.begin(); iter != ims.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eCalAddress)
        {
            CAdbkAddress::caladdrmap caladdrs = addr->GetCalendars();
            for(CAdbkAddress::caladdrmap::const_iterator iter = caladdrs.begin(); iter != caladdrs.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
    }

    Layout();
    mDirty = false;
}

bool CAddressFieldSubContainer::GetAddress(CAdbkAddress* addr)
{
    bool changed = mDirty;
    ulvector newtypes;
    cdstrvect newdatas;

	for(std::vector<CWnd*>::size_type i = 0; i < mFields.size() - 1; i++)
	{
        CAddressFieldBase* field = static_cast<CAddressFieldBase*>(mFields[i]);
        int newtype = 0;
        cdstring newdata;
        if (field->GetDetails(newtype, newdata))
        {
            changed = true;
        }
        newtypes.push_back(newtype);
        newdatas.push_back(newdata);
	}

    if (changed)
    {
        if (mType == eTel)
        {
            CAdbkAddress::phonemap& phones = addr->GetPhones();
            phones.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                phones.insert(CAdbkAddress::phonemap::value_type(static_cast<CAdbkAddress::EPhoneType>(newtypes[i]), newdatas[i]));
        }
        else if (mType == eEmail)
        {
            CAdbkAddress::emailmap& emails = addr->GetEmails();
            emails.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                emails.insert(CAdbkAddress::emailmap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }
        else if (mType == eAddress)
        {
            CAdbkAddress::addrmap& addrs = addr->GetAddresses();
            addrs.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                addrs.insert(CAdbkAddress::addrmap::value_type(static_cast<CAdbkAddress::EAddressType>(newtypes[i]), newdatas[i]));
        }
        else if (mType == eIM)
        {
            CAdbkAddress::immap& ims = addr->GetIMs();
            ims.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                ims.insert(CAdbkAddress::immap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }
        else if (mType == eCalAddress)
        {
            CAdbkAddress::caladdrmap& caladdrs = addr->GetCalendars();
            caladdrs.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                caladdrs.insert(CAdbkAddress::caladdrmap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }
    }

    return changed;
}

bool CAddressFieldSubContainer::SetInitialFocus()
{
	for(std::vector<CWnd*>::iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
		if (static_cast<CAddressFieldBase*>(*iter)->SetInitialFocus())
			return true;
	}

	return false;
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaHInitOffset = 8;
const int cCriteriaHWidthAdjust = 16;
const int cCriteriaVInitOffset = 20;
const int cCriteriaVBottomOffset = 4;
const int cCriteriaHeight = 25;
const int cCriteriaMultiHeight = 65;
const int cCriteriaWidth = 496;

CAddressFieldBase* CAddressFieldSubContainer::AddField(bool multi)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int cCriteriaHeight1 = cCriteriaHeight + small_offset;
	const int cCriteriaMultiHeight1 = cCriteriaMultiHeight + 4 * small_offset;

	CRect r;
	r.left = cCriteriaHInitOffset;
	r.right = cCriteriaWidth;
	r.top = 0;
	r.bottom = multi ? cCriteriaMultiHeight1 : cCriteriaHeight1;

    CAddressFieldBase* field = multi ?
    		(CAddressFieldBase*) new CAddressFieldMultiLine :
    		(CAddressFieldBase*) new CAddressFieldText;
    field->Create(r, this);
    field->SetMultipleInstance();
    if (mType == eTel)
        field->SetMenu(true);

    AddView(field);

    return field;
}

void CAddressFieldSubContainer::AddView(CWnd* view)
{
    // Put inside panel
    if (mFields.size() > 0)
        mFields.insert(mFields.end() - 1, view);
    else
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

void CAddressFieldSubContainer::AppendField()
{
    CAddressFieldBase* field = AddField(mType == eAddress);
    mDirty = true;
}

void CAddressFieldSubContainer::RemoveField(CWnd* view)
{
	for(std::vector<CWnd*>::iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
        CWnd* child = *iter;
        if (child == view)
        {
            delete child;
            mFields.erase(iter);
            Layout();
            mDirty = true;
            break;
        }
	}
}

void CAddressFieldSubContainer::Layout()
{
    CPoint new_pos(cCriteriaHInitOffset, cCriteriaVInitOffset);
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
    ::ResizeWindowTo(this, fsize.Width(), total_height + cCriteriaVBottomOffset);

    if (dynamic_cast<CAddressFieldContainer*>(GetParent()) != NULL)
        dynamic_cast<CAddressFieldContainer*>(GetParent())->Layout();
}
