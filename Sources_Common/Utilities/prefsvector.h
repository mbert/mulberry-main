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


// prefsvector.h - vectors of preference items

#ifndef __PREFSVECTOR__MULBERRY__
#define __PREFSVECTOR__MULBERRY__

#include <vector>
#include "ptrvector.h"

#include "cdstring.h"

class COptionsMap;

template <class T> class prefsvector : public std::vector<T>
{
public:
		prefsvector() :
	std::vector<T>() {}
		prefsvector(const prefsvector& copy) :
	std::vector<T>(copy) {}

	virtual ~prefsvector() {}

    prefsvector<T>& operator= (const prefsvector<T>& copy)
	{ return static_cast<prefsvector<T>&>(std::vector<T>::operator=(copy)); }
    bool operator== (const prefsvector<T>& other) const
	{ return static_cast<std::vector<T> >(*this) == static_cast<std::vector<T> >(other); }

	cdstring GetInfo() const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);
};

template <class T> class prefsptrvector : public ptrvector<T>
{
public:
		prefsptrvector() :
			ptrvector<T>() { mLastMapMultiCount = 0; }
		prefsptrvector(const prefsptrvector& copy) :
			ptrvector<T>(copy) { mLastMapMultiCount = copy.mLastMapMultiCount; }

	virtual ~prefsptrvector() {}

    prefsptrvector<T>& operator= (const prefsptrvector<T>& copy)
    	{ return static_cast<prefsptrvector<T>&>(ptrvector<T>::operator=(copy)); }
    bool operator== (const prefsptrvector<T>& other) const
    	{ return static_cast<ptrvector<T> >(*this) == static_cast<ptrvector<T> >(other); }

	cdstring GetInfo() const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);

	void WriteToMapMulti(COptionsMap* theMap, const char* key, bool dirty_only) const;
	bool ReadFromMapMulti(COptionsMap* theMap, const char* key, NumVersion& vers_prefs, bool clear_if_empty = true);

protected:
	unsigned long mLastMapMultiCount;
};

#endif
