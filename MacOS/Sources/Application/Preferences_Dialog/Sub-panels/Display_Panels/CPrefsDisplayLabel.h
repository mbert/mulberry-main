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


// Header for CPrefsDisplayLabel class

#ifndef __CPREFSDISPLAYLABEL__MULBERRY__
#define __CPREFSDISPLAYLABEL__MULBERRY__

#include "CPrefsDisplayPanel.h"

#include "CMessageFwd.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplayLabel = 5045;

const	PaneIDT		paneid_DLName[] = {'NAM1', 'NAM2', 'NAM3', 'NAM4', 'NAM5', 'NAM6', 'NAM7', 'NAM8'};

const	PaneIDT		paneid_DLColour[] = {'COL1', 'COL2', 'COL3', 'COL4', 'COL5', 'COL6', 'COL7', 'COL8'};
const	PaneIDT		paneid_DLUseColour[] = {'UCL1', 'UCL2', 'UCL3', 'UCL4', 'UCL5', 'UCL6', 'UCL7', 'UCL8'};

const	PaneIDT		paneid_DLBkgColour[] = {'BKG1', 'BKG2', 'BKG3', 'BKG4', 'BKG5', 'BKG6', 'BKG7', 'BKG8'};
const	PaneIDT		paneid_DLUseBkgColour[] = {'UBK1', 'UBK2', 'UBK3', 'UBK4', 'UBK5', 'UBK6', 'UBK7', 'UBK8'};

const	PaneIDT		paneid_DLBold[] = {'BLD1', 'BLD2', 'BLD3', 'BLD4', 'BLD5', 'BLD6', 'BLD7', 'BLD8'};

const	PaneIDT		paneid_DLItalic[] = {'ITL1', 'ITL2', 'ITL3', 'ITL4', 'ITL5', 'ITL6', 'ITL7', 'ITL8'};

const	PaneIDT		paneid_DLStrike[] = {'STK1', 'STK2', 'STK3', 'STK4', 'STK5', 'STK6', 'STK7', 'STK8'};

const	PaneIDT		paneid_DLUnderline[] = {'UND1', 'UND2', 'UND3', 'UND4', 'UND5', 'UND6', 'UND7', 'UND8'};

// Mesages
const	MessageT	msg_DLUseColour[] = {'UCL1', 'UCL2', 'UCL3', 'UCL4', 'UCL5', 'UCL6', 'UCL7', 'UCL8'};
const	MessageT	msg_DLUseBkgColour[] = {'UBK1', 'UBK2', 'UBK3', 'UBK4', 'UBK5', 'UBK6', 'UBK7', 'UBK8'};
const	MessageT	msg_DLIMAPLabels = 'IMAP';

// Resources
const	ResIDT		RidL_CPrefsDisplayLabelBtns = 5045;

// Classes
class LCheckBox;
class LGAColorSwatchControl;

class	CPrefsDisplayLabel : public CPrefsDisplayPanel,
							public LListener
{
private:
	SFullStyleItems2	mLabels[NMessage::eMaxLabels];
	cdstrvect			mIMAPLabels;

public:
	enum { class_ID = 'Dlbl' };

					CPrefsDisplayLabel();
					CPrefsDisplayLabel(LStream *inStream);
	virtual 		~CPrefsDisplayLabel();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
	
			void	OnIMAPLabels();
};

#endif
