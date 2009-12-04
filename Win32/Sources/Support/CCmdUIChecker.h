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


// Header for CCmdUIChecker class

#ifndef __CCMDUICHECKER__MULBERRY__
#define __CCMDUICHECKER__MULBERRY__

class CCmdUIChecker : public CCmdUI
{
public:
	CCmdUIChecker() { mEnabled = false; }
	virtual ~CCmdUIChecker() {}

	bool GetEnabled() const
		{ return mEnabled; }

	virtual void Enable(BOOL bOn = TRUE)
		{ mEnabled = bOn; m_bEnableChanged = TRUE; }

	virtual void SetCheck(int nCheck = 1)
		{ return; }
	virtual void SetRadio(BOOL bOn = TRUE)
		{ return; }
	virtual void SetText(LPCTSTR lpszText)
		{ return; }

protected:
	bool	mEnabled;

};

#endif