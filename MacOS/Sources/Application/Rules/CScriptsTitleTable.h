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


// Header for CScriptsTitleTable class

#ifndef __CSCRIPTSTITLETABLE__MULBERRY__
#define __CSCRIPTSTITLETABLE__MULBERRY__

#include "CTitleTable.h"

// Constants

// Resources
const	ResIDT		STRx_TargetsTitles = 1201;
enum
{
	str_TargetsTitleName = 1
};
const	ResIDT		STRx_ScriptsTitles = 1202;
enum
{
	str_ScriptsTitleName = 1
};


// Classes
class CScriptsTitleTable : public CTitleTable
{
public:
	enum { class_ID = 'ScTi' };

					CScriptsTitleTable();
					CScriptsTitleTable(LStream *inStream);
	virtual 		~CScriptsTitleTable();

	virtual CColumnInfoArray& GetColumns();					// Get column info

protected:
	CColumnInfoArray	mColumnInfo;

	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	InitColumnChanger(void);				// Init column changer
	virtual void	MenuChoice(short col, bool sort_col, short menu_item);

	virtual void	DrawItem(SColumnInfo& col_info,
								bool sort_col, const Rect &inLocalRect);		// Draw title
};

#endif
