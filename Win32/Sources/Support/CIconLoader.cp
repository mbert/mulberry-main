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


// CIconLoader : class to handle commonly used fonts in app

#include "CIconLoader.h"

CIconLoader CIconLoader::sIconLoader;

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
	BYTE	bWidth;               // Width of the image
	BYTE	bHeight;              // Height of the image (times 2)
	BYTE	bColorCount;          // Number of colors in image (0 if >=8bpp)
	BYTE	bReserved;            // Reserved
	WORD	wPlanes;              // Color Planes
	WORD	wBitCount;            // Bits per pixel
	DWORD	dwBytesInRes;         // how many bytes in this resource?
	WORD	nID;                  // the ID
} MEMICONDIRENTRY, *LPMEMICONDIRENTRY;
typedef struct 
{
	WORD			idReserved;   // Reserved
	WORD			idType;       // resource type (1 for icons)
	WORD			idCount;      // how many images?
	MEMICONDIRENTRY	idEntries[1]; // the entries for each image
} MEMICONDIR, *LPMEMICONDIR;
#pragma pack( pop )
typedef struct
{
	UINT			Width, Height, Colors; // Width, Height and bpp
	LPBYTE			lpBits;                // ptr to DIB bits
	DWORD			dwNumBytes;            // how many bytes?
	LPBITMAPINFO	lpbi;                  // ptr to header
	LPBYTE			lpXOR;                 // ptr to XOR image bits
	LPBYTE			lpAND;                 // ptr to AND image bits
} ICONIMAGE, *LPICONIMAGE;

HICON MakeIconFromResource( LPICONIMAGE lpIcon );
HICON MakeIconFromResource( LPICONIMAGE lpIcon )
{
    HICON        	hIcon = NULL;

    // Sanity Check
    if( lpIcon == NULL )
        return NULL;
    if( lpIcon->lpBits == NULL )
        return NULL;
    // Let the OS do the real work :)
    hIcon = CreateIconFromResourceEx( lpIcon->lpBits, lpIcon->dwNumBytes, TRUE, 0x00030000, 
            (*(LPBITMAPINFOHEADER)(lpIcon->lpBits)).biWidth, (*(LPBITMAPINFOHEADER)(lpIcon->lpBits)).biHeight/2, 0 );
    
    // It failed, odds are good we're on NT so try the non-Ex way
    if( hIcon == NULL )
    {
        // We would break on NT if we try with a 16bpp image
        if(lpIcon->lpbi->bmiHeader.biBitCount != 16)
        {	
            hIcon = CreateIconFromResource( lpIcon->lpBits, lpIcon->dwNumBytes, TRUE, 0x00030000 );
        }
    }
    return hIcon;
}

WORD DIBNumColors( LPSTR lpbi );
WORD DIBNumColors( LPSTR lpbi )
{
    WORD wBitCount;
    DWORD dwClrUsed;

    dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;

    if (dwClrUsed)
        return (WORD) dwClrUsed;

    wBitCount = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;

    switch (wBitCount)
    {
        case 1: return 2;
        case 4: return 16;
        case 8:	return 256;
        default:return 0;
    }
    return 0;
}

WORD PaletteSize( LPSTR lpbi );
WORD PaletteSize( LPSTR lpbi )
{
    return ( DIBNumColors( lpbi ) * sizeof( RGBQUAD ) );
}

LPSTR FindDIBBits( LPSTR lpbi );
LPSTR FindDIBBits( LPSTR lpbi )
{
   return ( lpbi + *(LPDWORD)lpbi + PaletteSize( lpbi ) );
}

#define WIDTHBYTES(bits)      ((((bits) + 31)>>5)<<2)

DWORD BytesPerLine( LPBITMAPINFOHEADER lpBMIH );
DWORD BytesPerLine( LPBITMAPINFOHEADER lpBMIH )
{
    return WIDTHBYTES(lpBMIH->biWidth * lpBMIH->biPlanes * lpBMIH->biBitCount);
}

BOOL AdjustIconImagePointers( LPICONIMAGE lpImage );
BOOL AdjustIconImagePointers( LPICONIMAGE lpImage )
{
    // Sanity check
    if( lpImage==NULL )
        return FALSE;
    // BITMAPINFO is at beginning of bits
    lpImage->lpbi = (LPBITMAPINFO)lpImage->lpBits;
    // Width - simple enough
    lpImage->Width = lpImage->lpbi->bmiHeader.biWidth;
    // Icons are stored in funky format where height is doubled - account for it
    lpImage->Height = (lpImage->lpbi->bmiHeader.biHeight)/2;
    // How many colors?
    lpImage->Colors = lpImage->lpbi->bmiHeader.biPlanes * lpImage->lpbi->bmiHeader.biBitCount;
    // XOR bits follow the header and color table
    lpImage->lpXOR = (LPBYTE)FindDIBBits((LPSTR)lpImage->lpbi);
    // AND bits follow the XOR bits
    lpImage->lpAND = lpImage->lpXOR + (lpImage->Height*BytesPerLine((LPBITMAPINFOHEADER)(lpImage->lpbi)));
    return TRUE;
}

HICON CIconLoader::GetIcon(CDC* pDC, UINT nID, UINT size)
{
	// Check for valid parameters
	if ((nID == 0) || (size != 16) && (size != 32))
		return NULL;
	
	// Determine color depth of DC
	int bpp = pDC->GetDeviceCaps(BITSPIXEL);
	if (bpp <= 4)
		bpp = 4;
	else if (bpp <= 8)
		bpp = 8;
	else
		bpp = 32;

	// Get appropriate icon
	return sIconLoader.GetIconFromCache(nID, size, bpp);
}

// DrawIcon
//	Draw appropriate icon using cached data - load into cache if not present
//
void CIconLoader::DrawIcon(CDC* pDC, int x, int y, UINT nID, UINT size)
{
	DrawState(pDC, x, y, nID, size, DSS_NORMAL);
}

void CIconLoader::DrawIcon(CDC* pDC, int x, int y, HICON hIcon, UINT size)
{
	DrawState(pDC, x, y, hIcon, size, DSS_NORMAL);
}

void CIconLoader::DrawState(CDC* pDC, int x, int y, UINT nID, UINT size, UINT state)
{
	// Get appropriate icon
	HICON icon = GetIcon(pDC, nID, size);
	if (icon)
	{
		pDC->DrawState(CPoint(x, y), CSize(size, size), icon, DST_ICON | state, (CBrush*) NULL);
	}
}

void CIconLoader::DrawState(CDC* pDC, int x, int y, HICON hIcon, UINT size, UINT state)
{
	pDC->DrawState(CPoint(x, y), CSize(size, size), hIcon, DST_ICON | state, (CBrush*) NULL);
}

// GetIconFromCache
//	Return icon from cached data - load into cache if not present
//
HICON CIconLoader::GetIconFromCache(UINT nID, UINT size, UINT colors)
{
	UINT index = (nID << 16) | ((colors & 0xFF) << 8) | (size & 0xFF);

	// Check whether currently in cache
	CIconCache::const_iterator found = mIconCache.find(index);
	if (found != mIconCache.end())
		return (*found).second;
	
	// Need to load it
	return LoadIconIntoCache(nID, size, colors);
}

// LoadIconIntoCache
//	Load icon into cache - assume its not already present
//
HICON CIconLoader::LoadIconIntoCache(UINT nID, UINT size, UINT colors)
{
	UINT index = (nID << 16) | ((colors & 0xFF) << 8) | (size & 0xFF);
	HICON icon = SmartLoadIcon(nID, size, colors);
	mIconCache.insert(CIconCache::value_type(index, icon));
	return icon;
}

// SmartLoadIcon
//	Load icon from resource using appropriate size and colour depth
//
HICON CIconLoader::SmartLoadIcon(UINT nID, UINT size, UINT colors)
{
	HICON result = NULL;

	HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(nID), RT_GROUP_ICON);
	if (hInst == NULL)
		return result;

	HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(nID), RT_GROUP_ICON);
	if (hResource == NULL)
		return result;
	
	HGLOBAL hGlobal = ::LoadResource(hInst, hResource);
	if (hGlobal == NULL)
		return result;

    LPMEMICONDIR lpIcon = (LPMEMICONDIR) ::LockResource(hGlobal);
    if (lpIcon == NULL)
    	return result;
    
    // Look at each icon
    UINT nID2 = 0;
    for(int i = 0; i < lpIcon->idCount; i++)
    {
    	LPMEMICONDIRENTRY lpIconEntry = &lpIcon->idEntries[i];
    	if ((lpIconEntry->bWidth == size) && (lpIconEntry->bHeight == size))
    	{
    		nID2 = lpIconEntry->nID;
	    	if (lpIconEntry->wBitCount == colors)
    			break;
    	}
    }

	if (nID2 != 0)
	{
		HINSTANCE hInst2 = AfxFindResourceHandle(MAKEINTRESOURCE(nID2), RT_ICON);
		if (hInst2 == NULL)
			return result;

		HRSRC hResource2 = ::FindResource(hInst2, MAKEINTRESOURCE(nID2), RT_ICON);
		if (hResource2 == NULL)
			return result;
		
		HGLOBAL hGlobal2 = ::LoadResource(hInst2, hResource2);
		if (hGlobal2 == NULL)
			return result;

		ICONIMAGE iconImage;
        iconImage.dwNumBytes = ::SizeofResource(hInst2, hResource2);
        iconImage.lpBits = (LPBYTE)::malloc(iconImage.dwNumBytes);
        ::memcpy(iconImage.lpBits, ::LockResource(hGlobal2), iconImage.dwNumBytes);
	    if (AdjustIconImagePointers(&iconImage))
	    {
	    	result = ::MakeIconFromResource(&iconImage);
	    }
		::free(iconImage.lpBits);
	}

	return result;

}