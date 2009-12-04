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


#include "RGB.h"

#include <cstring>
#include <iostream>


RGB::RGB()
{
}

RGB::RGB(int red, int green, int blue){
	RGB::red = red;
	RGB::green = green;
	RGB::blue = blue;
}

bool RGB::setColor(int red, int green, int blue){
	RGB::red = red;
	RGB::green = green;
	RGB::blue = blue;
	
	return true;
}

bool RGB::setColor(char *color){
	if(strcmp(color, "red")){
		red = 256;
		green = 0;
		blue = 0;
	}
	return true;
}

int RGB::getRed(){
	return red;
}

int RGB::getGreen(){
	return green;
}

int RGB::getBlue(){
	return blue;
}
