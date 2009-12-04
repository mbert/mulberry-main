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


// Header for CDrawUtils class

#ifndef __CDRAWUTILS__MULBERRY__
#define __CDRAWUTILS__MULBERRY__

 
// Classes
class JPainter;
class JRect;
class JXImage;

class CDrawUtils
{

public:
	static void DrawBackground(JPainter& p, const JRect& frame, bool selected, bool enabled);
	static void Draw3DBorder(JPainter& p, const JRect& rect, bool selected, bool enabled, bool frame = true, bool filled = true);
	static void DrawScrollBorder(JPainter& p, const JRect& rect, bool selected, bool enabled);
	static void DrawSimpleBorder(JPainter& p, const JRect& rect, bool selected, bool enabled, bool frame = true, bool filled = true);
	static void DrawSimpleLine(JPainter& p, const JRect& rect, bool vertical = true, bool selected = false, bool enabled = true);

private:

		CDrawUtils();
 		~CDrawUtils();
};

#endif
