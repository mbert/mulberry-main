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


// Source for CSoundManager class

#include "CSoundManager.h"

#include "CStatusWindow.h"
#include "C3PaneWindow.h"

#include "diriterator.h"

#include <algorithm>

CSoundManager CSoundManager::sSoundManager;

CSoundManager::CSoundManager()
{
#if PP_Target_Carbon
	mActiveMovie = NULL;
#endif
}

// Must call this after app's main has started as it will attemp to
// init Quicktime which cannot be done until after toolbox is initialised by app
void CSoundManager::Initialize()
{
	UEnvironment::InitEnvironment();
	LoadSounds();
}

void CSoundManager::LoadSounds()
{
	// On OS X we have to use AIFF system files
#if PP_Target_Carbon
	if (UEnvironment::GetOSVersion () >= 0x0A00)
		LoadSoundsFile();
	else
#endif
		// Earlier OS's use snd resources in the system file
		LoadSoundsRsrc();
		
	// Sort the list
	std::sort(mSounds.begin(), mSounds.end());
}

void CSoundManager::LoadSoundsRsrc()
{
	// Don't load the resources - we just want to know their names
	::SetResLoad(false);
	
	// Look at each snd resource
	short index = 1;
	Handle rsrc = NULL;
	do
	{
		rsrc = ::GetIndResource('snd ', index++);
		if (rsrc)
		{
			// Get its name
			SInt16 id;
			ResType type;
			Str255 name;
			::GetResInfo(rsrc, &id, &type, name);
			
			// Add name to list if its not empty
			if (*name)
				mSounds.push_back(name);
				
			// Release the resource
			::ReleaseResource(rsrc);
		}
	} while(rsrc);
	
	// Turn resource loading back on
	::SetResLoad(true);
}

#if PP_Target_Carbon
void CSoundManager::LoadSoundsFile()
{
	// Start QuickTime
	::EnterMovies();
	
	// Look in each primary domain
	LoadSoundsFileDomain(kSystemDomain);
	LoadSoundsFileDomain(kLocalDomain);
	//LoadSoundsFileDomain(kNetworkDomain); <- no network op as it might be slow
	LoadSoundsFileDomain(kUserDomain);
}

void CSoundManager::LoadSoundsFileDomain(short domain)
{
	// Locate sound directory in specified domain
	FSRef	sndSpec;
	OSErr err = ::FSFindFolder(domain, kSystemSoundsFolderType, kDontCreateFolder, &sndSpec);
	if (err != noErr)
		return;
		
	// Now iterate over directory adding all .aiff files
	{
		diriterator iter(sndSpec, false, ".aiff");
		
		const char* fname;
		while(iter.next(&fname))
		{
			// Copy into list without extension
			cdstring temp(fname);
			if (::strrchr(temp.c_str(), '.'))
				*::strrchr(temp.c_str_mod(), '.') = 0;
			
			// Only add if unique
			cdstrvect::const_iterator found = std::find(mSounds.begin(), mSounds.end(), temp);
			if (found == mSounds.end())
			{
				mSounds.push_back(temp);
				mDomainMap.insert(cdstrshortmap::value_type(temp, domain));
				mNameFileMap.insert(cdstrmap::value_type(temp, fname));
			}
		}
	}

	{		
		// Now iterate over directory adding all .aif files
		diriterator iter(sndSpec, false, ".aif");
		
		const char* fname;
		while(iter.next(&fname))
		{
			// Copy into list without extension
			cdstring temp(fname);
			if (::strrchr(temp.c_str(), '.'))
				*::strrchr(temp.c_str_mod(), '.') = 0;
			
			// Only add if unique
			cdstrvect::const_iterator found = std::find(mSounds.begin(), mSounds.end(), temp);
			if (found == mSounds.end())
			{
				mSounds.push_back(temp);
				mDomainMap.insert(cdstrshortmap::value_type(temp, domain));
				mNameFileMap.insert(cdstrmap::value_type(temp, fname));
			}
		}
	}
}
#endif

bool CSoundManager::PlaySound(const char* snd) const
{
	// On OS X we have to use AIFF system files
#if PP_Target_Carbon
	if (UEnvironment::GetOSVersion () >= 0x0A00)
		return PlaySoundFile(snd);
	else
#endif
		// Earlier OS's use snd resources in the system file
		return PlaySoundRsrc(snd);
}

bool CSoundManager::PlaySoundRsrc(const char* snd) const
{
	// Play named sound resource
	if (snd && *snd)
	{
		LStr255 title = snd;
		SndListHandle hdl = (SndListHandle) ::GetNamedResource('snd ', title);
		if (hdl)
		{
			{
				StHandleLocker _lock((Handle) hdl);
				::SndPlay(nil, hdl, false);
			}
			::ReleaseResource((Handle) hdl);
			return true;
		}
	}
	
	return false;
}

#if PP_Target_Carbon
bool CSoundManager::PlaySoundFile(const char* snd) const
{
	// Determine domain for the chosen sound
	cdstrshortmap::const_iterator found = mDomainMap.find(cdstring(snd));
	if (found == mDomainMap.end())
		return false;

	// Locate domain sound directory
	SInt16	sndVRefNum;
	SInt32	sndDirID;
	FSSpec	sndSpec;
	OSErr err = ::FindFolder((*found).second, kSystemSoundsFolderType, kDontCreateFolder, &sndVRefNum, &sndDirID);
	if (err != noErr)
		return false;
	err = ::FSMakeFSSpec(sndVRefNum, sndDirID, NULL, &sndSpec);
	if (err != noErr)
		return false;

	// Determine filename for the chosen sound
	cdstrmap::const_iterator foundfname = mNameFileMap.find(cdstring(snd));
	if (foundfname == mNameFileMap.end())
		return false;

	// Get file spec for this sound
	LStr255 str((*foundfname).second);
	::PLstrncpy(sndSpec.name, str, 31);
	sndSpec.parID = sndDirID;

	// Open a file as movie
	short myRefNum;
	err = ::OpenMovieFile(&sndSpec, &myRefNum, fsRdPerm);
	if (err != noErr)
		return false;

	// Create a movie from this file
	Movie myMovie;
	err = ::NewMovieFromFile(&myMovie, myRefNum, NULL, (StringPtr)NULL, newMovieActive, NULL);
	if (myRefNum != 0)
		::CloseMovieFile(myRefNum);
	if (err != noErr)
		return false;

	// Add movie to queue
	mMovieList.push_back(myMovie);

	return true;
}

void CSoundManager::SoundIdle()
{
	// Check whether no active movie but some in queue
	if (!mActiveMovie && mMovieList.size())
	{
		// Get the one at the front of the list and start it
		mActiveMovie = mMovieList.front();
		mMovieList.pop_front();
		LWindow* window = CStatusWindow::sStatusWindow;
		if (window == NULL)
			window = C3PaneWindow::s3PaneWindow;

		::SetMovieGWorld(mActiveMovie, window ? (CGrafPtr)window->GetMacPort() : NULL, NULL); 
		::StartMovie(mActiveMovie);
	}

	// Play the active sound
	if (mActiveMovie)
	{
		// Idle it if not done
		if (!::IsMovieDone(mActiveMovie))
			::MoviesTask(mActiveMovie, 0);
		else
		{
			// Dispose and remove from list
			::StopMovie(mActiveMovie);
			::DisposeMovie(mActiveMovie);
			mActiveMovie = NULL;
		}
	}
	
}

#endif