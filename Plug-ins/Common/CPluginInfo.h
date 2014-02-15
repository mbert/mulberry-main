/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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

// Plugin common information

// Common to all plugins
//long cPluginVersion =				0x04002001;				// v4.0d1
//long cPluginVersion =				0x04104003;				// v4.1a3
long cPluginVersion =				0x04106001;				// v4.1b1
//long cPluginVersion =				0x04088000;				// v4.0.8
const char* cPluginManufacturer =	"mulberrymail.com";

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define OS_ENDL "\r"
#elif __dest_os == __win32_os
#define OS_ENDL "\r\n"
#elif __dest_os == __linux_os
#define OS_ENDL "\n"
#endif
#define COPYRIGHT OS_ENDL OS_ENDL "Copyright Cyrus Daboo, 2006-2014."
