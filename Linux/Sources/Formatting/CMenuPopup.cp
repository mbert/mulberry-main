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


// Source for CMenuPopup class

#include "CMenuPopup.h"
#include "CPreferences.h"




// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMenuPopup::CMenuPopup(LStream *inStream)
		: LGAPopup(inStream)
{
	inStream->ReadData(&mAttachMenu, sizeof(Boolean));
	
	savedValue = -1;
	defaultValue = 1;
	
	mMenu = nil;
	mPopup = nil;
}

// Default destructor
CMenuPopup::~CMenuPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________




void CMenuPopup::Ambiguate(){
		// ¥ Get the current item setup in the menu
		if (mMenu)
		{
			SetupCurrentMenuItem (mPopup, 0);
		}
		
		// ¥ Call our superclass to handle the setting of the value
		mValue = 0;
	
		// ¥ Now we need to get the popup redrawn so that the change
		// will be seen
		Draw ( nil );
		
		ClearMarks();
}




// Delete mark after popup
void CMenuPopup::SetValue(Int32 inValue)
{
	bool different = (mValue != inValue);

	// Don't do LGAPopup version as it sets item mark
	LControl::SetValue (inValue);
	//LGAPopup::SetValue(inValue);
	Char16 mark = GetMenuFontSize() < 12 ? '¥' : checkMark;
	
	if(mMenu){
		Int32 max = ::CountMenuItems(mMenu);
	
		for(int i = 1; i <= max; i++){
			::SetItemMark (mMenu, i, 0);
		}
		
		::SetItemMark(mMenu, GetValue(), mark);
	}
	
	if (different)
		Draw ( nil );
}

	
/*void CMenuPopup::HandlePopupMenuSelect(Point inPopupLoc, Int16 inCurrentItem, Int16 &outMenuID, Int16 &outMenuItem)
{
	// ¥ Always make sure item is marked
	Char16 mark = GetMenuFontSize() < 12 ? '¥' : checkMark;
	//if(mMenu)
	//	::SetItemMark(mMenu, GetValue(), mark);
	if(mPopup)
		::SetItemMark(mPopup, GetValue(), mark);

	// Do inherited
	
	ThrowIfNil_ (mPopup);
	if (mPopup)
	{
		// ¥ Save off the current system font family and size
		Int16 saveFont = ::LMGetSysFontFam ();
		Int16 saveSize = ::LMGetSysFontSize ();
		
		// ¥ Enclose this all in a try catch block so that we can
		// at least reset the system font if something goes wrong
		try 
		{

			// ¥ Handle the actual insertion into the hierarchical menubar
			//::InsertMenu ( menuH, hierMenu );
			
			// ¥ Reconfigure the system font so that the menu will be drawn in
			// our desired font and size
			FocusDraw ();
			{				
				ResIDT	textTID = GetTextTraitsID ();
				TextTraitsH traitsH = UTextTraits::LoadTextTraits ( textTID );
				if ( traitsH ) 
				{
					::LMSetSysFontFam ( (**traitsH).fontNumber );
					::LMSetSysFontSize ( (**traitsH).size );
					::LMSetLastSPExtra ( -1L );
				}
			}

			// ¥ Before we display the menu we need to make sure that we have the
			// current item marked in the menu. NOTE: we do NOT use the current
			// item that has been passed in here as that always has a value of one
			// in the case of a pulldown menu
			if(mPopup)
				SetupCurrentMenuItem (mPopup, GetValue () );
	
			// ¥ Then we call PopupMenuSelect and wait for it to return
			Int32 result = ::PopUpMenuSelect(mPopup,
															inPopupLoc.v,
															inPopupLoc.h,
															inCurrentItem );
			
			// ¥ Then we extract the values from the returned result
			// these are then passed back out to the caller
			outMenuID = HiWord ( result );
			outMenuItem = LoWord ( result );
			
		}
		catch (...) 
		{
			// ignore errorsÉ
		}

		// ¥ Restore the system font
		::LMSetSysFontFam ( saveFont );
		::LMSetSysFontSize ( saveSize );
		::LMSetLastSPExtra ( -1L );

		// ¥ Finally get the menu removed
		//::DeleteMenu ( GetPopupMenuResID ());
		
	}
		
	// ¥ Remove the current mark
	//if(mMenu)
		//::SetItemMark(mMenu, GetValue(), 0);
	if(mPopup)
		::SetItemMark(mPopup, GetValue(), 0);
	
}*/


void
CMenuPopup::HandlePopupMenuSelect	(	Point		inPopupLoc,
												Int16		inCurrentItem,
												Int16		&outMenuID,
												Int16		&outMenuItem )
{
	
	// ¥ Always make sure item is marked
		Char16 mark = GetMenuFontSize() < 12 ? '¥' : checkMark;
	if(mMenu)
		::SetItemMark(mMenu, GetValue(), mark);
	if(mPopup)
		::SetItemMark(mPopup, GetValue(), mark);
	
	
	MenuHandle	menuH = GetMacMenuH ();
	ThrowIfNil_ ( menuH );
	if ( menuH )
	{
		// ¥ Save off the current system font family and size
		Int16 saveFont = ::LMGetSysFontFam ();
		Int16 saveSize = ::LMGetSysFontSize ();
		
		// ¥ Enclose this all in a try catch block so that we can
		// at least reset the system font if something goes wrong
		try 
		{

			// ¥ Handle the actual insertion into the hierarchical menubar
			::InsertMenu ( menuH, hierMenu );
			
			// ¥ Reconfigure the system font so that the menu will be drawn in
			// our desired font and size
			FocusDraw ();
			{				
				ResIDT	textTID = GetTextTraitsID ();
				TextTraitsH traitsH = UTextTraits::LoadTextTraits ( textTID );
				if ( traitsH ) 
				{
					::LMSetSysFontFam ( (**traitsH).fontNumber );
					::LMSetSysFontSize ( (**traitsH).size );
					::LMSetLastSPExtra ( -1L );
				}
			}

			// ¥ Before we display the menu we need to make sure that we have the
			// current item marked in the menu. NOTE: we do NOT use the current
			// item that has been passed in here as that always has a value of one
			// in the case of a pulldown menu
			SetupCurrentMenuItem ( menuH, GetValue () );
	
			// ¥ Then we call PopupMenuSelect and wait for it to return
			Int32 result = ::PopUpMenuSelect ( 	menuH,
															inPopupLoc.v,
															inPopupLoc.h,
															inCurrentItem );
			
			// ¥ Then we extract the values from the returned result
			// these are then passed back out to the caller
			outMenuID = HiWord ( result );
			outMenuItem = LoWord ( result );
			
		}
		catch (...) 
		{
			// ignore errorsÉ
		}

		// ¥ Restore the system font
		::LMSetSysFontFam ( saveFont );
		::LMSetSysFontSize ( saveSize );
		::LMSetLastSPExtra ( -1L );

		// ¥ Finally get the menu removed
		//::DeleteMenu ( GetPopupMenuResID ());
		
	}
	
	if(mMenu)
		::SetItemMark(mMenu, GetValue(), 0);
	if(mPopup)
		::SetItemMark(mPopup, GetValue(), 0);
	
}	


void CMenuPopup::SaveState(){
	savedValue = mValue;
	//mValue = 0;
}

void CMenuPopup::RestoreState(){
	if(savedValue >= 0){
		Int32 max = ::CountMenuItems(mPopup);
	
		for(int i = 1; i <= max; i++){
			if(mMenu)
				::SetItemMark (mMenu, i, 0);
			::SetItemMark (mPopup, i, 0);
		}
		mValue = savedValue;
		SetupCurrentMenuItem ( mPopup, savedValue);
		Char16 mark = GetMenuFontSize() < 12 ? '¥' : checkMark;
		if(mMenu)
			::SetItemMark(mMenu, mValue, mark);
		::SetItemMark(mPopup, mValue, mark);
		
	}
}

void CMenuPopup::ClearMarks(){
	Int32 max = ::CountMenuItems(mPopup);
	
	for(int i = 1; i <= max; i++){
		if(mMenu)
			::SetItemMark (mMenu, i, 0);
	}
}

void CMenuPopup::FinishCreateSelf(){
	Int32 max = ::CountMenuItems(mPopup);
	
	for(int i = 1; i <= max; i++){
		if(mMenu)
			::SetItemMark (mMenu, i, 0);
		::SetItemMark (mPopup, i, 0);
	}
}