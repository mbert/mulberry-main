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


// Header for CCriteriaBase class

#ifndef __CCRITERIABASE__MULBERRY__
#define __CCRITERIABASE__MULBERRY__

#include "CSearchItem.h"

// Constants

// Panes
const	PaneIDT		paneid_CriteriaBaseMoveUp = 'MVUP';
const	PaneIDT		paneid_CriteriaBaseMoveDown = 'MVDN';

// Mesages
const	MessageT	msg_CriteriaBaseMoveUp = 'MVUP';
const	MessageT	msg_CriteriaBaseMoveDown = 'MVDN';

// Resources

// Classes
class LBevelButton;

class CCriteriaBase : public LView,
							public LListener
{
private:
	LBevelButton*		mMoveUp;
	LBevelButton*		mMoveDown;

public:
					CCriteriaBase();
					CCriteriaBase(LStream *inStream);
	virtual 		~CCriteriaBase();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	void			SetTop(bool top);
	void			SetBottom(bool bottom);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void	OnMove(bool up);
	
	virtual void	SwitchWith(CCriteriaBase* other) = 0;
};

#endif
