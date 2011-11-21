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


// Header for CAddressFieldBase class

#ifndef __CADDRESSFIELD__MULBERRY__
#define __CADDRESSFIELD__MULBERRY__

// Constants

// Panes
const	PaneIDT		paneid_AddressFieldBaseAdd = 'FADD';
const	PaneIDT		paneid_AddressFieldBaseRemove = 'FREM';
const	PaneIDT		paneid_AddressFieldBaseTitle = 'TITL';
const	PaneIDT		paneid_AddressFieldBaseType = 'TYPE';
const	PaneIDT		paneid_AddressFieldBaseMove = 'MOVE';
const	PaneIDT		paneid_AddressFieldBaseData = 'DATA';

// Mesages
const	MessageT	msg_AddressFieldBaseAdd = 'FADD';
const	MessageT	msg_AddressFieldBaseRemove = 'FREM';

// Resources
const   ResIDT      menu_TelephoneType = 9100;
const   ResIDT      menu_GenericType = 9101;

// Classes
class CStaticText;
class LBevelButton;
class LPopupButton;

class CAddressFieldBase : public LView,
							public LListener
{
protected:
	LBevelButton*		mAdd;
	LBevelButton*		mRemove;
    CStaticText*        mTitle;
    LPopupButton*       mType;
    LView*              mDataMove;
    
    bool                mUsesType;
    int                 mOriginalType;
    cdstring            mOriginalData;

public:
					CAddressFieldBase();
					CAddressFieldBase(LStream *inStream);
	virtual 		~CAddressFieldBase();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	void			SetSingleInstance();
	void			SetMultipleInstance();
	void			SetBottom();
	void			HideType();
	void			HideTitle();
    
    virtual void    SetDetails(const cdstring& title, int type, const cdstring& data) = 0;
    virtual bool    GetDetails(int& newtype, cdstring& newdata) = 0;
    void            SetMenu(ResIDT typePopup);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void	OnAdd();
	void	OnRemove();
};

#endif
