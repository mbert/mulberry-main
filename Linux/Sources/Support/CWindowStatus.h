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


// Window status Support

#ifndef __CWINDOWSTATUS__MULBERRY__
#define __CWINDOWSTATUS__MULBERRY__

// CWindowStatus: indicates status of window

class CWindowStatus
{
	enum EWindowStatus
	{
		eWindowStatusOpening = 1,
		eWindowStatusOpen,
		eWindowStatusClosing,
		eWindowStatusClosed
	};

public:
	CWindowStatus()
		{ mStatus = eWindowStatusOpening; }
	~CWindowStatus()
		{ mStatus = eWindowStatusClosed; }
	
	void SetOpening()
		{ mStatus = eWindowStatusOpening; }
	bool IsOpening() const
		{ return (mStatus == eWindowStatusOpening); }
	
	void SetOpen()
		{ mStatus = eWindowStatusOpen; }
	bool IsOpen() const
		{ return (mStatus == eWindowStatusOpen); }
	
	void SetClosing()
		{ mStatus = eWindowStatusClosing; }
	bool IsClosing() const
		{ return (mStatus == eWindowStatusClosing); }
	
	void SetClosed()
		{ mStatus = eWindowStatusClosed; }
	bool IsClosed() const
		{ return (mStatus == eWindowStatusClosed); }
	
private:
	EWindowStatus	mStatus;
};

#endif
