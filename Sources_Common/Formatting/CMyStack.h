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



#ifndef __CMYSTACK__MULBERRY__
#define __CMYSTACK__MULBERRY__

#include <iostream.h>


class CStackElement{
	public:
		CStackElement(char *);
		~CStackElement();
		
		CStackElement *next;
		char *value;
};


class CMyStack{
	public:
		CMyStack();
		~CMyStack();
		void deleteStack(CStackElement *);
		void push(char *);
		char *pop();
	private:
		CStackElement *stack;
};



class CParamStackElement{
	public:
		CParamStackElement(bool font, int size, bool color);
		~CParamStackElement();
		
		CParamStackElement *next;
		
		bool IsFont(){return font;}
		bool IsColor(){return color;}
		int Size(){return size;}
		
	private:
		bool font;
		int size;
		bool color;
};



class CMyParamStack{
	public:
		CMyParamStack();
		~CMyParamStack();
		void deleteStack(CParamStackElement *);
		void push(bool font, int size, bool color);
		CParamStackElement *pop();
	private:
		CParamStackElement *stack;
};

#endif
