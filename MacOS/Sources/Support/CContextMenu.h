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


// CContextMenuAttachment

#ifndef __CCONTEXTMENU__MULBERRY__
#define __CCONTEXTMENU__MULBERRY__

const MessageT	msg_DoContextualMenu = 0x4001;

class CContextMenuProcessAttachment : public LAttachment
{
public:
	enum { class_ID = 'CMMp' };
	
					CContextMenuProcessAttachment(LApplication* itsApp);
					CContextMenuProcessAttachment(LStream *inStream);
	
	static bool HasCMM()
		{ return sHasCMM; }
	static bool IsCMMEvent(const EventRecord& event);
	static const EventRecord& CMMEvent()
		{ return sCMMEvent; }
	static bool ProcessingContextMenu()
		{ return sProcessing; }

protected:
	static bool			sHasCMM;					// Flag for Gestalt
	static EventRecord	sCMMEvent;
	static bool			sProcessing;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);					// Do context

private:
	LApplication* mApp;

	void	InitCMM(void);

};

class CContextMenuAttachment : public LAttachment
{
public:
	enum { class_ID = 'CMMa' };
	
					CContextMenuAttachment(ResIDT menu_id, LCommander* cmdr, bool execute_click = true);
					CContextMenuAttachment(LStream *inStream);
	
	static void AddUniqueContext(LAttachable* attach_to, ResIDT menu_id, LCommander* cmdr, bool execute_click = true);
	static void AddUniqueContext(LAttachable* attach_to, LAttachment* attach);

protected:
	ResIDT	mMenuID;
	LCommander* mCmdr;
	bool mExecuteClick;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);					// Do context

private:
	static std::map<ResIDT, LMenu*> sContextMenus;

	LMenu* GetMenu(ResIDT resid);
};


class CCopyToContextMenuAttachment : public CContextMenuAttachment
{
public:
	CCopyToContextMenuAttachment(ResIDT menu_id, LCommander* cmdr, bool execute_click = true)
		: CContextMenuAttachment(menu_id, cmdr, execute_click) {}
	CCopyToContextMenuAttachment(LStream *inStream)
		: CContextMenuAttachment(inStream) {}

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);					// Do context
};

class CNULLContextMenuAttachment : public CContextMenuAttachment
{
public:
	CNULLContextMenuAttachment()
		: CContextMenuAttachment(0, NULL, true) {}
	CNULLContextMenuAttachment(LStream *inStream)
		: CContextMenuAttachment(inStream) {}

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);					// Do context
};

// This is a mix-in to a pane/view etc to allow it to intercept the context menu
// command - including the original location of the click that caused the result

class CContextCommandIntercept
{
public:
	CContextCommandIntercept()
	{
	}
	virtual ~CContextCommandIntercept() {}

	virtual bool ObeyContextCommand(CommandT inCommand, const EventRecord& event) = 0;
};

#endif
