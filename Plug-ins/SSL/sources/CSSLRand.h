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


//	Gathers unpredictable system data to be used for generating
//	random bits

#include <MacTypes.h>

class CSSLRand
{
public:
	static CSSLRand* sSSLRand;
	
	CSSLRand ();
	void PeriodicAction ();

private:

	// Private calls

	void		AddTimeSinceMachineStartup ();
	void		AddAbsoluteSystemStartupTime ();
	void		AddAppRunningTime ();
	void		AddStartupVolumeInfo ();
	void		AddFiller ();

	void		AddCurrentMouse ();
	void		AddNow (double millisecondUncertainty);
	void		AddBytes (void *data, long size, double entropy);
	
	void		GetTimeBaseResolution ();
	unsigned long	SysTimer ();

	// System Info	
	bool		mSupportsLargeVolumes;
	bool		mIsPowerPC;
	bool		mIs601;
	
	// Time info
	double		mTimebaseTicksPerMillisec;
	unsigned long	mLastPeriodicTicks;
	
	// Mouse info
	long		mSamplePeriod;
	Point		mLastMouse;
	long		mMouseStill;
};
