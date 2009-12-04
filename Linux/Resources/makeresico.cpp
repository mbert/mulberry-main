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

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

void doIcon(const string& resid, const string& filename );

int main()
{
  string resid, filename, restype, discardable;
  cout << "#include <JXPM.h>" << endl;

  cin >> resid >> restype >> discardable >> filename;
  while (!cin.eof()) {
    if (restype == "ICON") {
      doIcon(resid, filename);
    } else {
      cerr << "Unknown restype (" << restype << " for Resource " << resid << endl;
    }
    cin >> resid >> restype >> discardable >> filename;
  }
}

static const char* cHexChar = "0123456789ABCDEF";

#if 0
void doIcon(const string& resid, const string& filename )
{
  string iconname, line;
  
  ifstream icofile(filename.c_str(), ios::in | ios::binary);
  if (!icofile.good()) {
    cerr << "Couldn't open " <<'"'<< filename <<'"'<< endl;
    return;
  }
  cout << "/* From file: " << filename << " */" << endl;
  iconname =  resid.substr(2, resid.length() - 2);
  cout << "static unsigned char " << iconname << "Data[] = {" << endl;
  
  unsigned char byte;
  byte = icofile.get();
  bool first = true;
  unsigned long ctr = 0;
  while (!icofile.eof()) {
  	if (first)
  		first = false;
  	else
  		cout << ", ";
  	if (ctr++ % 16 == 0)
  		cout << endl;
  	cout << "0x" << cHexChar[(byte >> 4) & 0x0F] << cHexChar[byte & 0x0F];
  	byte = icofile.get();
  }
  cout << "};" << endl;

  cout << "JICO " << iconname << "("<<iconname<<"Data);" << endl << endl;
}

#else

unsigned char SwapBytes(unsigned char x);
unsigned char SwapBytes(unsigned char x)
{
	return x;
}

unsigned short SwapBytes(unsigned short x);
unsigned short SwapBytes(unsigned short x)
{
	return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}

unsigned long SwapBytes(unsigned long x);
unsigned long SwapBytes(unsigned long x)
{
	return (((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24));
}

#pragma pack( push, 2 )
#pragma pack( 2 )
struct SIconEntry
{
	unsigned char	bWidth;               // Width of the image
	unsigned char	bHeight;              // Height of the image (times 2)
	unsigned char	bColorCount;          // Number of colors in image (0 if >=8bpp)
	unsigned char	bReserved;            // Reserved
	unsigned short	wPlanes;              // Color Planes
	unsigned short	wBitCount;            // Bits per pixel
	unsigned long	dwBytesInRes;         // how many bytes in this resource?
	unsigned long	dwOffset;             // the byte offset to the bitmap
};

struct SIconFamily
{
	unsigned short			idReserved;   // Reserved
	unsigned short			idType;       // resource type (1 for icons)
	unsigned short			idCount;      // how many images?
	SIconEntry				idEntries[1]; // the entries for each image
};
#pragma pack( pop )

struct SBitmapInfoHeader
{
  unsigned long   biSize; 
  unsigned long   biWidth; 
  unsigned long   biHeight; 
  unsigned short  biPlanes; 
  unsigned short  biBitCount; 
  unsigned long   biCompression; 
  unsigned long   biSizeImage; 
  unsigned long   biXPelsPerMeter; 
  unsigned long   biYPelsPerMeter; 
  unsigned long   biClrUsed; 
  unsigned long   biClrImportant; 
}; 

struct RGBQUAD
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char reserved;
};

unsigned long DIBNumColors( SBitmapInfoHeader* lpbi );
unsigned long DIBNumColors( SBitmapInfoHeader* lpbi )
{
    unsigned long wBitCount;
    unsigned long dwClrUsed;

    dwClrUsed = SwapBytes(lpbi->biClrUsed);

    if (dwClrUsed)
        return dwClrUsed;

    wBitCount = SwapBytes(lpbi->biBitCount);

    switch (wBitCount)
    {
        case 1: return 2;
        case 4: return 16;
        case 8:	return 256;
        default:return 0;
    }
    return 0;
}

unsigned long PaletteSize( SBitmapInfoHeader* lpbi );
unsigned long PaletteSize( SBitmapInfoHeader* lpbi )
{
    return ( DIBNumColors( lpbi ) * sizeof( RGBQUAD ) );
}

unsigned char* FindDIBBits( SBitmapInfoHeader* lpbi );
unsigned char* FindDIBBits( SBitmapInfoHeader* lpbi )
{
   return (unsigned char*)(lpbi) + SwapBytes(lpbi->biSize) + PaletteSize( lpbi );
}

#define WIDTHBYTES(bits)      ((((bits) + 31)>>5)<<2)

unsigned long BytesPerLine( SBitmapInfoHeader* lpbi );
unsigned long BytesPerLine( SBitmapInfoHeader* lpbi )
{
    return WIDTHBYTES(SwapBytes(lpbi->biWidth) * SwapBytes(lpbi->biPlanes) * SwapBytes(lpbi->biBitCount));
}

void WriteHex(unsigned char c);
void WriteHex(unsigned char c)
{
	cout << cHexChar[(c >> 4) & 0x0F] << cHexChar[c & 0x0F];
}

void WriteHex(unsigned long l);
void WriteHex(unsigned long l)
{
	cout << "0x";
	WriteHex((unsigned char)((l >> 24) & 0xFF));
	WriteHex((unsigned char)((l >> 16) & 0xFF));
	WriteHex((unsigned char)((l >> 8) & 0xFF));
	WriteHex((unsigned char)((l) & 0xFF));
	cout << ", ";
}

void doIcon(const string& resid, const string& filename )
{
	string iconname, line;

	ifstream icofile(filename.c_str(), ios::in | ios::binary);
	if (!icofile.good()) {
		cerr << "Couldn't open " <<'"'<< filename <<'"'<< endl;
		return;
	}

	// Read all data into buffer
	const unsigned long cBufferSize = 30 * 1024;
	unsigned char* buffer = new unsigned char [cBufferSize];
	icofile.read((char*)buffer, cBufferSize);

	// Get pointer to family structure
	SIconFamily* family = (SIconFamily*)(buffer);
	if (SwapBytes(family->idType) != 1)
	{
		cerr << "Could not find icons in \"" << filename << "\"" << endl;
		return;
	}

	// Count entries with 32-bit depth
	vector<unsigned long> entry_count;
	for(int i = 0; i < SwapBytes(family->idCount); i++)
	{
		// Find one that matches the size and is not a 16-colour variant
		SIconEntry* entry = &family->idEntries[i];

		// Locate bitmap header
		SBitmapInfoHeader* hdr = reinterpret_cast<SBitmapInfoHeader*>(((unsigned char*)family) + SwapBytes(entry->dwOffset));
		if (SwapBytes(hdr->biBitCount) == 0x20)
			entry_count.push_back(SwapBytes(entry->bWidth));
	}

	// Do nothing if no 32-bit icon
	if (entry_count.size() == 0)
	{
		cerr << "Could not find 32-bit icons in \"" << filename << "\"" << endl;
		return;
	}

	// Write header for data
	cout << "/* From file: " << filename << " */" << endl;
	iconname =  resid.substr(2, resid.length() - 2);
	cout << "static unsigned long " << iconname << "Data[] = {" << endl;
	
	// Write length *size
	WriteHex((unsigned long) entry_count.size());
	for(vector<unsigned long>::const_iterator iter = entry_count.begin(); iter != entry_count.end(); iter++)
	{
		WriteHex(*iter);
	}
	
	// Write each entry
	for(int i = 0; i < SwapBytes(family->idCount); i++)
	{
		// Find one that matches the size and is not a 16-colour variant
		SIconEntry* entry = &family->idEntries[i];

		// Locate bitmap header
		SBitmapInfoHeader* hdr = reinterpret_cast<SBitmapInfoHeader*>(((unsigned char*)family) + SwapBytes(entry->dwOffset));
		if (SwapBytes(hdr->biBitCount) == 0x20)
		{
			cout << endl;

	    	unsigned long size = SwapBytes(entry->bWidth);
	    	unsigned long bpl = BytesPerLine(hdr);
			unsigned char* lpXOR = FindDIBBits(hdr);
			unsigned long ctr = 0;
			for(unsigned long y = 0; y < size; y++)
			{
				for(unsigned long x = 0; x < size; x++)
				{
					if (ctr++ % 4 == 0)
						cout << endl;

					unsigned long pixel = *(unsigned long*)(lpXOR + bpl * (size - 1 - y) + x * 4);
					pixel = SwapBytes(pixel);
					WriteHex(pixel);
				}
			}
		}
	}
	
	// Write trailer
	cout << endl << "0x00, 0x00, 0x00, 0x00};" << endl;

	cout << "JICO " << iconname << "("<<iconname<<"Data);" << endl << endl;
}

#endif
