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


// Header for CScreenDimensions class

#ifndef __CSCREENDIMENSIONS__MULBERRY__
#define __CSCREENDIMENSIONS__MULBERRY__

#include "cdstring.h"

template<class T> class CScreenDimensions
{
public:
	CScreenDimensions()
		{ GetCurrentScreen(); }
	CScreenDimensions(const T& data)
		{ GetCurrentScreen(); mData = data; }
	CScreenDimensions(long width, long height, T& data)
		{ mWidth = width; mHeight = height; mData = data; }
	CScreenDimensions(const CScreenDimensions& copy)
		{ _copy(copy); }
	~CScreenDimensions() {}

	int operator==(const CScreenDimensions& other) const;			// Compare with same type
	int operator<(const CScreenDimensions& other) const			// Compare with same type
		{ if (mWidth != other.mWidth) return mWidth < other.mWidth;
		  return mHeight < other.mHeight; }

	CScreenDimensions& operator=(const CScreenDimensions& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	long GetWidth() const
		{ return mWidth; }
	long GetHeight() const
		{ return mHeight; }

	const T& GetData() const
		{ return mData; }

	cdstring GetInfo(void) const;
	bool SetInfo(char_stream& txt, NumVersion vers_prefs);

private:
	long mWidth;
	long mHeight;
	T mData;

	void _copy(const CScreenDimensions& copy)
		{ mWidth = copy.mWidth; mHeight = copy.mHeight; mData = copy.mData; }

	void GetCurrentScreen();									// get the current screen dimensions
};

#endif
