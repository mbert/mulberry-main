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


// Source for CSearchCriteriaSIEVE class

#include "CSearchCriteriaSIEVE.h"

#include "CFilterItem.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSearchCriteriaContainer.h"
#include "CSearchStyle.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	eCriteria_From = 1,
	eCriteria_To,
	eCriteria_CC,
	eCriteria_Bcc,
	eCriteria_Recipient,
	eCriteria_Correspondent,
	eCriteria_Sender,
	eCriteria_Subject,
	eCriteria_Header,
	eCriteria_Size,
	eCriteria_Separator1,
	eCriteria_Group,
	eCriteria_Separator2,
	eCriteria_All
};

enum
{
	eAddressMethod_Is = 1,
	eAddressMethod_IsNot,
	eAddressMethod_Contains,
	eAddressMethod_NotContains,
	eAddressMethod_StartsWith,
	eAddressMethod_NotStartsWith,
	eAddressMethod_EndsWith,
	eAddressMethod_NotEndsWith,
	eAddressMethod_Separator1,
	eAddressMethod_IsMe,
	eAddressMethod_IsNotMe,
	eAddressMethod_Separator2,
	eAddressMethod_Matches,
	eAddressMethod_NotMatches
};

enum
{
	eStringMethod_Is = 1,
	eStringMethod_IsNot,
	eStringMethod_Contains,
	eStringMethod_NotContains,
	eStringMethod_StartsWith,
	eStringMethod_NotStartsWith,
	eStringMethod_EndsWith,
	eStringMethod_NotEndsWith,
	eStringMethod_Separator1,
	eStringMethod_Matches,
	eStringMethod_NotMatches
};


enum
{
	eSizeMethod_Larger = 1,
	eSizeMethod_Smaller
};

enum
{
	eSize_Bytes = 1,
	eSize_KBytes,
	eSize_MBytes
};

enum
{
	eMode_Or = 1,
	eMode_And
};

// Default constructor
CSearchCriteriaSIEVE::CSearchCriteriaSIEVE()
{
}

// Constructor from stream
CSearchCriteriaSIEVE::CSearchCriteriaSIEVE(LStream *inStream)
		: CSearchCriteria(inStream)
{
}

// Default destructor
CSearchCriteriaSIEVE::~CSearchCriteriaSIEVE()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSearchCriteriaSIEVE::FinishCreateSelf(void)
{
	// Do inherited
	CSearchCriteria::FinishCreateSelf();

	// Get controls
	mPopup1 = (LPopupButton*) FindPaneByID(paneid_SearchCriteriaSPopup1);
	mPopup2 = (LPopupButton*) FindPaneByID(paneid_SearchCriteriaSPopup2);
	mPopup3 = (LPopupButton*) FindPaneByID(paneid_SearchCriteriaSPopup3);
	mPopup4 = (LPopupButton*) FindPaneByID(paneid_SearchCriteriaSPopup4);
	mText1 = (CTextFieldX*) FindPaneByID(paneid_SearchCriteriaSText1);
	mText2 = (CTextFieldX*) FindPaneByID(paneid_SearchCriteriaSText2);
	mText3 = (CTextFieldX*) FindPaneByID(paneid_SearchCriteriaSText3);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CSearchCriteriaSIEVEBtns);
}

// Handle buttons
void CSearchCriteriaSIEVE::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_SearchCriteriaSPopup1:
			OnSetCriteria(*(long*) ioParam);
			break;

		case msg_SearchCriteriaSPopup2:
			OnSetMethod(*(long*) ioParam);
			break;

		default:
			CSearchCriteria::ListenToMessage(inMessage, ioParam);
			break;
	}
}

bool CSearchCriteriaSIEVE::DoActivate()
{
	CTextFieldX* activate = NULL;
	if (mText2->IsVisible())
		activate = mText2;
	else if (mText1->IsVisible())
		activate = mText1;
	else if (mText3->IsVisible())
		activate = mText3;

	if (activate)
	{
		activate->GetSuperCommander()->SetLatentSub(activate);
		LCommander::SwitchTarget(activate);
		activate->SelectAll();

		return true;
	}
	else
		return false;
}

long CSearchCriteriaSIEVE::ShowOrAnd(bool show)
{
	if (show)
		mPopup4->Show();
	else
		mPopup4->Hide();

	return 0;
}

bool CSearchCriteriaSIEVE::IsOr() const
{
	return (mPopup4->GetValue() == eMode_Or);
}

void CSearchCriteriaSIEVE::SetOr(bool use_or)
{
	mPopup4->SetValue(use_or ? eMode_Or : eMode_And);
}

void CSearchCriteriaSIEVE::OnSetCriteria(long item1)
{
	// Set popup menu for method and show/hide text field as approriate
	bool method_refresh = false;
	switch(item1)
	{
	case eCriteria_From:
	case eCriteria_To:
	case eCriteria_CC:
	case eCriteria_Bcc:
	case eCriteria_Recipient:
	case eCriteria_Correspondent:
	case eCriteria_Sender:
		if (mPopup2->GetMenuID() != MENU_SIEVEAddressCriteria)
		{
			mPopup2->SetMenuID(MENU_SIEVEAddressCriteria);
			mPopup2->SetValue(1);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Show();
		mText2->Hide();
		mText3->Hide();
		break;
	case eCriteria_Subject:
		if (mPopup2->GetMenuID() != MENU_SIEVEStringCriteria)
		{
			mPopup2->SetMenuID(MENU_SIEVEStringCriteria);
			mPopup2->SetValue(1);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Show();
		mText2->Hide();
		mText3->Hide();
		break;
	case eCriteria_Header:
		mText2->SetText(cdstring::null_str);
		mPopup2->Hide();
		mPopup3->Hide();
		mText1->Show();
		mText2->Show();
		mText3->Hide();
		break;
	case eCriteria_Size:
		if (mPopup2->GetMenuID() != MENU_SIEVESizeCriteria)
		{
			mPopup2->SetMenuID(MENU_SIEVESizeCriteria);
			mPopup2->SetValue(1);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Show();
		mText1->Hide();
		mText2->Hide();
		mText3->Show();
		break;
	case eCriteria_Group:
	case eCriteria_All:
		mPopup2->Hide();
		mPopup3->Hide();
		mText1->Hide();
		mText2->Hide();
		mText3->Hide();
		break;
	}

	// Special for group display
	if (item1 == eCriteria_Group)
		MakeGroup(CFilterItem::eSIEVE);
	else
		RemoveGroup();

	// Refresh method for new criteria
	if (method_refresh)
		OnSetMethod(mPopup2->GetValue());
}

void CSearchCriteriaSIEVE::OnSetMethod(long item)
{
	// Show/hide text field as appropriate
	switch(mPopup1->GetValue())
	{
	case eCriteria_From:
	case eCriteria_To:
	case eCriteria_CC:
	case eCriteria_Bcc:
	case eCriteria_Recipient:
	case eCriteria_Correspondent:
	case eCriteria_Sender:
		switch(item)
		{
		case eAddressMethod_Is:
		case eAddressMethod_IsNot:
		case eAddressMethod_Contains:
		case eAddressMethod_NotContains:
		case eAddressMethod_StartsWith:
		case eAddressMethod_NotStartsWith:
		case eAddressMethod_EndsWith:
		case eAddressMethod_NotEndsWith:
		case eAddressMethod_Matches:
			mText1->Show();
			break;
		case eAddressMethod_IsMe:
		case eAddressMethod_IsNotMe:
			mText1->Hide();
			mText1->SetText(cdstring::null_str);
			break;
		}
		break;
	default:
		break;
	}
}

CSearchItem* CSearchCriteriaSIEVE::GetSearchItem() const
{
	switch(mPopup1->GetValue())
	{
	case eCriteria_From:
		return ParseAddress(CSearchItem::eFrom);
	case eCriteria_To:
		return ParseAddress(CSearchItem::eTo);
	case eCriteria_CC:
		return ParseAddress(CSearchItem::eCC);
	case eCriteria_Bcc:
		return ParseAddress(CSearchItem::eBcc);
	case eCriteria_Recipient:
		return ParseAddress(CSearchItem::eRecipient);
	case eCriteria_Correspondent:
		return ParseAddress(CSearchItem::eCorrespondent);
	case eCriteria_Sender:
		return ParseAddress(CSearchItem::eSender);
	case eCriteria_Subject:
		return ParseText(CSearchItem::eSubject);
	case eCriteria_Header:
		{
			cdstring text1 = mText1->GetText();
			cdstring text2 = mText2->GetText();
			return new CSearchItem(CSearchItem::eHeader, text2, text1);
		}
	case eCriteria_Size:
		return ParseSize();
	case eCriteria_Group:
		return mGroupItems->ConstructSearch();
	case eCriteria_All:
		return new CSearchItem(CSearchItem::eAll);
	default:
		return nil;
	}
}

CSearchItem* CSearchCriteriaSIEVE::ParseAddress(CSearchItem::ESearchType type) const
{
	cdstring text = mText1->GetText();

	switch(mPopup2->GetValue())
	{
	case eAddressMethod_Is:
		return new CSearchItem(type, text, CSearchItem::eIs);
	case eAddressMethod_IsNot:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eIs));
	case eAddressMethod_Contains:
		return new CSearchItem(type, text, CSearchItem::eContains);
	case eAddressMethod_NotContains:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eContains));
	case eAddressMethod_StartsWith:
		return new CSearchItem(type, text, CSearchItem::eStartsWith);
	case eAddressMethod_NotStartsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eStartsWith));
	case eAddressMethod_EndsWith:
		return new CSearchItem(type, text, CSearchItem::eEndsWith);
	case eAddressMethod_NotEndsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eEndsWith));
	case eAddressMethod_IsMe:
		return new CSearchItem(type);
	case eAddressMethod_IsNotMe:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type));
	case eAddressMethod_Matches:
		return new CSearchItem(type, text, CSearchItem::eMatches);
	case eAddressMethod_NotMatches:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eMatches));
	default:
		return nil;
	}
}

CSearchItem* CSearchCriteriaSIEVE::ParseText(CSearchItem::ESearchType type) const
{
	cdstring text = mText1->GetText();

	switch(mPopup2->GetValue())
	{
	case eStringMethod_Is:
		return new CSearchItem(type, text, CSearchItem::eIs);
	case eStringMethod_IsNot:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eIs));
	case eStringMethod_Contains:
		return new CSearchItem(type, text, CSearchItem::eContains);
	case eStringMethod_NotContains:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eContains));
	case eStringMethod_StartsWith:
		return new CSearchItem(type, text, CSearchItem::eStartsWith);
	case eStringMethod_NotStartsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eStartsWith));
	case eStringMethod_EndsWith:
		return new CSearchItem(type, text, CSearchItem::eEndsWith);
	case eStringMethod_NotEndsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eEndsWith));
	case eStringMethod_Matches:
		return new CSearchItem(type, text, CSearchItem::eMatches);
	case eStringMethod_NotMatches:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eMatches));
	default:
		return nil;
	}
}

CSearchItem* CSearchCriteriaSIEVE::ParseSize() const
{
	long size = mText3->GetNumber();
	switch(mPopup3->GetValue())
	{
	case eSize_Bytes:
		break;
	case eSize_KBytes:
		size *= 1024L;
		break;
	case eSize_MBytes:
		size *= 1024L * 1024L;
		break;
	}
	return new CSearchItem((mPopup2->GetValue() == eSizeMethod_Larger) ? CSearchItem::eLarger : CSearchItem::eSmaller, size);
}

void CSearchCriteriaSIEVE::SetSearchItem(const CSearchItem* spec, bool negate)
{
	long popup1 = 1;
	long popup2 = 1;
	long popup3 = eSize_Bytes;
	cdstring text1;
	cdstring text2;
	cdstring text3;

	if (spec)
	{
		switch(spec->GetType())
		{
		case CSearchItem::eAll:			// -
			popup1 = eCriteria_All;
			break;

		case CSearchItem::eAnd:			// CSearchItemList*
			break;

		case CSearchItem::eBcc:			// cdstring*
			popup1 = eCriteria_Bcc;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eCC:			// cdstring*
			popup1 = eCriteria_CC;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;
		case CSearchItem::eFrom:		// cdstring*
			popup1 = eCriteria_From;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eGroup:		// CSearchItemList*
			popup1 = eCriteria_Group;
			break;

		case CSearchItem::eHeader:		// cdstrpair*
			popup1 = eCriteria_Header;
			text2 = static_cast<const cdstrpair*>(spec->GetData())->first;
			text1 = static_cast<const cdstrpair*>(spec->GetData())->second;
			break;

		case CSearchItem::eLarger:		// long
			popup1 = eCriteria_Size;
			popup2 = eSizeMethod_Larger;
			long size = reinterpret_cast<long>(spec->GetData());
			if (size >= 1024L * 1024L)
			{
				size /= 1024L * 1024L;
				popup3 = eSize_MBytes;
			}
			else if (size >= 1024L)
			{
				size /= 1024L;
				popup3 = eSize_KBytes;
			}
			text3 = size;
			break;

		case CSearchItem::eNot:			// CSearchItem*
			// Do negated - can only be text based items
			SetSearchItem(static_cast<const CSearchItem*>(spec->GetData()), true);

			// Must exit now without changing any UI item since they have already been done
			return;

		case CSearchItem::eOr:			// CSearchItemList*
			break;

		case CSearchItem::eSmaller:		// long
			popup1 = eCriteria_Size;
			popup2 = eSizeMethod_Smaller;
			size = reinterpret_cast<long>(spec->GetData());
			if (size >= 1024L)
			{
				size /= 1024L;
				popup3 = eSize_KBytes;
			}
			else if (size >= 1024L * 1024L)
			{
				size /= 1024L * 1024L;
				popup3 = eSize_MBytes;
			}
			text3 = size;
			break;

		case CSearchItem::eSubject:		// cdstring*
			popup1 = eCriteria_Subject;
			text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = GetPopup2(spec, negate, false);
			break;

		case CSearchItem::eTo:			// cdstring*
			popup1 = eCriteria_To;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eRecipient:	// cdstring*
			popup1 = eCriteria_Recipient;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eCorrespondent:	// cdstring*
			popup1 = eCriteria_Correspondent;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eSender:	// cdstring*
			popup1 = eCriteria_Sender;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		default:
			break;
		}
	}

	mPopup1->SetValue(popup1);
	mPopup2->SetValue(popup2);
	mPopup3->SetValue(popup3);
	mText1->SetText(text1);
	mText2->SetText(text2);
	mText3->SetText(text3);
	
	// Set group contents
	if ((spec != NULL) && (spec->GetType() == CSearchItem::eGroup))
		mGroupItems->InitGroup(CFilterItem::eSIEVE, spec);
}

long CSearchCriteriaSIEVE::GetPopup2(const CSearchItem* spec, bool negate, bool addr) const
{
	switch(spec->GetMatchType())
	{
	case CSearchItem::eDefault:
	case CSearchItem::eContains:
	default:
		return addr ?
				(negate ? (long)eAddressMethod_NotContains : (long)eAddressMethod_Contains) :
				(negate ? (long)eStringMethod_NotContains : (long)eStringMethod_Contains);
	case CSearchItem::eIs:
		return addr ?
				(negate ? (long)eAddressMethod_IsNot : (long)eAddressMethod_Is) :
				(negate ? (long)eStringMethod_IsNot : (long)eStringMethod_Is);
	case CSearchItem::eStartsWith:
		return addr ?
				(negate ? (long)eAddressMethod_NotStartsWith : (long)eAddressMethod_StartsWith) :
				(negate ? (long)eStringMethod_NotStartsWith : (long)eStringMethod_StartsWith);
	case CSearchItem::eEndsWith:
		return addr ?
				(negate ? (long)eAddressMethod_NotEndsWith : (long)eAddressMethod_EndsWith) :
				(negate ? (long)eStringMethod_NotEndsWith : (long)eStringMethod_EndsWith);
	case CSearchItem::eMatches:
		return addr ?
				(negate ? (long)eAddressMethod_NotMatches : (long)eAddressMethod_Matches) :
				(negate ? (long)eStringMethod_NotMatches : (long)eStringMethod_Matches);
	}
}
