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

#undef __std
#define __std(ref) ref
#include "ansi_files.h"
#include "file_io.h"

#ifdef __cplusplus                  /*hh 971206  namespace support*/
	extern "C" {
#endif

void thread_yield_proc();
void thread_yield_proc()
{
	YieldToAnyThread();
}

void __init_file(FILE * file, __file_modes mode, char * buff, size_t size)
{
	file->handle            = 0;
	file->mode              = mode;
	file->state.io_state    = __neutral;
	file->state.free_buffer = 0;
	file->state.eof         = 0;
	file->state.error       = 0;
	file->position          = 0;
	
	if (size)
		setvbuf(file, buff, _IOFBF, size);
	else
		setvbuf(file, 0,    _IONBF, 0);
	
	file->buffer_ptr = file->buffer;
	file->buffer_len = 0;
	
	switch (file->mode.file_kind)
	{
#ifndef _No_Disk_File_OS_Support
		case __disk_file:
			file->position_proc    = __position_file;
			file->read_proc        = __read_file;
			file->write_proc       = __write_file;
			file->close_proc       = __close_file;
			break;
#endif /* ndef _No_Disk_File_OS_Support */
	}
	
	__set_idle_proc(file, thread_yield_proc);
}

#ifdef __cplusplus                  /*hh 971206  expanded __extern macro*/
	}
#endif
