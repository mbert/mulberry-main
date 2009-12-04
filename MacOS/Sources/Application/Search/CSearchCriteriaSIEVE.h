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


// Header for CSearchCriteriaSIEVE class

#ifndef __CSEARCHCRITERIASIEVE__MULBERRY__
#define __CSEARCHCRITERIASIEVE__MULBERRY__

#include "CSearchCriteria.h"

// Constants

// Panes
const	PaneIDT		paneid_SearchCriteriaS = 1013;
const	PaneIDT		paneid_SearchCriteriaSPopup1 = 'POP1';
const	PaneIDT		paneid_SearchCriteriaSPopup2 = 'POP2';
const	PaneIDT		paneid_SearchCriteriaSPopup3 = 'POP3';
const	PaneIDT		paneid_SearchCriteriaSPopup4 = 'POP4';
const	PaneIDT		paneid_SearchCriteriaSText1 = 'TXT1';
const	PaneIDT		paneid_SearchCriteriaSText2 = 'TXT2';
const	PaneIDT		paneid_SearchCriteriaSText3 = 'TXT3';
const	PaneIDT		paneid_SearchCriteriaSDate = 'DATE';

// Mesages
const	MessageT	msg_SearchCriteriaSPopup1 = 'POP1';
const	MessageT	msg_SearchCriteriaSPopup2 = 'POP2';

// Resources
const	ResIDT		RidL_CSearchCriteriaSIEVEBtns = 1013;
const	ResIDT		MENU_SIEVEAddressCriteria = 1222;
const	ResIDT		MENU_SIEVEStringCriteria = 1223;
const	ResIDT		MENU_SIEVEHeaderCriteria = 1224;
const	ResIDT		MENU_SIEVESizeCriteria = 1015;

// Classes
class CSearchItem;
class CTextFieldX;
class LPopupButton;

class	CSearchCriteriaSIEVE : public CSearchCriteria
{
private:
	LPopupButton*		mPopup1;
	LPopupButton*		mPopup2;
	LPopupButton*		mPopup3;
	LPopupButton*		mPopup4;
	CTextFieldX*		mText1;
	CTextFieldX*		mText2;
	CTextFieldX*		mText3;

public:
	enum { class_ID = 'SrcS' };

					CSearchCriteriaSIEVE();
					CSearchCriteriaSIEVE(LStream *inStream);
	virtual 		~CSearchCriteriaSIEVE();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

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
	
	CSearchItem* ParseAddress(CSearchItem::ESearchType type) const;
	CSearchItem* ParseText(CSearchItem::ESearchType type) const;
	CSearchItem* ParseSize() const;
	
	long GetPopup2(const CSearchItem* spec, bool negate, bool addr) const;
};

#endif
