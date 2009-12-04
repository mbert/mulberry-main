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

#ifndef __CFILEDOCUMENT__MULBERRY__
#define __CFILEDOCUMENT__MULBERRY__

#include <JXFileDocument.h>
#include "CCommander.h"
#include "CWindowStatus.h"

#include <JXWindow.h>
#include "CMainMenu.h"

class CTextDisplay;

class CFileDocument : public CCommander,		// Commander must be first so it gets destroyed last
						public JXFileDocument,
						public CWindowStatus
{
public:
	CFileDocument(JXDirector* owner, const JCharacter* fileName,
								JBoolean onDisk, JBoolean wantBackupFile,
								JCharacter* defaultFileNameSuffix);

	void Show() {GetWindow()->Show();}
	//Override OnCreate with window creation code. 
	virtual void OnCreate() {}

	CMainMenu::MenusArray& GetMenus()
		{ return mainMenus; }
	CMainMenu::MenuId GetMainMenuID(JXTextMenu* menu) const;

	CTextDisplay* GetTextDisplay()
		{ return mTextDisplay; }

	virtual void SafetySave(const JXDocumentManager::SafetySaveReason reason);

protected:
	//The array to hold the main menubar and menus
	CMainMenu::MenusArray mainMenus;
	CTextDisplay* mTextDisplay;

	virtual void DiscardChanges() {}
	void CreateMainMenu(JXContainer *wnd, unsigned short which, bool listen = true);
	virtual void Receive (JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void	ResetState(bool force = false) = 0;			// Reset window state
	virtual void	SaveState(void) {}							// Save current state in prefs
	virtual void	SaveDefaultState(void) = 0;					// Save current state as default
};

#endif
