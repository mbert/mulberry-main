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


// Header for CProgressBar class

#ifndef __CPROGRESSBAR__MULBERRY__
#define __CPROGRESSBAR__MULBERRY__

#include <JXProgressIndicator.h>

#include <time.h>

class JXWindowDirector;

class CProgressBar : public JXProgressIndicator
{
public:

	CProgressBar(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);

	void	SetIndeterminate(bool indeter)
		{ mIndeterminate = indeter; }
	bool	IsIndeterminate() const
		{ return mIndeterminate; }

	void	Start();
	void	SpendTime();

protected:
	bool			mIndeterminate;
	Time			mLastTime;
	unsigned long	mCtr;

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);
};

#endif
