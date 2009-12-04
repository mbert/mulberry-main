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


// Source for CSearchCriteriaLocal class

#include "CSearchCriteriaLocal.h"

#include "CDateControl.h"
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
	eCriteria_DateSent,
	eCriteria_DateReceived,
	eCriteria_Subject,
	eCriteria_Body,
	eCriteria_Header,
	eCriteria_Text,
	eCriteria_Size,
	//eCriteria_Separator,
	eCriteria_Recent,
	eCriteria_Seen,
	eCriteria_Answered,
	eCriteria_Flagged,
	eCriteria_Deleted,
	eCriteria_Draft,
	//eCriteria_Separator,
	eCriteria_Label1,
	eCriteria_Label2,
	eCriteria_Label3,
	eCriteria_Label4,
	eCriteria_Label5,
	eCriteria_Label6,
	eCriteria_Label7,
	eCriteria_Label8,
	//eCriteria_Separator,
	eCriteria_Group,
	//eCriteria_Separator,
	eCriteria_SearchSet,
	//eCriteria_Separator,
	eCriteria_All,
	eCriteria_Selected
};

const char* cSearchCriteria = 
	"From Address %r| To Address %r| CC Address %r| Bcc Address %r| "
	"Recipient %r| Correspondent %r| Sender %r| Date Sent %r| Date Received %r| "
	"Subject %r| Body %r| Header Field %r| Entire Message %r| Size %l %r| "
	"Recent Flag %r| Seen Flag %r| Answered Flag %r| Important Flag %r| "
	"Deleted Flag %r| Draft Flag %l %r| "
	"Label1 %r | Label2 %r | Label3 %r | Label4 %r | Label5 %r | Label6 %r | Label7 %r | Label8 %r %l |"
	"Grouped %r %l| Saved Search %r %l| All %r | Selected Only %r";

enum
{
	eAddressMethod_Contains = 1,
	eAddressMethod_NotContains,
	eAddressMethod_IsMe,
	eAddressMethod_IsNotMe
};

const char* cSearchAddressMethod = 
	"Contains %r| Does not Contain %r| Is Me %r| Is not Me %r";

enum
{
	eDateMethod_Before = 1,
	eDateMethod_On,
	eDateMethod_After,
	eDateMethod_Is,
	eDateMethod_IsNot,
	eDateMethod_IsWithin,
	eDateMethod_IsNotWithin
};

const char* cSearchDateMethod =
	"Before %r| On %r| After %l %r| "
	"is %r| is not %r| is Within %r| is not Within %r";

enum
{
	eDateRelMethod_SentToday = 1,
	eDateRelMethod_SentYesterday,
	eDateRelMethod_SentWeek,
	eDateRelMethod_Sent7Days,
	eDateRelMethod_SentMonth,
	eDateRelMethod_SentYear
};

const char* cSearchDateRelMethod =
	"Today %r| since Yesterday %r| this Week %r| "
	"within 7 Days %r| this Month %r| this Year %r";

enum
{
	eDateWithin_Days = 1,
	eDateWithin_Weeks,
	eDateWithin_Months,
	eDateWithin_Years
};

const char* cSearchDateWithin =
	"Days %r| Weeks %r| Months %r| Years %r";

enum
{
	eTextMethod_Contains = 1,
	eTextMethod_NotContains
};

const char* cSearchTextMethod = "Contains %r| Does not Contain %r";

enum
{
	eSizeMethod_Larger = 1,
	eSizeMethod_Smaller
};

const char* cSearchSizeMethod = "Is Greater Than %r| Is Less Than %r";

enum
{
	eFlagMethod_Set = 1,
	eFlagMethod_NotSet
};

const char* cSearchFlagMethod = "Is Set %r| Is Not Set %r";

enum
{
	eSize_Bytes = 1,
	eSize_KBytes,
	eSize_MBytes
};

const char* cSearchSizeUnits = "bytes %r| KB %r| MB %r";

enum
{
	eSearchSetMethod_Is = 1,
	eSearchSetMethod_IsNot
};

const char* cSearchSetMethod = "Is %r| Is Not %r";

enum
{
	eMode_Or = 1,
	eMode_And
};

const char* cSearchMode = "or %r| and %r";

enum
{
	ePopup2_AddressContains = 1,
	ePopup2_DateMethod,
	ePopup2_TextMethod,
	ePopup2_SizeMethod,
	ePopup2_FlagMethod,
};

// Default constructor
CSearchCriteriaLocal::CSearchCriteriaLocal(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CSearchCriteria(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CSearchCriteriaLocal::~CSearchCriteriaLocal()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSearchCriteriaLocal::OnCreate()
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

    mDate =
        new CDateControl(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 350,0, 140,20);
    assert( mDate != NULL );

    mPopup5 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 348,0, 142,20);
    assert( mPopup5 != NULL );

    mPopup6 =
        new HPopupMenu("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 415,0, 76,20);
    assert( mPopup6 != NULL );

    mPopup7 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 318,0, 172,20);
    assert( mPopup7 != NULL );

    mPopup8 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 212,0, 102,20);
    assert( mPopup8 != NULL );

// end JXLayout1

	mUp->SetImage(IDI_MOVEUP);
	mDown->SetImage(IDI_MOVEDOWN);

	mText1->OnCreate();
	mText2->OnCreate();
	mText3->OnCreate();

	mPopup1->SetMenuItems(cSearchCriteria);
	mPopup1->SetValue(eCriteria_From);
	mPopup2->SetMenuItems(cSearchAddressMethod);
	mPopup2ID = ePopup2_AddressContains;
	mPopup2->SetValue(eAddressMethod_Contains);
	mPopup3->SetMenuItems(cSearchSizeUnits);
	mPopup3->SetValue(eSize_KBytes);
	mPopup4->SetMenuItems(cSearchMode);
	mPopup4->SetValue(eMode_Or);
	mPopup5->SetMenuItems(cSearchDateRelMethod);
	mPopup5->SetValue(eDateRelMethod_SentToday);
	mPopup6->SetMenuItems(cSearchDateWithin);
	mPopup6->SetValue(eDateWithin_Days);
	mPopup8->SetMenuItems(cSearchSetMethod);
	mPopup8->SetValue(eSearchSetMethod_Is);

	// Show/hide items
	mPopup3->Hide();
	mPopup4->Hide();
	mPopup5->Hide();
	mText2->Hide();
	mText3->Hide();
	mDate->Hide();
	mPopup6->Hide();
	mPopup7->Hide();
	mPopup8->Hide();

	InitLabelNames();

	// Listen to certain items
	ListenTo(mPopup1);
	ListenTo(mPopup2);

	SetBtns(mUp, mDown);
}

// Handle buttons
void CSearchCriteriaLocal::Receive(JBroadcaster* sender, const Message& message)
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

void CSearchCriteriaLocal::SetRules(bool rules)
{
	mRules = rules;
	
	// Remove select item from popup if not rules
	if (!mRules)
	{
		mPopup1->RemoveItem(eCriteria_Selected);
	}
}

long CSearchCriteriaLocal::ShowOrAnd(bool show)
{
	if (show)
		mPopup4->Show();
	else
		mPopup4->Hide();
	
	return 0;
}

bool CSearchCriteriaLocal::IsOr() const
{
	return (mPopup4->GetValue() == eMode_Or);
}

void CSearchCriteriaLocal::SetOr(bool use_or)
{
	mPopup4->SetValue(use_or ? eMode_Or : eMode_And);
}

void CSearchCriteriaLocal::OnSetCriteria(long item1)
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
		if (mPopup2ID != ePopup2_AddressContains)
		{
			mPopup2->SetMenuItems(cSearchAddressMethod);
			mPopup2ID = ePopup2_AddressContains;
			mPopup2->SetToPopupChoice(kTrue, eAddressMethod_Contains);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Show();
		mText2->Hide();
		mText3->Hide();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		mPopup7->Hide();
		mPopup8->Hide();
		break;
	case eCriteria_DateSent:
	case eCriteria_DateReceived:
		if (mPopup2ID != ePopup2_DateMethod)
		{
			mPopup2->SetMenuItems(cSearchDateMethod);
			mPopup2ID = ePopup2_DateMethod;
			mPopup2->SetToPopupChoice(kTrue, eDateMethod_Before);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Hide();
		mText2->Hide();
		mText3->Hide();
		mDate->Show();
		mPopup5->Hide();
		mPopup6->Hide();
		mPopup7->Hide();
		mPopup8->Hide();
		break;
	case eCriteria_Subject:
	case eCriteria_Body:
	case eCriteria_Text:
		if (mPopup2ID != ePopup2_TextMethod)
		{
			mPopup2->SetMenuItems(cSearchTextMethod);
			mPopup2ID = ePopup2_TextMethod;
			mPopup2->SetToPopupChoice(kTrue, eTextMethod_Contains);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Show();
		mText2->Hide();
		mText3->Hide();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		mPopup7->Hide();
		mPopup8->Hide();
		break;
	case eCriteria_Header:
		mText2->SetText(cdstring::null_str);
		mPopup2->Hide();
		mPopup3->Hide();
		mText1->Show();
		mText2->Show();
		mText3->Hide();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		mPopup7->Hide();
		mPopup8->Hide();
		break;
	case eCriteria_Size:
		if (mPopup2ID != ePopup2_SizeMethod)
		{
			mPopup2->SetMenuItems(cSearchSizeMethod);
			mPopup2ID = ePopup2_SizeMethod;
			mPopup2->SetToPopupChoice(kTrue, eSizeMethod_Larger);
		}
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Show();
		mText1->Hide();
		mText2->Hide();
		mText3->Show();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		mPopup7->Hide();
		mPopup8->Hide();
		break;
	case eCriteria_Recent:
	case eCriteria_Seen:
	case eCriteria_Answered:
	case eCriteria_Flagged:
	case eCriteria_Deleted:
	case eCriteria_Draft:
	case eCriteria_Label1:
	case eCriteria_Label2:
	case eCriteria_Label3:
	case eCriteria_Label4:
	case eCriteria_Label5:
	case eCriteria_Label6:
	case eCriteria_Label7:
	case eCriteria_Label8:
		if (mPopup2ID != ePopup2_FlagMethod)
		{
			mPopup2->SetMenuItems(cSearchFlagMethod);
			mPopup2ID = ePopup2_FlagMethod;
			mPopup2->SetToPopupChoice(kTrue, eFlagMethod_Set);
		}
		mText1->SetText(cdstring::null_str);
		mPopup2->Show();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Hide();
		mText2->Hide();
		mText3->Hide();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		mPopup7->Hide();
		mPopup8->Hide();
		break;
	case eCriteria_SearchSet:
		mPopup2->Hide();
		method_refresh = true;
		mPopup3->Hide();
		mText1->Hide();
		mText2->Hide();
		mText3->Hide();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		InitSearchSets();
		mPopup7->Show();
		mPopup8->Show();
		break;
	case eCriteria_Group:
	case eCriteria_All:
	case eCriteria_Selected:
		mPopup2->Hide();
		mPopup3->Hide();
		mText1->Hide();
		mText2->Hide();
		mText3->Hide();
		mDate->Hide();
		mPopup5->Hide();
		mPopup6->Hide();
		break;
	}

	// Special for group display
	if (item1 == eCriteria_Group)
		MakeGroup(CFilterItem::eLocal);
	else
		RemoveGroup();

	// Refresh method for new criteria
	if (method_refresh)
		OnSetMethod(mPopup2->GetValue());
}

void CSearchCriteriaLocal::OnSetMethod(long item)
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
		case eAddressMethod_Contains:
		case eAddressMethod_NotContains:
			mText1->Show();
			break;
		case eAddressMethod_IsMe:
		case eAddressMethod_IsNotMe:
			mText1->Hide();
			mText1->SetText(cdstring::null_str);
			break;
		}
		break;
	case eCriteria_DateSent:
	case eCriteria_DateReceived:
		switch(item)
		{
		case eDateMethod_Before:
		case eDateMethod_On:
		case eDateMethod_After:
			mText3->Hide();
			mDate->Show();
			mPopup5->Hide();
			mPopup6->Hide();
			break;
		case eDateMethod_Is:
		case eDateMethod_IsNot:
			mText3->Hide();
			mDate->Hide();
			mPopup5->Show();
			mPopup6->Hide();
			break;
		case eDateMethod_IsWithin:
		case eDateMethod_IsNotWithin:
			mText3->Show();
			mDate->Hide();
			mPopup5->Hide();
			mPopup6->Show();
			break;
		}
		break;
	default:
		break;
	}
}

void CSearchCriteriaLocal::InitLabelNames()
{
	// Change name of labels
	for(short i = eCriteria_Label1; i < eCriteria_Label1 + NMessage::eMaxLabels; i++)
	{
		cdstring labelName(CPreferences::sPrefs->mLabels.GetValue()[i - eCriteria_Label1]->name);
		mPopup1->SetItemText(i, labelName);
	}

}

void CSearchCriteriaLocal::InitSearchSets()
{
	// Remove any existing items from main menu
	mPopup7->RemoveAllItems();
	
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		mPopup7->AppendItem((*iter)->GetName(), kFalse, kTrue);

	// Set to first value
	mPopup7->SetToPopupChoice(kTrue, 1);
}

CSearchItem* CSearchCriteriaLocal::GetSearchItem() const
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
	case eCriteria_DateSent:
		return ParseDate(true);
	case eCriteria_DateReceived:
		return ParseDate(false);
	case eCriteria_Subject:
		return ParseText(CSearchItem::eSubject);
	case eCriteria_Body:
		return ParseText(CSearchItem::eBody);
	case eCriteria_Text:
		return ParseText(CSearchItem::eText);
	case eCriteria_Header:
		{
			cdstring text1 = mText1->GetText();

			cdstring text2 = mText2->GetText();
			text2.trimspace();

			// Strip trailing colon from header field
			if (text2.compare_end(":"))
				text2[text2.length() - 1] = 0;

			// Look for '!' at start of header field as negate item
			if (text2[0UL] == '!')
				return new CSearchItem(CSearchItem::eNot, new CSearchItem(CSearchItem::eHeader, text2.c_str() + 1, text1));
			else
				return new CSearchItem(CSearchItem::eHeader, text2, text1);
		}
	case eCriteria_Size:
		return ParseSize();
	case eCriteria_Recent:
		return ParseFlag(CSearchItem::eRecent, CSearchItem::eOld);
	case eCriteria_Seen:
		return ParseFlag(CSearchItem::eSeen, CSearchItem::eUnseen);
	case eCriteria_Answered:
		return ParseFlag(CSearchItem::eAnswered, CSearchItem::eUnanswered);
	case eCriteria_Flagged:
		return ParseFlag(CSearchItem::eFlagged, CSearchItem::eUnflagged);
	case eCriteria_Deleted:
		return ParseFlag(CSearchItem::eDeleted, CSearchItem::eUndeleted);
	case eCriteria_Draft:
		return ParseFlag(CSearchItem::eDraft, CSearchItem::eUndraft);
	case eCriteria_Label1:
	case eCriteria_Label2:
	case eCriteria_Label3:
	case eCriteria_Label4:
	case eCriteria_Label5:
	case eCriteria_Label6:
	case eCriteria_Label7:
	case eCriteria_Label8:
		return ParseLabel(CSearchItem::eLabel, mPopup1->GetValue() - eCriteria_Label1);
	case eCriteria_SearchSet:
		{
			cdstring style = mPopup7->GetCurrentItemText().GetCString();
			
			const CSearchItem* found = CPreferences::sPrefs->mSearchStyles.GetValue().FindStyle(style)->GetSearchItem();
			
			// May need to negate
			if (mPopup8->GetValue() == eSearchSetMethod_Is)
				return (found ? new CSearchItem(CSearchItem::eNamedStyle, style) : NULL);
			else
				return (found ? new CSearchItem(CSearchItem::eNot, new CSearchItem(CSearchItem::eNamedStyle, style)) : NULL);
		}
	case eCriteria_Group:
		return mGroupItems->ConstructSearch();
	case eCriteria_All:
		return new CSearchItem(CSearchItem::eAll);
	case eCriteria_Selected:
		return new CSearchItem(CSearchItem::eSelected);
	default:
		return NULL;
	}
}

CSearchItem* CSearchCriteriaLocal::ParseAddress(CSearchItem::ESearchType type) const
{
	cdstring text = mText1->GetText();

	switch(mPopup2->GetValue())
	{
	case eAddressMethod_Contains:
		return new CSearchItem(type, text);
	case eAddressMethod_NotContains:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text));
	case eAddressMethod_IsMe:
		return new CSearchItem(type);
	case eAddressMethod_IsNotMe:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type));
	default:
		return NULL;
	}
}

CSearchItem* CSearchCriteriaLocal::ParseDate(bool sent) const
{
	switch(mPopup2->GetValue())
	{
	case eDateMethod_Before:
		return new CSearchItem(sent ? CSearchItem::eSentBefore : CSearchItem::eBefore, mDate->GetDate());
	case eDateMethod_On:
		return new CSearchItem(sent ? CSearchItem::eSentOn : CSearchItem::eOn, mDate->GetDate());
	case eDateMethod_After:
		return new CSearchItem(sent ? CSearchItem::eSentSince : CSearchItem::eSince, mDate->GetDate());

	// Look at relative date popup
	case eDateMethod_Is:
	case eDateMethod_IsNot:
	{
		bool is = (mPopup2->GetValue() == eDateMethod_Is);

		// Set up types for different categories
		CSearchItem::ESearchType typeToday = (sent ? CSearchItem::eSentOn : CSearchItem::eOn);
		CSearchItem::ESearchType typeOther = (sent ? CSearchItem::eSentSince : CSearchItem::eSince);
		CSearchItem::ESearchType typeNot = (sent ? CSearchItem::eSentBefore : CSearchItem::eBefore);

		// Look at menu item chosen
		switch(mPopup5->GetValue())
		{
		case eDateRelMethod_SentToday:
			return new CSearchItem(is ? typeToday : typeNot, static_cast<unsigned long>(CSearchItem::eToday));
		case eDateRelMethod_SentYesterday:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eSinceYesterday));
		case eDateRelMethod_SentWeek:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eThisWeek));
		case eDateRelMethod_Sent7Days:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eWithin7Days));
		case eDateRelMethod_SentMonth:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eThisMonth));
		case eDateRelMethod_SentYear:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eThisYear));
		default:
			return NULL;
		}
		break;
	}

	// Look at relative date popup
	case eDateMethod_IsWithin:
	case eDateMethod_IsNotWithin:
	{
		bool is = (mPopup2->GetValue() == eDateMethod_IsWithin);

		// Set up types for different categories
		CSearchItem::ESearchType typeIs = (sent ? CSearchItem::eSentSince : CSearchItem::eSince);
		CSearchItem::ESearchType typeIsNot = (sent ? CSearchItem::eSentBefore : CSearchItem::eBefore);

		// Get numeric value
		cdstring text = mText3->GetText();
		long size = ::atol(text);
		unsigned long within = (size > 0) ? size : 1;
		if (within > 0x0000FFFF)
			within = 0x0000FFFF;

		// Look at menu item chosen
		switch(mPopup6->GetValue())
		{
		case eDateWithin_Days:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinDays) | within);
		case eDateWithin_Weeks:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinWeeks) | within);
		case eDateWithin_Months:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinMonths) | within);
		case eDateWithin_Years:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinYears) | within);
		default:
			return NULL;
		}
		break;
	}

	default:
		return NULL;
	}
}

CSearchItem* CSearchCriteriaLocal::ParseText(CSearchItem::ESearchType type) const
{
	cdstring text = mText1->GetText();

	if (mPopup2->GetValue() == eTextMethod_Contains)
		return new CSearchItem(type, text);
	else
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text));
}

CSearchItem* CSearchCriteriaLocal::ParseSize() const
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

CSearchItem* CSearchCriteriaLocal::ParseFlag(CSearchItem::ESearchType type1, CSearchItem::ESearchType type2) const
{
	return new CSearchItem((mPopup2->GetValue() == eFlagMethod_Set) ? type1 : type2);
}

CSearchItem* CSearchCriteriaLocal::ParseLabel(CSearchItem::ESearchType type, unsigned long index) const
{
	if (mPopup2->GetValue() == eFlagMethod_Set)
		return new CSearchItem(type, index);
	else
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, index));
}

void CSearchCriteriaLocal::SetSearchItem(const CSearchItem* spec, bool negate)
{
	long popup1 = eCriteria_From;
	long popup2 = eAddressMethod_Contains;
	long popup3 = eSize_KBytes;
	long popup5 = eDateRelMethod_SentToday;
	long popup6 = eDateWithin_Days;
	long popup7 = 1;
	long popup8 = eSearchSetMethod_Is;
	cdstring text1;
	cdstring text2;
	cdstring text3;
	time_t date = ::time(NULL);
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

		case CSearchItem::eAnswered:	// -
			popup1 = eCriteria_Answered;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eBcc:			// cdstring*
			popup1 = eCriteria_Bcc;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eBefore:		// date
			popup1 = eCriteria_DateReceived;
			popup2 = GetDatePopup(spec, eDateMethod_Before, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eBody:			// cdstring*
			popup1 = eCriteria_Body;
			text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = negate ? eTextMethod_NotContains : eTextMethod_Contains;
			break;

		case CSearchItem::eCC:			// cdstring*
			popup1 = eCriteria_CC;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eDeleted:		// -
			popup1 = eCriteria_Deleted;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eDraft:		// -
			popup1 = eCriteria_Draft;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eFlagged:		// -
			popup1 = eCriteria_Flagged;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eFrom:		// cdstring*
			popup1 = eCriteria_From;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eGroup:		// CSearchItemList*
			popup1 = eCriteria_Group;
			break;

		case CSearchItem::eHeader:		// cdstrpair*
			popup1 = eCriteria_Header;
			if (negate)
				text2 = "!";
			text2 += static_cast<const cdstrpair*>(spec->GetData())->first;
			text1 = static_cast<const cdstrpair*>(spec->GetData())->second;
			break;

		case CSearchItem::eKeyword:		// cdstring*
			break;

		case CSearchItem::eLabel:		// unsigned long
		{
			unsigned long index = reinterpret_cast<unsigned long>(spec->GetData());
			if (index >= NMessage::eMaxLabels)
				index = 0;
			popup1 = eCriteria_Label1 + index;
			popup2 = negate ? eFlagMethod_NotSet : eFlagMethod_Set;
			break;
		}

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

		case CSearchItem::eNew:			// -
			break;

		case CSearchItem::eNot:			// CSearchItem*
			// Do negated - can only be text based items
			SetSearchItem(static_cast<const CSearchItem*>(spec->GetData()), true);
			
			// Must exit now without changing any UI item since they have already been done
			return;

		case CSearchItem::eNumber:		// ulvector* only - no key
			break;

		case CSearchItem::eOld:			// -
			popup1 = eCriteria_Recent;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eOn:			// date
			popup1 = eCriteria_DateReceived;
			popup2 = GetDatePopup(spec, eDateMethod_On, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eOr:			// CSearchItemList*
			break;

		case CSearchItem::eRecent:		// -
			popup1 = eCriteria_Recent;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eSeen:		// -
			popup1 = eCriteria_Seen;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eSender:		// cdstring*
			popup1 = eCriteria_Sender;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eSelected:	// -
			popup1 = eCriteria_Selected;
			break;

		case CSearchItem::eSentBefore:	// date
			popup1 = eCriteria_DateSent;
			popup2 = GetDatePopup(spec, eDateMethod_Before, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSentOn:		// date
			popup1 = eCriteria_DateSent;
			popup2 = GetDatePopup(spec, eDateMethod_On, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSentSince:	// date
			popup1 = eCriteria_DateSent;
			popup2 = GetDatePopup(spec, eDateMethod_After, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSince:		// date
			popup1 = eCriteria_DateReceived;
			popup2 = GetDatePopup(spec, eDateMethod_After, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
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
			popup2 = negate ? eTextMethod_NotContains : eTextMethod_Contains;
			break;

		case CSearchItem::eText:			// cdstring*
			popup1 = eCriteria_Text;
			text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = negate ? eTextMethod_NotContains : eTextMethod_Contains;
			break;

		case CSearchItem::eTo:			// cdstring*
			popup1 = eCriteria_To;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eUID:			// ulvector*
			break;

		case CSearchItem::eUnanswered:	// -
			popup1 = eCriteria_Answered;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUndeleted:	// -
			popup1 = eCriteria_Deleted;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUndraft:		// -
			popup1 = eCriteria_Draft;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUnflagged:	// -
			popup1 = eCriteria_Flagged;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUnkeyword:	// cdstring*
			break;

		case CSearchItem::eUnseen:		// -
			popup1 = eCriteria_Seen;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eRecipient:		// cdstring*
			popup1 = eCriteria_Recipient;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eCorrespondent:	// cdstring*
			popup1 = eCriteria_Correspondent;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eNamedStyle:	// -
			popup1 = eCriteria_SearchSet;
			popup7 = CPreferences::sPrefs->mSearchStyles.GetValue().FindIndexOf(*static_cast<const cdstring*>(spec->GetData())) + 1;
			popup8 = negate ? eSearchSetMethod_IsNot : eSearchSetMethod_Is;
			break;
		default:;
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
	mDate->SetDate(date);
	mPopup5->SetValue(popup5);
	mPopup6->SetValue(popup6);
	mPopup7->SetValue(popup7);
	mPopup8->SetValue(popup8);
	
	// Set group contents
	if ((spec != NULL) && (spec->GetType() == CSearchItem::eGroup))
		mGroupItems->InitGroup(CFilterItem::eLocal, spec);
}

long CSearchCriteriaLocal::GetDatePopup(const CSearchItem* spec, long original, cdstring& text1, cdstring& text3, long& popup5, long& popup6) const
{
	switch(reinterpret_cast<unsigned long>(spec->GetData()))
	{
	// Relative date
	case CSearchItem::eToday:
	case CSearchItem::eSinceYesterday:
	case CSearchItem::eThisWeek:
	case CSearchItem::eWithin7Days:
	case CSearchItem::eThisMonth:
	case CSearchItem::eThisYear:
	{
		// Set relative popup
		switch(reinterpret_cast<unsigned long>(spec->GetData()))
		{
		case CSearchItem::eToday:
		default:
			popup5 = eDateRelMethod_SentToday;
			return (original == eDateMethod_On) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eSinceYesterday:
			popup5 = eDateRelMethod_SentYesterday;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eThisWeek:
			popup5 = eDateRelMethod_SentWeek;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eWithin7Days:
			popup5 = eDateRelMethod_Sent7Days;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eThisMonth:
			popup5 = eDateRelMethod_SentMonth;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eThisYear:
			popup5 = eDateRelMethod_SentYear;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		}
	}
	
	default:;
	}
	
	switch(reinterpret_cast<unsigned long>(spec->GetData()) & CSearchItem::eWithinMask)
	{
	// Relative date
	case CSearchItem::eWithinDays:
	case CSearchItem::eWithinWeeks:
	case CSearchItem::eWithinMonths:
	case CSearchItem::eWithinYears:
	{
		unsigned long within = reinterpret_cast<unsigned long>(spec->GetData()) & CSearchItem::eWithinValueMask;

		// Set relative popup
		switch(reinterpret_cast<unsigned long>(spec->GetData()) & CSearchItem::eWithinMask)
		{
		case CSearchItem::eWithinDays:
		default:
			popup6 = eDateWithin_Days;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		case CSearchItem::eWithinWeeks:
			popup6 = eDateWithin_Weeks;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		case CSearchItem::eWithinMonths:
			popup6 = eDateWithin_Months;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		case CSearchItem::eWithinYears:
			popup6 = eDateWithin_Years;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		}
	}
	
	default:;
	}
	
	// Standard date
	text1 = spec->GenerateDate(false);
	return original;
}
