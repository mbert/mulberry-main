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


// CAddressDisplay.h : header file
//

#ifndef __CCALENDARADDRESSDISPLAY__MULBERRY__
#define __CCALENDARADDRESSDISPLAY__MULBERRY__

#include "CTextDisplay.h"

#include "CCalendarAddress.h"

/////////////////////////////////////////////////////////////////////////////
// CCalendarAddressDisplay window

class CTwister;

class CCalendarAddressDisplay : public CTextDisplay
{
public:
	CCalendarAddressDisplay(JXContainer* enclosure,
									const HSizingOption hSizing, const VSizingOption vSizing,
									const JCoordinate x, const JCoordinate y,
									const JCoordinate w, const JCoordinate h);
	virtual ~CCalendarAddressDisplay();

	static void		AllowResolve(bool allow)
						{ sResolve = allow; }

	virtual void OnCreate();

			void	SetTwister(CTwister* twister)
		{ mTwister = twister; }

	virtual CCalendarAddressList*	GetAddresses();

	// Generated message map functions
protected:

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	JBoolean OKToUnfocus();
	void OnExpandAddress();

	void	ResolveAddressList();
	void	ResolveExpandAddress(const cdstring& text, cdstrvect& results, bool resolve, bool expand);
	bool	ExpandAddressText(const cdstring& expand, cdstrvect& results);

	virtual void HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList,
									  const Atom action, const Time time,
									  const JXWidget* source);

private:
	static bool		sResolve;
	CTwister*		mTwister;
	bool			mResolving;
};

/////////////////////////////////////////////////////////////////////////////

#endif
