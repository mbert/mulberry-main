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


// CRemotePrefsSets.h

// Class to handle remote sets of preferences

#ifndef __CREMOTEPREFSSETS__MULBERRY__
#define __CREMOTEPREFSSETS__MULBERRY__

#include "cdstring.h"

// Classes
class CRemotePrefsSet
{
public:
		CRemotePrefsSet();
		~CRemotePrefsSet();

	void	ListRemoteSets(void);
	void	UpdateRemoteSets(void);
	
	cdstrvect& GetRemoteSets(void)
		{ return mSets; }
	
	bool	IsDuplicateName(const cdstring& test, cdstring& duplicate) const;
	void	DeleteSet(cdstring set);
	void	RenameSet(cdstring old_set, cdstring new_set);

private:
	cdstrvect	mSets;
	
	void	ConvertName(cdstring& name) const;
};

#endif
