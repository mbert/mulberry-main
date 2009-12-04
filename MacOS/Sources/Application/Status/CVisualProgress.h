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


// Header for CVisualProgress class

#ifndef __CVISUALPROGRESS__MULBERRY__
#define __CVISUALPROGRESS__MULBERRY__

#include "CProgress.h"

#include "CStaticText.h"

#include <LDialogBox.h>

#include <LProgressBar.h>

// Constants
const	ClassIDT	class_ProgressDialog ='dlog';
const	PaneIDT		paneid_ProgressDialog = 8001;
const	PaneIDT		paneid_ProgressBar = 'PBAR';
const	PaneIDT		paneid_ProgressTitle = 'TITL';

// Classes
class CBarPane : public LProgressBar, public CProgress
{
public:
	enum { class_ID = 'PBar' };

					CBarPane(LStream *inStream);
	virtual			~CBarPane();
	
	virtual void	SetCount(unsigned long count);
	virtual void	SetTotal(unsigned long total);
};

class CBalloonDialog;

class CProgressDialog : public LDialogBox, public CProgress {

private:
	CBarPane*		mBarPane;
	CStaticText*	mTitle;

public:
	enum { class_ID = 'PDlg' };

					CProgressDialog();
					CProgressDialog(LStream *inStream);
	virtual			~CProgressDialog();

	static CBalloonDialog* StartDialog(const cdstring& rsrc);
	static void EndDialog(CBalloonDialog* dlog);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetDescriptor(const cdstring& title)
						{ mTitle->SetText(title); }		// Set title
	virtual void	SetCount(unsigned long count) 
						{ if (mBarPane) mBarPane->SetCount(count); }
	virtual void	SetTotal(unsigned long total) 
						{ if (mBarPane) mBarPane->SetTotal(total); }
	virtual void	SetIndeterminate()
						{ if (mBarPane) mBarPane->SetIndeterminateFlag(true, true); }
	
	virtual void	Redraw(void)
						{ Draw(nil); }						// Redraw - forces immediate update
};

#endif
