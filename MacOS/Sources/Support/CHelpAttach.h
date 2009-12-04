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


// Header for CHelpAttach class

#ifndef __CHELPATTACH__MULBERRY__
#define __CHELPATTACH__MULBERRY__


#if !PP_Target_Carbon
#include <Balloons.h>
#endif

// Consts

// Panes

// Messages

const MessageT	msg_ShowHelp = 0x4000;

// Resources
 
// Classes

class LPane;

class CHelpAttach : public LAttachment {

protected:
	bool			mHasBalloonHelp;			// Flag for Gestalt
	short			mStrId;						// Id for STR# rsrc
	short			mIndex;						// Index for balloon help text
	short			mCurrentIndex;				// Index of current balloon
	static LPane*	sHelpPane;					// Last help pane

public:
	enum { class_ID = 'Help' };
	
					CHelpAttach(short strId, short index);
					CHelpAttach(LStream *inStream);
	
protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);					// Show help balloon
#if !PP_Target_Carbon
	virtual	void	FillHMRecord(HMMessageRecord &theHelpMsg, void *ioParam);		// Fill in the HMMessageRecord
	virtual bool	SameBalloon(void *ioParam);										// Will display same balloon?
#endif
};

class CHelpPaneAttach : public CHelpAttach {

protected:
	short			mEnabledIndex;					// Index for enabled pane balloon help text
	short			mDisabledIndex;					// Index for disabled pane balloon help text

public:
	enum { class_ID = 'PHlp' };
	
					CHelpPaneAttach(short strId, short index_enabled, short index_disabled);
					CHelpPaneAttach(LStream *inStream);
	
protected:
#if !PP_Target_Carbon
	virtual	void	FillHMRecord(HMMessageRecord &theHelpMsg, void *ioParam);		// Fill in the HMMessageRecord
#endif
};
class CHelpControlAttach : public CHelpAttach {

protected:
	short			mEnabledOnIndex;				// Index of balloon help text for enabled control on
	short			mEnabledOffIndex;				// Index of balloon help text for enabled control off
	short			mDisabledIndex;					// Index of balloon help text for disabled control

public:
	enum { class_ID = 'CHlp' };
	
					CHelpControlAttach(short strId, short index_enabled_on, short index_enabled_off, short index_disabled);
					CHelpControlAttach(LStream *inStream);
	
protected:
#if !PP_Target_Carbon
	virtual	void	FillHMRecord(HMMessageRecord &theHelpMsg, void *ioParam);		// Fill in the HMMessageRecord
#endif

};

#endif
