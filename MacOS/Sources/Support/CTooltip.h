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


// Header for CTooltip class

#ifndef __CTOOLTIP__MULBERRY__
#define __CTOOLTIP__MULBERRY__

// Constants
const	PaneIDT		paneid_TooltipWindow = 10004;
const	PaneIDT		paneid_TooltipText = 'TIPT';


// Classes
class CTextDisplay;

class CTooltip : public LWindow, public LPeriodical
{
public:
	enum { class_ID = 'TipW' };
	
	static CTooltip* sTooltip;

			CTooltip(LStream *inStream);
	virtual	~CTooltip();

	static void ShowTooltip(LPane* target, const cdstring& txt);
	static void ShowTooltip(LPane* target, SPoint32 pos, const cdstring& txt);
	static void HideTooltip();
	static void EnableTooltips(bool enable);

	virtual	void SpendTime(const EventRecord& inMacEvent);
	
protected:
	CTextDisplay*	mTxt;
	UInt32			mEnterTime;
	LPane*			mTarget;
	SPoint32		mTargetPos;
	bool			mAutoPos;
	cdstring		mTitle;
	bool			mEnabled;
	
	virtual void	FinishCreateSelf(void);

	static void InitTooltip();
	void StartTooltip(LPane* target, SPoint32 pos, bool auto_pos, const cdstring& txt);
	void ShowIt();
	void StopTooltip();
	void EnableTooltip(bool enable);
};

#endif
