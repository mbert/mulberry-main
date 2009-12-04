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


// Header for C3PaneMessage class

#ifndef __C3PANEMESSAGE__MULBERRY__
#define __C3PANEMESSAGE__MULBERRY__

#include "C3PaneViewPanel.h"

// Constants
const	PaneIDT		paneid_3PaneMessage = 1731;
const	PaneIDT		paneid_3PaneMessageZoom = 'ZOOM';

// Messages
const	MessageT	msg_3PaneMessageZoom = 'ZOOM';

// Resources

// Classes
class CMessageView;
class CMessage;

class C3PaneMessage : public C3PaneViewPanel, public LListener
{
public:
	enum { class_ID = '3PNm' };

					C3PaneMessage();
					C3PaneMessage(LStream *inStream);
	virtual 		~C3PaneMessage();

	virtual bool	TestClose();

	virtual bool	IsSpecified() const;

	virtual CBaseView*	GetBaseView() const;
	CMessageView*	GetMessageView() const
		{ return mMessageView; }

	void SetMessage(CMessage* msg);

	virtual cdstring GetTitle() const;
	virtual ResIDT	GetIconID() const;

protected:
	CMessageView*	mMessageView;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

			void	OnZoom();
};

#endif
