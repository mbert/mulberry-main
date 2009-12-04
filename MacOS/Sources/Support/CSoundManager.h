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


// Header for CSoundManager class

#ifndef __CSOUNDMANAGER__MULBERRY__
#define __CSOUNDMANAGER__MULBERRY__

#include "cdstring.h"
#include <list.h>

// Classes

class CSoundManager
{
public:
	static CSoundManager sSoundManager;

	CSoundManager();

	void Initialize();

	const cdstrvect& GetSounds() const
		{ return mSounds; }

	bool PlaySound(const char* snd) const;

#if PP_Target_Carbon
	void SoundIdle();
#endif

private:
	cdstrvect		mSounds;
#if PP_Target_Carbon
	typedef std::map<cdstring, short> cdstrshortmap;
	cdstrshortmap	mDomainMap;
	cdstrmap		mNameFileMap;
	typedef list<Movie> movielist;
	mutable Movie			mActiveMovie;
	mutable movielist		mMovieList;
#endif
	
	void LoadSounds();
	void LoadSoundsRsrc();
#if PP_Target_Carbon
	void LoadSoundsFile();
	void LoadSoundsFileDomain(short domain);
#endif

	bool PlaySoundRsrc(const char* snd) const;
#if PP_Target_Carbon
	bool PlaySoundFile(const char* snd) const;
#endif
};

#endif
