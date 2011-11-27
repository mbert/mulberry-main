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


// CAddressFieldSubContainer.h

#ifndef __CAddressFieldSubContainer__MULBERRY__
#define __CAddressFieldSubContainer__MULBERRY__

#include "CWndAligner.h"

#include <vector>

// Classes
class CAdbkAddress;
class CAddressFieldBase;

class CAddressFieldSubContainer : public CWnd, public CWndAligner
{
	friend class CAddressFieldContainer;
	friend class CAddressFieldBase;

public:
    enum EContainerType
    {
        eTel = 0,
        eEmail,
        eAddress,
        eIM,
        eCalAddress,
        eURI
    };

				CAddressFieldSubContainer();
	virtual		~CAddressFieldSubContainer();

	virtual BOOL	Create(const CRect& rect, CWnd* pParentWnd);

	unsigned long	GetCount() const
		{ return mFields.size(); }

	void	SetContainerType(EContainerType ctype)
		{ mType = ctype; }

	void	SetAddress(const CAdbkAddress* addr);
	bool	GetAddress(CAdbkAddress* addr);

	bool	SetInitialFocus();

protected:
	CStatic			mTitle;
	EContainerType	mType;
    bool            mDirty;

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CAddressFieldBase*	AddField(bool multi=false);
	void                AddView(CWnd* view);
	void                RemoveField(CWnd* view);
    void                AppendField();

    void                Layout();

private:
    std::vector<CWnd*> mFields;

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
