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


// CPreferenceEnum : class to implement a favourite item

#include "CPreferenceEnum.h"

#include "CPreferences.h"

#include "char_stream.h"
#include "CStringUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CPreferenceEnum

// Write prefs
template <class T> cdstring CPreferenceEnum<T>::GetInfo(void) const
{
	// Enum is index into values
	cdstring all(GetValues()[mValue]);
	return all;
}

// Read prefs
template <class T> bool CPreferenceEnum<T>::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Get the value string
	cdstring str;
	txt.get(str);

	// Look up its index in the values list
	mValue = static_cast<T>(::strindexfind(str, GetValues(), 0));
	return true;
}

#pragma mark ____________________________Specialisations of values

const char* ESaveOptions_values[] = {"File", "Mailbox", "Choose" };
template<> const char** CPreferenceEnum<ESaveOptions>::GetValues() const
{
	return ESaveOptions_values;
}
template class CPreferenceEnum<ESaveOptions>;

const char* EMDNOptions_values[] = {"Always", "Never", "Prompt" };
template<> const char** CPreferenceEnum<EMDNOptions>::GetValues() const
{
	return EMDNOptions_values;
}
template class CPreferenceEnum<EMDNOptions>;
