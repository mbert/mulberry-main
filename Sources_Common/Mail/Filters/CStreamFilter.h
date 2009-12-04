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


// Header for CStreamFilter class

#ifndef __CSTREAMFILTER__MULBERRY__
#define __CSTREAMFILTER__MULBERRY__

#include <iostream>
#include "filterbuf.h"

// Classes

class CProgress;
class LStream;

class CStreamFilter : public std::iostream
{
public:
	CStreamFilter(filterbuf* filter, std::ostream* sout = NULL, CProgress* progress = NULL) : std::iostream(0)
		{ mFilterBuf = filter; mFilterBuf->SetStream(sout); rdbuf(mFilterBuf); }
	CStreamFilter(filterbuf* filter, LStream* sout, CProgress* progress = NULL) : std::iostream(0)
		{ mFilterBuf = filter; mFilterBuf->SetStream(sout); rdbuf(mFilterBuf); }
	virtual ~CStreamFilter()
		{ if (mFilterBuf) delete mFilterBuf; }

	virtual void	SetStream(std::ostream* sout)
		{ mFilterBuf->SetStream(sout); }
	virtual std::ostream*	GetOStream()
		{ return mFilterBuf->GetOStream(); }
	virtual void	SetBuffer(char* sout, unsigned long size)
		{ mFilterBuf->SetBuffer(sout, size); }

	virtual void	SetProgress(CProgress* progress)
		{ mFilterBuf->SetProgress(progress); }

	unsigned long	GetWritten() const
		{ return mFilterBuf->GetWritten(); }

protected:
	filterbuf* mFilterBuf;
};

#endif
