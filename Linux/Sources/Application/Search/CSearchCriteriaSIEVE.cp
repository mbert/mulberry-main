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
#include "CSearchBase.h"
#include "CSearchCriteriaContainer.h"
#include "CSearchStyle.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include "JXMultiImageButton.h"

#include <cassert>

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
	eCriteria_Group,
	eCriteria_All
};

const char* cSearchCriteriaSIEVE = 
	"From Address %r| To Address %r| CC Address %r| Bcc Address %r| "
	"Recipient %r| Correspondent %r| Sender %r| "
	"Subject %r| Header Field %r| Size %l %r| "
	"Grouped %l %r| All %r";


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
	eAddressMethod_IsMe,
	eAddressMethod_IsNotMe,
	eAddressMethod_Matches,
	eAddressMethod_NotMatches
};

const char* cSearchAddressMethodSIEVE = 
	"Is %r| Is Not %r| "
	"Contains %r| Does not Contain %r| "
	"Starts with %r| Does not Start with %r|"
	"Ends with %r| Does not End with %r %l|"
	"Is Me %r| Is not Me %r %l|"
	"Matches %r| Does not Match %r";

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
	eStringMethod_Matches,
	eStringMethod_NotMatches
};

const char* cSearchStringMethodSIEVE = 
	"Is %r| Is Not %r| "
	"Contains %r| Does not Contain %r| "
	"Starts with %r| Does not Start with %r|"
	"Ends with %r| Does not End with %r %l|"
	"Matches %r| Does not Match %r";

enum
{
	eSizeMethod_Larger = 1,
	eSizeMethod_Smaller
};

const char* cSearchSizeMethodSIEVE = "Is Greater Than %r| Is Less Than %r";

enum
{
	eSize_Bytes = 1,
	eSize_KBytes,
	eSize_MBytes
};

const char* cSearchSizeUnitsSIEVE = "bytes %r| KB %r| MB %r";

enum
{
	eMode_Or = 1,
	eMode_And
};

const char* cSearchModeSIEVE = "or %r| and %r";

enum
{
	ePopup2_AddressContains = 1,
	ePopup2_TextMethod,
	ePopup2_SizeMethod
};

// Default constructor
CSearchCriteriaSIEVE::CSearchCriteriaSIEVE(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CSearchCriteria(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CSearchCriteriaSIEVE::~CSearchCriteriaSIEVE()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSearchCriteriaSIEVE::OnCreate()
{
// begin JXLayout1

    mUp =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,0, 15,10);
    assert( mUp != NULL );

    mDown =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,10, 15,10);
    assert( mDown != NULL );

    mPopup4 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 22,0, 62,20);
    assert( mPopup4 != NULL );

    mPopup1 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 86,0, 122,20);
    assert( mPopup1 != NULL );

    mPopup2 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 213,0, 132,20);
    assert( mPopup2 != NULL );

    mText1 =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 350,0, 140,20);
    assert( mText1 != NULL );

    mText2 =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 213,0, 132,20);
    assert( mText2 != NULL );

    mText3 =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 349,0, 60,20);
    assert( mText3 != NULL );

    mPopup3 =
        new HPopupMenu("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 415,0, 76,20);
    assert( mPopup3 != NULL );

// end JXLayout1

	mUp->SetImage(IDI_MOVEUP);
	mDown->SetImage(IDI_MOVEDOWN);

	mText1->OnCreate();
	mText2->OnCreate();
	mText3->OnCreate();

	mPopup1->SetMenuItems(cSearchCriteriaSIEVE);
	mPopup1->SetValue(eCriteria_From);
	mPopup2->SetMenuItems(cSearchAddressMethodSIEVE);
	mPopup2ID = ePopup2_AddressContains;
	mPopup2->SetValue(eAddressMethod_Contains);
	mPopup3->SetMenuItems(cSearchSizeUnitsSIEVE);
	mPopup3->SetValue(eSize_KBytes);
	mPopup4->SetMenuItems(cSearchModeSIEVE);
	mPopup4->SetValue(eMode_Or);

	// Show/hide items
	mPopup3->Hide();
	mPopup4->Hide();
	mText2->Hide();
	mText3->Hide();

	// Listen to certain items
	ListenTo(mPopup1);
	ListenTo(mPopup2);

	SetBtns(mUp, mDown);
}

// Handle buttons
void CSearchCriteriaSIEVE::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mPopup1)
    	{
			OnSetCriteria(index);
			return;
		}
    	else if (sender == mPopup2)
    	{
			OnSetMethod(index);
			return;
		}
	}

	CSearchCriteria::Receive(sender, message);
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
	switch(item1)
	{
	case eCriteria_From:
	case eCriteria_To:
	case eCriteria_CC:
	case eCriteria_Bcc:
	case eCriteria_Recipient:
	case eCriteria_Correspondent:
	case eCriteria_Sender:
		if (mPopup2ID != ePopup2_AddressContains)
		{
			mPopup2->SetMenuItems(cSearchAddressMethodSIEVE);
			mPopup2ID = ePopup2_AddressContains;
			mPopup2->SetToPopupChoice(kTrue, eAddressMethod_Contains);
		}
		mPopup2->Show();
		mPopup3->Hide();
		mText1->Show();
		mText2->Hide();
		mText3->Hide();
		break;
	case eCriteria_Subject:
		if (mPopup2ID != ePopup2_TextMethod)
		{
			mPopup2->SetMenuItems(cSearchStringMethodSIEVE);
			mPopup2ID = ePopup2_TextMethod;
			mPopup2->SetToPopupChoice(kTrue, eStringMethod_Contains);
		}
		mPopup2->Show();
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
		if (mPopup2ID != ePopup2_SizeMethod)
		{
			mPopup2->SetMenuItems(cSearchSizeMethodSIEVE);
			mPopup2ID = ePopup2_SizeMethod;
			mPopup2->SetToPopupChoice(kTrue, eSizeMethod_Larger);
		}
		mPopup2->Show();
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
	cdstring text = mText3->GetText();
	long size = ::atol(text);
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
	long popup1 = eCriteria_From;
	long popup2 = eAddressMethod_Contains;
	long popup3 = eSize_KBytes;
	cdstring text1;
	cdstring text2;
	cdstring text3;
	long size = 0;

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
			size = reinterpret_cast<long>(spec->GetData());
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
	OnSetCriteria(popup1);
	mPopup2->SetValue(popup2);
	OnSetMethod(popup2);
	mPopup3->SetValue(popup3);
	mText1->SetText(text1.c_str());
	mText2->SetText(text2.c_str());
	mText3->SetText(text3.c_str());
	
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
				(negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
				(negate ? eStringMethod_NotContains : eStringMethod_Contains);
	case CSearchItem::eIs:
		return addr ?
				(negate ? eAddressMethod_IsNot : eAddressMethod_Is) :
				(negate ? eStringMethod_IsNot : eStringMethod_Is);
	case CSearchItem::eStartsWith:
		return addr ?
				(negate ? eAddressMethod_NotStartsWith : eAddressMethod_StartsWith) :
				(negate ? eStringMethod_NotStartsWith : eStringMethod_StartsWith);
	case CSearchItem::eEndsWith:
		return addr ?
				(negate ? eAddressMethod_NotEndsWith : eAddressMethod_EndsWith) :
				(negate ? eStringMethod_NotEndsWith : eStringMethod_EndsWith);
	case CSearchItem::eMatches:
		return addr ?
				(negate ? eAddressMethod_NotMatches : eAddressMethod_Matches) :
				(negate ? eStringMethod_NotMatches : eStringMethod_Matches);
	}
}
