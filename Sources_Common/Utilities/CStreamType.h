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

// CStreamType.h

#ifndef __CSTREAMTYPE__MULBERRY__
#define __CSTREAMTYPE__MULBERRY__

#include "CStreamTypeFwd.h"

template <class streamT> class CStreamType
{
public:
	CStreamType(streamT* stream, EEndl endlt)
		{ mStream = stream; mEndl = endlt; }
	~CStreamType() {}
	
	streamT& Stream()
		{ return *mStream; }
	streamT* GetStream()
		{ return mStream; }
	
	EEndl GetEndlType() const
		{ return mEndl; }

	const char* endl() const
		{ return get_endl(mEndl); }

	unsigned long endl_len() const
		{ return get_endl_len(mEndl); }

	bool IsSameType(EEndl comp) const
		{ return (mEndl != eEndl_Any) && ((mEndl == eEndl_Auto) ? lendl : mEndl) == ((comp == eEndl_Auto) ? lendl : comp); }

	bool IsLocalType() const
		{ return IsSameType(lendl); }
	bool IsNetworkType() const
		{ return IsSameType(eEndl_CRLF); }

private:
	streamT*	mStream;
	EEndl		mEndl;
};

#endif
