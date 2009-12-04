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

#include "HPopupMenu.h"

// Constants

// Classes
class CDateControl;
class CSearchItem;
class CTextInputField;

class CSearchCriteriaLocal : public CSearchCriteria
{
public:
					CSearchCriteriaLocal(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CSearchCriteriaLocal();

	virtual void	OnCreate();

	virtual void	SetRules(bool rules);

	virtual long	ShowOrAnd(bool show);
	virtual bool	IsOr() const;
	virtual void	SetOr(bool use_or);

	virtual CSearchItem*	GetSearchItem() const;
	virtual void			SetSearchItem(const CSearchItem* spec, bool negate = false); 
	
protected:
// begin JXLayout1

    JXMultiImageButton* mUp;
    JXMultiImageButton* mDown;
    HPopupMenu*         mPopup4;
    HPopupMenu*         mPopup1;
    HPopupMenu*         mPopup2;
    CTextInputField*    mText1;
    CTextInputField*    mText2;
    CTextInputField*    mText3;
    HPopupMenu*         mPopup3;
    CDateControl*       mDate;
    HPopupMenu*         mPopup5;
    HPopupMenu*         mPopup6;
    HPopupMenu*         mPopup7;
    HPopupMenu*         mPopup8;

// end JXLayout1
	int	mPopup2ID;
	
	virtual void	Receive(JBroadcaster* sender, const Message& message);

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
