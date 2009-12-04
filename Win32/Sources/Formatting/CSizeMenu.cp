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


// Source for CSizePopup class


#include "CSizeMenu.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"


IMPLEMENT_DYNCREATE(CSizePopup, CPopupButton)

BEGIN_MESSAGE_MAP(CSizePopup, CPopupButton)
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSizePopup::CSizePopup()
{
}

// Default destructor
CSizePopup::~CSizePopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________


void CSizePopup::SetSize(int size)
{
	switch(size)
	{
	case 8:
		SetValue(IDM_SIZE_8);
		break;
	case 9:
		SetValue(IDM_SIZE_9);
		break;
	case 10:
		SetValue(IDM_SIZE_10);
		break;
	case 11:
		SetValue(IDM_SIZE_11);
		break;
	case 12:
		SetValue(IDM_SIZE_12);
		break;
	case 13:
		SetValue(IDM_SIZE_13);
		break;
	case 14:
		SetValue(IDM_SIZE_14);
		break;
	case 16:
		SetValue(IDM_SIZE_16);
		break;
	case 18:
		SetValue(IDM_SIZE_18);
		break;
	case 20:
		SetValue(IDM_SIZE_20);
		break;
	case 24:
		SetValue(IDM_SIZE_24);
		break;
	default:
		SetValue(0);
	}
	
}

int CSizePopup::GetSize() const
{
	switch(GetValue())
	{
	case IDM_SIZE_8:
		return 8;
	case IDM_SIZE_9:
		return 9;
	case IDM_SIZE_10:
		return 10;
	case IDM_SIZE_11:
		return 11;
	case IDM_SIZE_12:
		return 12;
	case IDM_SIZE_13:
		return 13;
	case IDM_SIZE_14:
		return 14;
	case IDM_SIZE_16:
		return 16;
	case IDM_SIZE_18:
		return 18;
	case IDM_SIZE_20:
		return 20;
	case IDM_SIZE_24:
		return 24;
	default:
		return 12;
	}
}

void CSizePopup::OnUpdateSize(CCmdUI* pCmdUI, bool enable)
{
	pCmdUI->Enable(enable);
	pCmdUI->SetCheck((pCmdUI->m_nID == mValue) && enable);
}
