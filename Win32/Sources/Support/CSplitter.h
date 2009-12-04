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


// CSplitter

// Class that implements a better splitter window

#ifndef __CSPLITTER__MULBERRY__
#define __CSPLITTER__MULBERRY__

class CSplitter : public CSplitterWnd
{
	DECLARE_DYNAMIC(CSplitter)

// Implementation
public:
	CSplitter();
	~CSplitter();

	void CreateViews(bool horiz, CCreateContext* pContext);
	void ShowView(bool view1, bool view2);

	long GetRelativeSplitPos() const;
	void SetRelativeSplitPos(long split);

protected:
	CWnd*			mSub1;
	CWnd*			mSub2;
	mutable long	mDividerPos;
	bool			mHorizontal;
	bool			mVisible1;
	bool			mVisible2;
};

#endif
