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


// CDateControl.h : header file
//

#ifndef __CDATECONTROL__MULBERRY__
#define __CDATECONTROL__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CDateControl window

class CDateControl : public CWnd
{
	DECLARE_DYNAMIC(CDateControl)

// Construction
public:
	CDateControl();
	virtual ~CDateControl();

	void SetDate(time_t date)
		{ if (mCommonCtrlEx) SetDateEx(date); else SetDateOld(date); }
	time_t GetDate() const
		{ if (mCommonCtrlEx) return GetDateEx(); else return GetDateOld(); }
	
	void SetCommonCtrlEx(bool ex)
		{ mCommonCtrlEx = ex; }

protected:
	DECLARE_MESSAGE_MAP()

private:
	bool mCommonCtrlEx;
	
	void SetDateEx(time_t date);
	time_t GetDateEx() const;
	
	void SetDateOld(time_t date);
	time_t GetDateOld() const;
};

/////////////////////////////////////////////////////////////////////////////

#endif
