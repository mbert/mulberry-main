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


// Header for CAddressViewOptions class

#ifndef __CADDRESSVIEWOPTIONS__MULBERRY__
#define __CADDRESSVIEWOPTIONS__MULBERRY__

#include "CPreferenceItem.h"

// Classes

class CAddressViewOptions : public CPreferenceItem
{
public:
	CAddressViewOptions();
	CAddressViewOptions(const CAddressViewOptions& copy)
		{ _copy(copy); }
	virtual ~CAddressViewOptions();

	CAddressViewOptions& operator=(const CAddressViewOptions& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CAddressViewOptions& comp) const;					// Compare with same type

	bool	GetShowPreview() const
		{ return mShowPreview; }
	void	SetShowPreview(bool show)
		{ mShowPreview = show; }

	bool	GetSelectAddressPane() const
		{ return mSelectAddressPane; }
	void	SetSelectAddressPane(bool select)
		{ mSelectAddressPane = select; }

	virtual cdstring GetInfo() const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	bool				mShowPreview;				// Show preview in separate window mode
	bool				mSelectAddressPane;			// Select address pane when previewing

private:
	void _copy(const CAddressViewOptions& copy);
};

#endif
