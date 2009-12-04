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

// CSoundManager.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 22-Jul-2000
// Author: Cyrus Daboo
// Platforms: Unix
//
// Description:
// This class implements a manager for DLL based sound playing in Mulberry.
//

#include "CSoundManager.h"

#include <dlfcn.h>
#include <esd.h>
#include <stddef.h>

#pragma mark ____________________________consts

#pragma mark ____________________________statics

CSoundManager CSoundManager::sSoundManager;

#pragma mark ____________________________CSoundManager

// Constructor
CSoundManager::CSoundManager()
{
	mDriver = NULL;

	// Try to load each type of driver in some order and
	// use the first one that works!

	CSoundDriver* drv = NULL;

	// Try Esound first
	drv = new CEsoundDriver;
	if (drv->LoadDriver())
	{
		mDriver = drv;
		return;
	}
}

CSoundManager::~CSoundManager()
{
	if (mDriver)
		delete mDriver;
	mDriver = NULL;
}

bool CSoundManager::PlaySound(const char* spath)
{
	if (mDriver)
		return mDriver->PlaySound(spath);
	else
		return false;
}

#pragma mark ____________________________CEsoundDriver

const char cEsoundLib[] = "libesd.so";
const char cEsoundPlaySymbol[] = "esd_play_file";
typedef int (*esd_play_file_pp)(const char *, const char *, int);

// Constructor
CEsoundDriver::CEsoundDriver()
{
	mDLL = NULL;
	mProc = NULL;
}

// Destructor
CEsoundDriver::~CEsoundDriver()
{
	if (mDLL)
		::dlclose(mDLL);
	mDLL = NULL;
	mProc = NULL;
}

// Load DLL
bool CEsoundDriver::LoadDriver()
{
	mDLL = ::dlopen(cEsoundLib, RTLD_NOW);
	if (mDLL)
	{
		mProc = ::dlsym(mDLL, cEsoundPlaySymbol);
		if (!mProc)
		{
			::dlclose(mDLL);
			mDLL = NULL;
		}
	}

	return mDLL;
}

// Play a sound in a file
bool CEsoundDriver::PlaySound(const char* spath)
{
	if (mDLL && mProc)
	{
		(*((esd_play_file_pp) mProc))("mulberry", spath, 1);
		return true;
	}

	return false;
}

