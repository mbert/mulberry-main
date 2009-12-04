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


// CSubPanelController

// Class that implements a tab control and manages its panels as well

#ifndef __CSUBPANELCONTROLLER__MULBERRY__
#define __CSUBPANELCONTROLLER__MULBERRY__

class CTabPanel;

class CSubPanelController : public CWnd
{
public:
	CSubPanelController();
	~CSubPanelController();

	// Add/removing pages
	virtual unsigned long AddPanel(CTabPanel* aPanel);
	virtual void RemovePanel(CTabPanel* aPanel);
	virtual void RemovePanel(unsigned long index);

	virtual long GetCurrentIndex() const
		{ return mCurrentIndex; }
	virtual unsigned long GetPanelCount() const
		{ return mPanels.size(); }
	virtual CTabPanel* GetCurrentPanel()
		{ return mCurrentPanel; }

	// Managing data in panels
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void	SetContent(void* data);						// Set data
	virtual void	UpdateContent(void* data);					// Force update of data
	virtual void	SetPanelContent(void* data);				// Set data
	virtual void	UpdatePanelContent(void* data);				// Force update of data

	// Switching between panels
	virtual void	SetPanel(long index);						// Force update of display panel

protected:
	typedef vector<CTabPanel*> CTabPanelList;
	CTabPanelList mPanels;
	CTabPanel* mCurrentPanel;
	long mCurrentIndex;
	
	// Panel management
	virtual void	InstallPanel(CTabPanel* aPanel);
	virtual void	DestroyPanel(CTabPanel* aPanel);
};

#endif
