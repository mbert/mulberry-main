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


// Header for CFilter class

#ifndef __CCREATEFILEFILTER__MULBERRY__
#define __CCREATEFILEFILTER__MULBERRY__

#include "CFilter.h"
#include "CFullFileStream.h"

class CCreateFileFilter : public CFilter
{
public:
					CCreateFileFilter(LStream* aStream = NULL, CProgress* progress = NULL);
	virtual			~CCreateFileFilter();

	virtual void	SetStream(LStream* aStream)
						{ mStream = aStream;
						  mFileStream = (CFullFileStream*) aStream;}

protected:
	CFullFileStream*	mFileStream;
	bool				mFileCreated;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FinderInfo			mDecodedInfo;
#else
	long				mDummy;
#endif
	cdstring			mDecodedName;
	unsigned char		mDecodedNameLen;
	bool				mUseRsrc;

	virtual void	CreateFile();

private:
			void	InitCreateFileFilter();
};

#endif
