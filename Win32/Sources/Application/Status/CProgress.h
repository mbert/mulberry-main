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


// Header for CProgress class

#ifndef __CPROGRESS__MULBERRY__
#define __CPROGRESS__MULBERRY__

#include "CMacProgressCtrl.h"

#include "cdstring.h"

// Classes

class CProgress {

protected:
	unsigned long	mPercentage;
	unsigned long	mCount;
	unsigned long	mTotal;
	cdstring		mTitle;

public:
					CProgress();
	virtual			~CProgress() {}

	virtual void	SetDescriptor(cdstring& inDescriptor)
						{ mTitle = inDescriptor; }

	virtual void	Reset(void)
		{ SetPercentage(0); }

	virtual void	SetCount(unsigned long count)
		{ mCount = (count > mTotal) ? mTotal : count; }
	virtual unsigned long	GetCount(void) const
		{ return mCount; }
	virtual void	BumpCount(unsigned long add = 1)
		{ SetCount(mCount + add); }

	virtual void	SetTotal(unsigned long total)
		{ mCount = 0; mTotal = total; }
	virtual unsigned long	GetTotal(void) const
		{ return mTotal; }

	virtual void	SetPercentage(unsigned long percentage) 
		{ mPercentage = (percentage < 0) ? 0 : ((percentage > 100) ? 100 : percentage); }
	virtual unsigned long	GetPercentage(void) const
		{ return mPercentage; }

	virtual void	SetPercentageValue(unsigned long value)
		{ SetPercentage((mTotal != 0) ? (value * 100) / mTotal : 100); }

	virtual void	Redraw() {}
};

class CBarPane : public CMacProgressCtrl, public CProgress
{
public:
					CBarPane() {}
	virtual			~CBarPane() {}

	virtual void	SetCount(unsigned long count);
	virtual void	SetTotal(unsigned long total);
};

#endif
