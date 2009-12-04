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



#ifndef __STPRESERVESELECTION__MULBERRY__
#define __STPRESERVESELECTION__MULBERRY__

template <class T> class StPreserveSelection {
public:
	StPreserveSelection(T* t) {
		mT = t;
		mHasSelection = t->HasSelection();
		if (mHasSelection) {
			t->GetSelection(&mStart, &mEnd);
		} else {
			t->GetCaretLocation(&mStart);
		}
	}

	~StPreserveSelection() {
		if (mHasSelection) {
			mT->SetSelection(mStart, mEnd);
		} else {
			mT->SetCaretLocation(mStart);
		}
	}
private:
	bool mHasSelection;
	JIndex mStart, mEnd;
	T* mT;
};
#endif
