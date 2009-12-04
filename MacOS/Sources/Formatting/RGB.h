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


#ifndef __RGB__MULBERRY__
#define __RGB__MULBERRY__

class RGB{
	public:
		RGB();
		RGB(int, int, int);
		bool setColor(int, int, int);
		bool setColor(char *);
		int getRed();
		int getGreen();
		int getBlue();
	private:
		unsigned short red;
		unsigned short green;
		unsigned short blue;
};

#endif
