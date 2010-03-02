/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CIconLoader : class to handle common fonts

#ifndef __CICONLOADER__MULBERRY__
#define __CICONLOADER__MULBERRY__

class CPreferences;

class CIconLoader
{
public:
	static CIconLoader sIconLoader;
	
	static HICON	GetIcon(UINT nID, UINT size, UINT colors)
		{ return sIconLoader.GetIconFromCache(nID, size, colors); }

	static HICON	GetIcon(CDC* pDC, UINT nID, UINT size);

	static void DrawIcon(CDC* pDC, int x, int y, UINT nID, UINT size);
	static void DrawIcon(CDC* pDC, int x, int y, HICON hIcon, UINT size);
	static void DrawState(CDC* pDC, int x, int y, UINT nID, UINT size, UINT state);
	static void DrawState(CDC* pDC, int x, int y, HICON hIcon, UINT size, UINT state);

private:
	typedef std::map<UINT, HICON> CIconCache;
	
	CIconCache		mIconCache;
	
	HICON			GetIconFromCache(UINT nID, UINT size, UINT colors);
	
	HICON			LoadIconIntoCache(UINT nID, UINT size, UINT colors);
	HICON			SmartLoadIcon(UINT nID, UINT size, UINT colors);

	CIconLoader() {}
	~CIconLoader() {}
};

#endif
