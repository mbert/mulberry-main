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


#include "CMyStack.h"
#include <iostream.h>

CStackElement::CStackElement(char *string){
	value = string;
	next = NULL;
}

CStackElement::~CStackElement(){
}


CMyStack::CMyStack(){
	stack = NULL;
}

CMyStack::~CMyStack(){
	deleteStack(stack);
}

void CMyStack::deleteStack(CStackElement *member){
	if(member){
		deleteStack(member->next);
		delete member;
	}
}

void CMyStack::push(char *string){
	CStackElement *element = new CStackElement(string);
	element->next = stack;
	stack = element;
}

char *CMyStack::pop(){
	CStackElement *give = stack;
	if(stack)
		stack = stack->next;
	else
		stack = NULL;

	char* result = give->value;
	delete give;

	return result;
}


CParamStackElement::CParamStackElement(bool f, int s, bool c){
	font = f;
	size = s;
	color = c;
	next = NULL;
}

CParamStackElement::~CParamStackElement(){
}


CMyParamStack::CMyParamStack(){
	stack = NULL;
}

CMyParamStack::~CMyParamStack(){
	deleteStack(stack);
}

void CMyParamStack::deleteStack(CParamStackElement *member){
	if(member){
		deleteStack(member->next);
		delete member;
	}
}

void CMyParamStack::push(bool face, int size, bool color){
	CParamStackElement *element = new CParamStackElement(face, size, color);
	element->next = stack;
	stack = element;
}

CParamStackElement *CMyParamStack::pop(){
	CParamStackElement *give = stack;
	if(stack)
		stack = stack->next;
	else
		stack = NULL;

	return give;
}