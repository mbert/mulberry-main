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


// Header for CFileOptionsMap class

#ifndef __CFILEOPTIONSMAP__MULBERRY__
#define __CFILEOPTIONSMAP__MULBERRY__

#include "CBlockOptionsMap.h"

// Classes
#ifdef PREFS_USE_LSTREAM
class LStream;
#endif

class CFileOptionsMap : public CBlockOptionsMap
{

public:
	CFileOptionsMap(void);
	virtual ~CFileOptionsMap(void);

	// Read/Write map
	virtual bool ReadMap(void);
	virtual bool WriteMap(bool verify);
	virtual void DeleteMap(void);

	virtual bool WriteEmptyEnums() const
		{ return false; }

	// Set file
#ifdef PREFS_USE_LSTREAM
	virtual void SetStream(LStream* aStream)
					{ mArchive = aStream; }
#else
	virtual void SetArchive(CArchive* anArchive)
					{ mArchive = anArchive; }
#endif

private:
#ifdef PREFS_USE_LSTREAM
	LStream*	mArchive;
#else
	CArchive*	mArchive;
#endif
};

#endif
