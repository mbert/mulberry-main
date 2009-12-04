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


// Source for malloc debugging

#include "memdebug.h"

#if __dest_os != __linux_os
#include <pool_alloc.h>
#else
#define	__sys_alloc	malloc
#define __sys_free	free
#endif
#include <assert.h>
#include <string.h>
#include <stdlib.h>

const unsigned long cTracebackSize = 10UL;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define OS_ENDL			"\r"
#define DO_TRACEBACK_NAMES	1
#elif __dest_os == __win32_os
#define OS_ENDL			"\r\n"
#define DO_TRACEBACK_NAMES	0
#elif __dest_os == __linux_os
#define OS_ENDL			"\n"
#define DO_TRACEBACK_NAMES	0
#endif

//#define DONT_FREE_BLOCKS
#define VALIDITY_CHECKS		false
#define PTR_CHECKS			false

enum EMemoryBlockFlags
{
	eBlock_Init   = 0,
	eBlock_InUse = 1UL << 0,
	eBlock_Free  = 1UL << 1,
	eBlock_Leak  = 1UL << 2
};

struct SMemoryBlock;
struct SMemoryInfoBlock
{
	unsigned long	mAllocNum;						// Numeric allocation number
	unsigned long	mFlags;						// Leak check
	unsigned long	mSize;							// Requested size
	unsigned long	mStackTrace[cTracebackSize];	// Stack trace
	SMemoryBlock*	mNext;							// Next in list
	SMemoryBlock*	mPrevious;						// Previous in list
	void*			mDeadBeef;						// Boundary
};


struct SMemoryBlock
{
	SMemoryInfoBlock	mInfo;			// Info block
	void*				mMem;			// Actual memory allocted
};

static bool				sTrackMemory = true;
static bool				sValidityChecks = VALIDITY_CHECKS;
static bool				sPtrChecks = PTR_CHECKS;
static unsigned long	sAllocCtr = 0;
static SMemoryBlock*	sHeadBlock = NULL;
static SMemoryBlock*	sTailBlock = NULL;

const unsigned long		cCheckBlocksSize = 10UL;
static void*			sCheckBlocks[cCheckBlocksSize] = {NULL};

void* const deadbeef = (void*)0xDEADBEEF;
void* const deadbeee = (void*)0xDEADBEEE;

void register_memoryblock(SMemoryBlock* block);
void unregister_memoryblock(SMemoryBlock* block);
void block_error();
void ptr_checks();
void ptr_check(void* ptr);

void register_memoryblock(SMemoryBlock* block)
{
	if (!sHeadBlock)
	{
		sHeadBlock = block;
		sTailBlock = block;
	}
	else
	{
		sTailBlock->mInfo.mNext = block;
		block->mInfo.mPrevious = sTailBlock;
		sTailBlock = block;
	}
}

void unregister_memoryblock(SMemoryBlock* block)
{
	// Link next block to the previous block
	if (block->mInfo.mNext)
		block->mInfo.mNext->mInfo.mPrevious = block->mInfo.mPrevious;
	
	// Link previous block to the next block
	if (block->mInfo.mPrevious)
		block->mInfo.mPrevious->mInfo.mNext = block->mInfo.mNext;
	
	// If current block is the head, set the head to the next block
	if (sHeadBlock == block)
		sHeadBlock = block->mInfo.mNext;
	
	// If current block is the tail, set the tail to the previous block
	if (sTailBlock == block)
		sTailBlock = block->mInfo.mPrevious;
}

void block_error()
{
	bool result = true;
	result = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::DebugStr("\pmalloc deadbeef error.");
#else
	assert(false);
#endif
}

void ptr_checks()
{
	for(unsigned long i = 0; i < cCheckBlocksSize; i++)
	{
		if (sCheckBlocks[i] != NULL)
			ptr_check(sCheckBlocks[i]);
	}
}

void ptr_check(void* ptr)
{
	// Bump pointer back to start of info
	SMemoryBlock* block = (SMemoryBlock*) (((unsigned long) ptr) - sizeof(SMemoryInfoBlock));
	
	// Verify markers
	if ((block->mInfo.mFlags & eBlock_InUse) != 0)
	{
		if (block->mInfo.mDeadBeef != deadbeef)
			block_error();
		else if (*((void**) ((char*) ptr + block->mInfo.mSize)) != deadbeef)
			block_error();
	}
	else if ((block->mInfo.mFlags & eBlock_Free) != 0)
	{
		if (block->mInfo.mDeadBeef != deadbeee)
			block_error();
		else if (*((void**) ((char*) ptr + block->mInfo.mSize)) != deadbeee)
			block_error();
	}
	else
		block_error();
}

void start_leak_check()
{
	// Clear leaked flag on all blocks
	SMemoryBlock* block = sHeadBlock;
	while(block)
	{
		block->mInfo.mFlags &= ~eBlock_Leak;
		block = block->mInfo.mNext;
	}
}

void report_leaks(const char* title)
{
	// Must turn off memory tracking whilst report leaks as memory allocations
	// during this function could cause corruption of the loop
	sTrackMemory = false;

	char buffer[1024];
	FILE* fout = ::fopen("leaks", "a+");
	
	// Write title
	::sprintf(buffer, "-------------------"OS_ENDL"Leak report: %s"OS_ENDL, title);
	::fwrite(buffer, 1, ::strlen(buffer), fout);

	// Look for leak flag in all blocks
	unsigned long leak_ctr = 0;
	unsigned long total_size = 0;
	SMemoryBlock* block = sHeadBlock;
	while(block)
	{
		if ((block->mInfo.mFlags & eBlock_Leak) != 0)
		{
			// Update summary info
			leak_ctr++;
			total_size += block->mInfo.mSize;

			// Report the leak
			::sprintf(buffer, OS_ENDL"Leak at 0x%p size 0x%08lX number %ld"OS_ENDL, &block->mMem, block->mInfo.mSize, block->mInfo.mAllocNum);
			::fwrite(buffer, 1, ::strlen(buffer), fout);
			
			// Report stack
			for(unsigned long i = 0; i < cTracebackSize; i++)
			{
				// Output program counter of caller
				unsigned long ip = block->mInfo.mStackTrace[i];
				::sprintf(buffer, "  Frame #%ld at 0x%08lX: ", i, ip);
				::fwrite(buffer, 1, ::strlen(buffer), fout);
				
#if DO_TRACEBACK_NAMES
				if (ip & 0xFF000000)
				{
					// Look for traceback table
					while(*(unsigned long*)ip != 0x4E800020)
						ip++;
					
					// Bump over space before traceback
					ip += 20;
					
					// Now pointing to pascal string of routing name
					unsigned short ctr = *(unsigned short*)ip;
					if (ctr > 1000)
						ctr = 1000;
					ip += 2;
					int j = 0;
					for(; j < ctr; j++)
					{
						buffer[j] = *(char*)ip;
						ip++;
					}
					buffer[j] = 0;
					::fwrite(buffer, 1, ::strlen(buffer), fout);
				}
#endif

				::sprintf(buffer, OS_ENDL);
				::fwrite(buffer, 1, ::strlen(buffer), fout);
				::fflush(fout);
			}
		}
		
		block = block->mInfo.mNext;
	}
	
	// Write summary info
	::sprintf(buffer, OS_ENDL OS_ENDL "SUMMARY"OS_ENDL"Number of leaks: %ld"OS_ENDL"Total size of leaks: %ld"OS_ENDL, leak_ctr, total_size);
	::fwrite(buffer, 1, ::strlen(buffer), fout);

	::fclose(fout);
	
	// Turn on memory tracking again
	sTrackMemory = true;
}

void valid_blocks()
{
	// Scan all blocks looking for invalid markers
	SMemoryBlock* block = sHeadBlock;
	SMemoryBlock* last_block = NULL;
	unsigned long last_alloc = 0;
	while(block)
	{
		// Verify markers
		if (block->mInfo.mDeadBeef != deadbeef)
			block_error();
		else if (*((void**) ((char*) block + sizeof(SMemoryInfoBlock) + block->mInfo.mSize)) != deadbeef)
			block_error();
		
		// Verify links
		if (block->mInfo.mNext && (block->mInfo.mNext->mInfo.mPrevious != block))
			block_error();

		last_block = block;
		last_alloc = block->mInfo.mAllocNum;
		block = block->mInfo.mNext;
	}
}

static unsigned long _mutex = 0;
#if __dest_os == __linux_os
void* debug_malloc(size_t size)
#else
void* std::malloc(size_t size)
#endif
{
	// Crude mutex
	while(_mutex != 0) {}
	_mutex++;
	if (_mutex > 1)
		block_error();

	// Do block validity checks if requested
	if (sValidityChecks)
		valid_blocks();

	// Do ptr checks if requested
	if (sPtrChecks)
		ptr_checks();

	// Add extra length for block info
	size_t actual_size = size + sizeof(SMemoryInfoBlock) + sizeof(void*);
	
	// Create block using default malloc - exit immediately if NULL
	SMemoryBlock* block = (SMemoryBlock*) __sys_alloc(actual_size);
	if (block == NULL)
	{
		_mutex--;
		return NULL;
	}
	
	// Initialise info - alloc num os zero if not tracking blocks
	block->mInfo.mAllocNum = sTrackMemory ? ++sAllocCtr : 0;
	block->mInfo.mFlags = eBlock_Leak | eBlock_InUse;
	block->mInfo.mSize = size;
	block->mInfo.mNext = NULL;
	block->mInfo.mPrevious = NULL;
	block->mInfo.mDeadBeef = deadbeef;
	
	// Set trailing marker
	*((void**) ((char*) block + actual_size - sizeof(unsigned long))) = deadbeef;

	// Register block for leak detection
	if (sTrackMemory)
		register_memoryblock(block);

	// Generate stack backtrace
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	register unsigned long stackptrreg;
	asm
	{
		mr	stackptrreg, SP
	}
	for(unsigned long i = 0; i < cTracebackSize; i++)
	{
		if (stackptrreg)
		{
			// Do check for valid stack chain move. For some reason we do not get a 
			// NULL stack at the top of the chain in a thread, so we need to make sure
			// the next link in the chain is not too far away from the current one as
			// a sanity check.
			unsigned long old_stackptrreg = stackptrreg;
			stackptrreg = *(unsigned long*)stackptrreg;
			if (stackptrreg && (stackptrreg - old_stackptrreg > 0x10000))
				stackptrreg = 0;
		}
		if (stackptrreg)
			block->mInfo.mStackTrace[i] = *(unsigned long*)(stackptrreg + 8);
		else
			block->mInfo.mStackTrace[i] = 0;
	}
#elif __dest_os == __win32_os
	register unsigned long* framePtr;
	asm
	{
		MOV framePtr, EBP
	}
	for(unsigned long i = 0; i < cTracebackSize; i++)
	{
		if (framePtr != 0)
			block->mInfo.mStackTrace[i] = framePtr[1];
		else
			block->mInfo.mStackTrace[i] = 0;
		if (framePtr != 0)
			framePtr = (unsigned long*) framePtr[0];
		if ((framePtr != 0) && IsBadReadPtr(framePtr, 8))
			framePtr = 0;
	}
#elif __dest_os == __linux_os
	for(unsigned long i = 0; i < cTracebackSize; i++)
	{
		block->mInfo.mStackTrace[i] = 0;
	}
#endif
	
	// Bump past info section to return actual block memory
	_mutex--;
	return &block->mMem;
}

#if __dest_os == __linux_os
void* debug_calloc(size_t count, size_t size)
#else
void* std::calloc(size_t count, size_t size)
#endif
{
	void* result = std::malloc(count * size);
	if (result != NULL)
		::memset(result, 0, count * size);
	return result;
}

#if __dest_os == __linux_os
void* debug_realloc(void *ptr, size_t size)
#else
void* std::realloc(void *ptr, size_t size)
#endif
{
	// Allocate new space
	void* result = std::malloc(size);
	if (result == NULL)
		return NULL;
	
	if (ptr != NULL)
	{
		// Bump pointer back to start of info
		SMemoryBlock* block = (SMemoryBlock*) (((unsigned long) ptr) - sizeof(SMemoryInfoBlock));
		
		// Copy over original data
		::memcpy(result, ptr, block->mInfo.mSize);
		
		// Free the old one
		::free(ptr);
	}
	
	// Return the new one
	return result;
}

#if __dest_os == __linux_os
void debug_free(void* ptr)
#else
void std::free(void* ptr)
#endif
{
	if (ptr != 0)
	{
		// Crude mutex
		while(_mutex != 0) {}
		_mutex++;
		if (_mutex > 1)
			block_error();

		// Do block validity checks if requested
		if (sValidityChecks)
			valid_blocks();

		// Do ptr checks if requested
		if (sPtrChecks)
			ptr_checks();

		// Bump pointer back to start of info
		SMemoryBlock* block = (SMemoryBlock*) (((unsigned long) ptr) - sizeof(SMemoryInfoBlock));
		
		// Verify markers
		if (block->mInfo.mDeadBeef != deadbeef)
			block_error();
		else if (*((void**) ((char*) ptr + block->mInfo.mSize)) != deadbeef)
			block_error();

		// Set marker to another error value to trap duplicate frees
		block->mInfo.mFlags = eBlock_Free;
		block->mInfo.mDeadBeef = deadbeee;
		*((void**) ((char*) ptr + block->mInfo.mSize)) = deadbeee;

		// Unregister from leak detector if it was registered before
		if (block->mInfo.mAllocNum != 0)
			unregister_memoryblock(block);

		// Do ptr checks if requested
		if (sPtrChecks)
			ptr_checks();

		// Free using default free
		_mutex--;
#ifndef DONT_FREE_BLOCKS
		__sys_free(block);
#endif
	}	
}

#if __dest_os == __mac_os_x
void * __sys_alloc(size_t size)
{
	return NewPtr(size);
}

void __sys_free(void *ptr)
{
	DisposePtr((Ptr)ptr);
}
#endif
