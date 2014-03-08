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

#ifndef _H_CINPUTFIELD
#define _H_CINPUTFIELD

#include <JXColormap.h>
#include <JXWidget.h>

class JXContainer;

template <class T> class CInputField : public T {
public:
	CInputField(JXContainer* enclosure,
							const JXWidget::HSizingOption hSizing, 
							const JXWidget::VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
};

template <class T> CInputField<T>::CInputField(JXContainer* enclosure,
												 const JXWidget::HSizingOption hSizing, 
												 const JXWidget::VSizingOption vSizing,
												 const JCoordinate x, const JCoordinate y,
												 const JCoordinate w, const JCoordinate h)
  :T(enclosure, hSizing, vSizing, x, y, w, h)
{
  this->SetBackColor(this->GetColormap()->GetWhiteColor());
  this->ShouldAllowDragAndDrop(kTrue);
}

template <class T> class CMultiInputField : public T {
public:
	CMultiInputField(JXContainer* enclosure,
							const JXWidget::HSizingOption hSizing, 
							const JXWidget::VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
};

template <class T> CMultiInputField<T>::CMultiInputField(JXContainer* enclosure,
												 const JXWidget::HSizingOption hSizing, 
												 const JXWidget::VSizingOption vSizing,
												 const JCoordinate x, const JCoordinate y,
												 const JCoordinate w, const JCoordinate h)
  :T(kFalse, kTrue, enclosure, hSizing, vSizing, x, y, w, h)
{
  this->SetBackColor(this->GetColormap()->GetWhiteColor());
  this->ShouldAllowDragAndDrop(kTrue);
}

#endif
