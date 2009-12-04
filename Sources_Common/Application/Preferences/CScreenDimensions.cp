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


// Source for CScreenDimensions class

#include "CScreenDimensions.h"

#include "CWindowStates.h"

#include "char_stream.h"

#if __framework == __jx
#include "CMulberryApp.h"
#include <JXDisplay.h>
#include <jXGlobals.h>
#endif

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;


// Get the current screen dimensions
template<class T> int CScreenDimensions<T>::operator==(const CScreenDimensions<T>& other) const
{
	return (mWidth == other.mWidth) &&
				(mHeight == other.mHeight) &&
				(mData == other.mData);
}

template<class T> void CScreenDimensions<T>::GetCurrentScreen()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	long horiz;
	long vert;

	Rect desktop = (**::GetMainDevice()).gdRect;
	horiz = desktop.right - desktop.left;
	vert = desktop.bottom - desktop.top;

#elif __dest_os == __win32_os
	long horiz;
	long vert;

	CRect desktop;
	CWnd::GetDesktopWindow()->GetClientRect(desktop);
	horiz = desktop.Width();
	vert = desktop.Height();
#elif __dest_os == __linux_os
	JRect rect = JXGetApplication()->GetCurrentDisplay()->GetBounds();
	long horiz = rect.width();
	long vert = rect.height();
#else
#error __dest_os
#endif

	mWidth = horiz;
	mHeight = vert;
}

template<class T> cdstring CScreenDimensions<T>::GetInfo(void) const
{
	cdstring all = cdstring(mWidth);
	all += cSpace;
	all += cdstring(mHeight);
	all += cSpace;
	all += mData.GetInfo();

	// Put it all together
	return all;
}

template<class T> bool CScreenDimensions<T>::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	cdstring temp;
	txt.get(mWidth);
	txt.get(mHeight);

	return mData.SetInfo(txt, vers_prefs);
}

// Specialisation for cdstring
template<> cdstring CScreenDimensions<cdstring>::GetInfo(void) const
{
	cdstring all = cdstring(mWidth);
	all += cSpace;
	all += cdstring(mHeight);
	all += cSpace;

	// Must quote this
	cdstring temp(mData);
	temp.quote();
	all += temp;

	// Put it all together
	return all;
}

template<> bool CScreenDimensions<cdstring>::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mWidth);
	txt.get(mHeight);

	txt.get(mData);

	return true;
}

// Specialisation for Rect
template<> int CScreenDimensions<Rect>::operator==(const CScreenDimensions<Rect>& other) const
{
	return (mWidth == other.mWidth) &&
			(mHeight == other.mHeight) &&
			(mData.left == other.mData.left) &&
			(mData.top == other.mData.top) &&
			(mData.right == other.mData.right) &&
			(mData.bottom == other.mData.bottom);
}

template<> cdstring CScreenDimensions<Rect>::GetInfo(void) const
{
	cdstring all = cdstring(mWidth);
	all += cSpace;
	all += cdstring(mHeight);
	all += cSpace;

	all += cdstring((long) mData.left);
	all += cSpace;
	all += cdstring((long) mData.top);
	all += cSpace;
	all += cdstring((long) mData.right);
	all += cSpace;
	all += cdstring((long) mData.bottom);

	// Put it all together
	return all;
}

template<> bool CScreenDimensions<Rect>::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	cdstring temp;
	txt.get(mWidth);
	txt.get(mHeight);

	long ltemp;
	txt.get(ltemp);
	mData.left = ltemp;
	txt.get(ltemp);
	mData.top = ltemp;
	txt.get(ltemp);
	mData.right = ltemp;
	txt.get(ltemp);
	mData.bottom = ltemp;

	return true;
}

// Instantiate
template class CScreenDimensions<cdstring>;
template class CScreenDimensions<Rect>;
template class CScreenDimensions<CColumnInfoArray>;
