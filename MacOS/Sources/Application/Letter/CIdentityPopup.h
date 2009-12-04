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


// Header for CIdentityPopup class

#ifndef __CIDENTITYPOPUP__MULBERRY__
#define __CIDENTITYPOPUP__MULBERRY__

#include <LPopupButton.h>

// Consts

enum
{
	eIdentityPopup_New = 1,
	eIdentityPopup_Edit,
	eIdentityPopup_Delete,
	eIdentityPopup_Custom,
	eIdentityPopup_FirstWithoutCustom = 5,
	eIdentityPopup_FirstWithCustom = 6
};

// Classes
class CIdentity;
class CPreferences;
class LCommander;
class CIdentityList;
class cdstring;

class CIdentityPopup : public LPopupButton
{

public:
	enum { class_ID = 'IDop' };

					CIdentityPopup(LStream *inStream);
	virtual 		~CIdentityPopup();

	unsigned long	FirstIndex() const
	{
		return mHasCustom ? eIdentityPopup_FirstWithCustom : eIdentityPopup_FirstWithoutCustom;
	}
	virtual void	SetValue(long value);					// Handle special cases
			void	SetIdentity(CPreferences* prefs, const CIdentity* id);
			void	SetIdentity(CPreferences* prefs, const cdstring& name);
			const CIdentity& GetIdentity(CPreferences* prefs) const;
			const CIdentity* GetCustomIdentity() const
			{
				return mCustomIdentity;
			}

			void	Reset(const CIdentityList& ids, bool custom = false);			// Reset items
			void	DoNewIdentity(CPreferences* prefs);			// Add new identity
			bool	DoEditIdentity(CPreferences* prefs);		// Edit identity
			void	DoDeleteIdentity(CPreferences* prefs);		// Delete existing identities

			bool	EditCustomIdentity();

			unsigned long	GetCount() const;

private:
	long			mOldValue;
	bool			mHasCustom;
	CIdentity*		mCustomIdentity;

};

#endif
