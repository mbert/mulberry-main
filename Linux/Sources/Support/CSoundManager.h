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

// CSoundManager.h
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

#ifndef __CSOUNDMANAGER__MULBERRY__
#define __CSOUNDMANAGER__MULBERRY__

class CSoundDriver;

class CSoundManager
{
public:
	static CSoundManager sSoundManager;

	CSoundManager();
	~CSoundManager();

	bool PlaySound(const char* spath);

protected:
	CSoundDriver*	mDriver;
};

class CSoundDriver
{
public:
	CSoundDriver() {}
	virtual ~CSoundDriver() {}

	virtual bool LoadDriver() = 0;
	virtual bool PlaySound(const char* spath) = 0;
};

class CEsoundDriver : public CSoundDriver
{
public:
	CEsoundDriver();
	virtual ~CEsoundDriver();

	virtual bool LoadDriver();
	virtual bool PlaySound(const char* spath);

protected:
	void*	mDLL;
	void*	mProc;
};

#endif
