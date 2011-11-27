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


// CAddressFieldContainer.h

#ifndef __CAddressFieldContainer__MULBERRY__
#define __CAddressFieldContainer__MULBERRY__

#include "CWndAligner.h"

#include <vector>

// Classes
class CAdbkAddress;
class CAddressFieldBase;
class CAddressFieldSubContainer;

class CAddressFieldContainer : public CWnd, public CWndAligner
{
	friend class CAddressFieldBase;
    friend class CAddressFieldSubContainer;

public:
				CAddressFieldContainer();
	virtual		~CAddressFieldContainer();

	virtual BOOL	Create(const CRect& rect, CWnd* pParentWnd);

	unsigned long	GetCount() const
		{ return mFields.size(); }

	void	SetAddress(const CAdbkAddress* addr);
	bool	GetAddress(CAdbkAddress* addr);

	bool	SetInitialFocus();

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CAddressFieldBase*          AddField(bool multi=false);
	CAddressFieldSubContainer*	AddContainer(int ctype);
	void                		AddView(CWnd* view);

    void                Layout();

private:
    std::vector<CWnd*>		mFields;

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
