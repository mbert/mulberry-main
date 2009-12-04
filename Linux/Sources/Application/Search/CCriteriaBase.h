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

#include <JXWidgetSet.h>

#include <vector>

// Constants

// Classes
class CCriteriaBase;
typedef std::vector<CCriteriaBase*> CCriteriaBaseList;
class JXMultiImageButton;

class CCriteriaBase : public JXWidgetSet
{
public:
					CCriteriaBase(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CCriteriaBase();

	void			SetTop(bool top);
	void			SetBottom(bool bottom);

protected:
	JXMultiImageButton*	mMoveUp;
	JXMultiImageButton*	mMoveDown;

			void 	SetBtns(JXMultiImageButton* up, JXMultiImageButton* down);
	virtual void	Receive(JBroadcaster* sender, const Message& message);
			void	OnSetMove(bool up);
	
	virtual CCriteriaBaseList& GetList() = 0;
	virtual void	SwitchWith(CCriteriaBase* other) = 0;
};

#endif
