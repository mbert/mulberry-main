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


//	This class defines a header and footer class. This is essentially an
//	edit field with a specialized PrintPanelSelf() member function.

#include "CHeadAndFoot.h"

// ---------------------------------------------------------------------------
//		¥ CHeadAndFoot
// ---------------------------------------------------------------------------
//	The default constructor sets any data members.

CHeadAndFoot::CHeadAndFoot(bool use_box)
{
	mFileName[0] = 0;
	mHasBox = use_box;
}

// ---------------------------------------------------------------------------
//		¥ CHeadAndFoot
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor constructs the base class and sets
//	any data members.


CHeadAndFoot::CHeadAndFoot(LStream *inStream)
	: LEditField(inStream)
{
	mFileName[0] = 0;

	UseWordWrap(true);
}

// ---------------------------------------------------------------------------
//		¥ ~CHeadAndFoot
// ---------------------------------------------------------------------------
//	The default destructor does nothing.

CHeadAndFoot::~CHeadAndFoot()
{
}

// ---------------------------------------------------------------------------
//		¥ SetFileName
// ---------------------------------------------------------------------------
//	We store the name of the file in a data member in case we want to
//	print it in the header or the footer. This is kind of kludgy, but
//	it's cheap and it works.

void
CHeadAndFoot::SetFileName(ConstStr255Param inFileName)
{
	::BlockMoveData(inFileName, mFileName, StrLength(inFileName) + 1);
}

// ---------------------------------------------------------------------------
//		¥ PrintPanelSelf
// ---------------------------------------------------------------------------
//	Print the header and footer. You can use special characters to print
//	page numbers or the file name:
//		%f	File name
//		%p	Page Number
//		%h	Horizontal panel number
//		%v	Vertical panel number
//		%%	just a %

void
CHeadAndFoot::PrintPanelSelf(
	const PanelSpec	&inPanel)
{
	Str255	saveText;
	Str255	replaceText;
	Str255	tmpString;
	short	i, j;

	GetDescriptor(saveText);

		//	Parse the header or footer string, looking for
		//	our special escape character.
	for (i = j = 1; i <= saveText[0]; i++, j++) {
		if (saveText[i] != '%')
			replaceText[j] = saveText[i];
		else {
			UInt32	theInt = 0;

			if (++i > saveText[0])
				break;

			switch (saveText[i]) {
				case '%':
					tmpString[0] = 1;
					tmpString[1] = '%';
					break;

				case 'f':
					::BlockMoveData(mFileName, tmpString, StrLength(mFileName) + 1);
					break;

				case 'p':
					theInt = inPanel.pageNumber;
					break;

				case 'h':
					theInt = inPanel.horizIndex;
					break;

				case 'v':
					theInt = inPanel.vertIndex;
					break;

				default:
					tmpString[0] = 2;
					tmpString[1] = '%';
					tmpString[2] = saveText[i];
					break;
			}

			if (theInt != 0)
				::NumToString(theInt, tmpString);

			::BlockMoveData(tmpString + 1, replaceText + j, StrLength(tmpString));
			j += StrLength(tmpString) - 1;
		}
	}

	replaceText[0] = j - 1;

		//	Change our string to the one we just made up

	SetDescriptor(replaceText);

		//	Let the normal drawing routines do all the work

	DrawSelf();

		//	But be sure to set things back to the way they were

	SetDescriptor(saveText);
}
