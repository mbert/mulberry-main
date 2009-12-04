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

// CAboutPluginsDialog.h : header file
//

#ifndef __CABOUTPLUGINSDIALOG__MULBERRY__
#define __CABOUTPLUGINSDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "UNX_LTextHierTable.h"
#include "CPlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutPluginsDialog dialog

class CAboutPluginsDialog;

class CStaticText;
class JXTextButton;

class CAboutPluginsTable : public LTextHierTable
{
public:
	CAboutPluginsTable(JXScrollbarSet* scrollbarSet,
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);

	void SetDlog(CAboutPluginsDialog* dlog)
		{ mDlog = dlog; }

protected:
	CAboutPluginsDialog* mDlog;
	virtual void DoSelectionChanged();
};

class CAboutPluginsDialog : public CDialogDirector
{
	friend class CAboutPluginsTable;

// Construction
public:
	CAboutPluginsDialog(JXDirector* supervisor);

	static bool PoseDialog();

protected:
	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual void OnAboutPluginsRegister();
	virtual void OnClickAboutPluginsList();

// begin JXLayout

    CStaticText*  mName;
    CStaticText*  mVersion;
    CStaticText*  mType;
    CStaticText*  mManufacturer;
    CStaticText*  mDescription;
    CStaticText*  mMode;
    JXTextButton* mOKBtn;
    JXTextButton* mRegisterBtn;

// end JXLayout
	CAboutPluginsTable*	mList;

	CPluginList		mHierPlugins;

private:
	void	InitTable();
	void	ShowPluginInfo(const CPlugin* plugin);
};

#endif
