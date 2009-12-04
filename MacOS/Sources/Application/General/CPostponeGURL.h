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


// Header for CPostponeGURL class

#ifndef __CPOSTPONEGURL__MULBERRY__
#define __CPOSTPONEGURL__MULBERRY__


// Classes
class CPostponeGURL : public LPeriodical
{
public:
	enum EType
	{
		eMailto = 0,
		eWebcal
	};
					CPostponeGURL(char *text, EType type);
					~CPostponeGURL();

	static	void	Pause(bool pause_it)
						{ sPause = pause_it; }

protected:
	virtual void	SpendTime(const EventRecord &inMacEvent);

private:
	char			*mText;
	EType			mType;
	bool			mFirstTime;
	static bool		sPause;

};

#endif
