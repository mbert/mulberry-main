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


// Header for CProgressDialog class

#ifndef __CProgressDialog__MULBERRY__
#define __CProgressDialog__MULBERRY__

#include "CDialogDirector.h"

#include "CProgress.h"

#include "cdstring.h"

// Classes
class CStaticText;
class JXTextButton;

class CProgressDialog : public CDialogDirector, public CProgress
{
public:
					CProgressDialog(JXDirector* supervisor);
	virtual 		~CProgressDialog();

	static CProgressDialog* StartDialog(const cdstring& rsrc);
	static void StopDialog(CProgressDialog* dlog);

	virtual void	SetDescriptor(const cdstring& title);
	virtual void	SetCount(unsigned long count);
	virtual void	SetTotal(unsigned long total);
	virtual void	SetIndeterminate();

protected:
// begin JXLayout

    JXTextButton* mCancelBtn;
    CStaticText*  mTitle;
    CBarPane*     mBarPane;

// end JXLayout

	virtual void OnCreate();
};

#endif
