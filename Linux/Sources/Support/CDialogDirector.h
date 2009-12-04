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


// Header for CDialogDirector class

#ifndef __CDIALOGDIRECTOR__MULBERRY__
#define __CDIALOGDIRECTOR__MULBERRY__

#include <JXDialogDirector.h>

class CDialogDirector : public JXDialogDirector
{
public:
	enum
	{
		kDialogContinue = 0,
		kDialogClosed_OK = 1,
		kDialogClosed_Btn1 = 1,
		kDialogClosed_Cancel = 2,
		kDialogClosed_Btn2 = 2,
		kDialogClosed_Btn3,
		kDialogClosed_Btn4
	};
	CDialogDirector(JXDirector* supervisor, const JBoolean modal = kTrue);

	virtual int DoModal(bool do_create = true);

	virtual void	 Activate();
	virtual JBoolean Deactivate();
	virtual void EndDialog(int btn);

	virtual int GetClosedState() const
		{ return mClosed; }


protected:
	int		mClosed;
	bool	mPendingResize;

	virtual void OnCreate() = 0;
	virtual void Receive(JBroadcaster* sender, const Message& message);
			void AdjustSize(const JSize w, const JSize h);
			void BoundsResized(const JCoordinate dw, const JCoordinate dh);
	virtual void Continue();
};
#endif
