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


// Header for CSearchCriteriaLocal class

#ifndef __CSEARCHCRITERIALOCAL__MULBERRY__
#define __CSEARCHCRITERIALOCAL__MULBERRY__

#include "CSearchCriteria.h"

// Constants

// Panes
const	PaneIDT		paneid_SearchCriteria = 1011;
const	PaneIDT		paneid_SearchCriteriaPopup1 = 'POP1';
const	PaneIDT		paneid_SearchCriteriaPopup2 = 'POP2';
const	PaneIDT		paneid_SearchCriteriaPopup3 = 'POP3';
const	PaneIDT		paneid_SearchCriteriaPopup4 = 'POP4';
const	PaneIDT		paneid_SearchCriteriaPopup5 = 'POP5';
const	PaneIDT		paneid_SearchCriteriaPopup6 = 'POP6';
const	PaneIDT		paneid_SearchCriteriaPopup7 = 'POP7';
const	PaneIDT		paneid_SearchCriteriaPopup8 = 'POP8';
const	PaneIDT		paneid_SearchCriteriaText1 = 'TXT1';
const	PaneIDT		paneid_SearchCriteriaText2 = 'TXT2';
const	PaneIDT		paneid_SearchCriteriaText3 = 'TXT3';
const	PaneIDT		paneid_SearchCriteriaDate = 'DATE';

// Mesages
const	MessageT	msg_SearchCriteriaPopup1 = 'POP1';
const	MessageT	msg_SearchCriteriaPopup2 = 'POP2';

// Resources
const	ResIDT		RidL_CSearchCriteriaLocalBtns = 1011;
const	ResIDT		MENU_SearchAddressCriteria = 1012;
const	ResIDT		MENU_SearchDateCriteria = 1013;
const	ResIDT		MENU_SearchTestCriteria = 1014;
const	ResIDT		MENU_SearchSizeCriteria = 1015;
const	ResIDT		MENU_SearchFlagCriteria = 1016;
const	ResIDT		MENU_SearchSetCriteria = 1019;
const	ResIDT		MENU_SearchDateRelCriteria = 1021;
const	ResIDT		MENU_SearchDateWithin = 1022;

// Classes
class CDateControl;
class CSearchItem;
class CTextFieldX;
class LPopupButton;

class	CSearchCriteriaLocal : public CSearchCriteria
{
private:
	LPopupButton*		mPopup1;
	LPopupButton*		mPopup2;
	LPopupButton*		mPopup3;
	LPopupButton*		mPopup4;
	LPopupButton*		mPopup5;
	LPopupButton*		mPopup6;
	LPopupButton*		mPopup7;
	LPopupButton*		mPopup8;
	CTextFieldX*		mText1;
	CTextFieldX*		mText2;
	CTextFieldX*		mText3;
	CDateControl*		mDate;

public:
	enum { class_ID = 'SrcC' };

					CSearchCriteriaLocal();
					CSearchCriteriaLocal(LStream *inStream);
	virtual 		~CSearchCriteriaLocal();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetRules(bool rules);

	virtual long			ShowOrAnd(bool show);
	virtual bool			IsOr() const;
	virtual void			SetOr(bool use_or);

	virtual CSearchItem*	GetSearchItem() const;
	virtual void			SetSearchItem(const CSearchItem* spec, bool negate = false); 

	virtual bool			DoActivate();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void	OnSetCriteria(long item);
	void	OnSetMethod(long item);
	
	void	InitLabelNames();
	void	InitSearchSets();

	CSearchItem* ParseAddress(CSearchItem::ESearchType type) const;
	CSearchItem* ParseDate(bool sent) const;
	CSearchItem* ParseText(CSearchItem::ESearchType type) const;
	CSearchItem* ParseSize() const;
	CSearchItem* ParseFlag(CSearchItem::ESearchType type1, CSearchItem::ESearchType type2) const;
	CSearchItem* ParseLabel(CSearchItem::ESearchType type, unsigned long index) const;
	
	long GetDatePopup(const CSearchItem* spec, long original, cdstring& text1, cdstring& text3, long& popup5, long& popup6) const;
};

#endif
