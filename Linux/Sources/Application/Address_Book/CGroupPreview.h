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


// Header for CGroupPreview class

#ifndef __CGROUPPREVIEW__MULBERRY__
#define __CGROUPPREVIEW__MULBERRY__

#include "CAddressPreviewBase.h"

// Classes
class CGroup;
class C3PaneWindow;
class CStaticText;
class CTextInputField;
class CTextInputDisplay;
class CToolbarButton;
class JXTextButton;

class CGroupPreview : public CAddressPreviewBase
{
public:
					CGroupPreview(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CGroupPreview();

			void	OnCreate();

	virtual void	Close();
	virtual void	Focus();

	void		SetGroup(CGroup* grp);
	void		ClearGroup();
	CGroup*	GetGroup() const
		{ return mGroup; }

protected:
// begin JXLayout1

    CStaticText*       mDescriptor;
    CToolbarButton*    mZoomBtn;
    CTextInputField*   mGroupName;
    CTextInputField*   mNickName;
    CTextInputDisplay* mAddressList;
    JXTextButton*      mSortBtn;

// end JXLayout1
	CGroup*			mGroup;
	C3PaneWindow*	m3PaneWindow;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	SetFields(const CGroup* grp);			// Set fields in dialog
			bool	GetFields(CGroup* grp);					// Get fields from dialog
			void	SortAddresses();						// Sort addresses in list

			void	OnZoom();
};

#endif
