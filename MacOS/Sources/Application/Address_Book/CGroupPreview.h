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

// Constants
const	PaneIDT		paneid_GroupPreview = 1734;
const	PaneIDT		paneid_GroupPreviewDesc = 'DESC';
const	PaneIDT		paneid_GroupPreviewZoom = 'ZOOM';
const	PaneIDT		paneid_GroupPreviewNickName = 'NICK';
const	PaneIDT		paneid_GroupPreviewGroupName = 'FULL';
const	PaneIDT		paneid_GroupPreviewAddresses = 'ADDS';
const	PaneIDT		paneid_GroupPreviewSort = 'SORT';

// Messages
const	MessageT	msg_GroupPreviewZoom = 'ZOOM';
const	MessageT	msg_GroupPreviewSort = 'SORT';

// Classes
class CGroup;
class CTextFieldX;
class CTextDisplay;
class C3PaneWindow;
class CStaticText;

class CGroupPreview : public CAddressPreviewBase, public LListener
{
public:
	enum { class_ID = 'Gpre' };

					CGroupPreview();
					CGroupPreview(LStream *inStream);
	virtual 		~CGroupPreview();

	virtual void	Close();
	virtual void	Focus();

	void		SetGroup(CGroup* grp);
	void		ClearGroup();
	CGroup*	GetGroup() const
		{ return mGroup; }

protected:
	CStaticText*	mDescriptor;
	C3PaneWindow*	m3PaneWindow;
	CTextFieldX*	mGroupName;
	CTextFieldX*	mNickName;
	CTextDisplay*	mAddresses;
	CGroup*			mGroup;

	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);	// Respond to clicks in the icon buttons

			void	SetFields(const CGroup* grp);			// Set fields in dialog
			bool	GetFields(CGroup* grp);					// Get fields from dialog
			void	SortAddresses();						// Sort addresses in list

			void	OnZoom();
};

#endif
