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

#include "CMachOFunctions.h"

#define PGP_MACINTOSH	1
#define PGP_DEBUG		0

#include "pgpFeatures.h"
#include "pgpEncode.h"
#include "pgpRandomPool.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"

#include <stdarg.h>

extern CMachOLoader* sPGPSDKLoader;

#pragma mark ______________________PGPSDK

//PGPError PGPAddJobOptions(PGPJobRef job, PGPOptionListRef firstOption, ...);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPAddJobOptions, (PGPJobRef job, PGPOptionListRef firstOption, ...), (job, firstOption, __va_start(firstOption)))

//PGPError PGPAddKeys(PGPKeySetRef keysToAdd, PGPKeySetRef set);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPAddKeys, (PGPKeySetRef keysToAdd, PGPKeySetRef set), (keysToAdd, set))

//PGPError PGPCountKeys(PGPKeySetRef keys, PGPUInt32 *numKeys);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPCountKeys, (PGPKeySetRef keys, PGPUInt32 *numKeys), (keys, numKeys))

//PGPError PGPEncode(PGPContextRef context, PGPOptionListRef firstOption, ...);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPEncode, (PGPContextRef context, PGPOptionListRef firstOption, ...), (context, firstOption, __va_start(firstOption)))

//PGPError PGPDecode(PGPContextRef context, PGPOptionListRef firstOption, ...);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPDecode, (PGPContextRef context, PGPOptionListRef firstOption, ...), (context, firstOption, __va_start(firstOption)))

//PGPError PGPFilterKeyDB(PGPKeyDBRef keyDB, PGPFilterRef filter, PGPKeySetRef *resultSet);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFilterKeyDB, (PGPKeyDBRef keyDB, PGPFilterRef filter, PGPKeySetRef *resultSet), (keyDB, filter, resultSet))

//PGPError PGPFreeContext(PGPContextRef context);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeContext, (PGPContextRef context), (context))

//PGPError PGPFreeData(void *allocation);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeData, (void *allocation), (allocation))

//PGPError PGPFreeFileSpec(PGPFileSpecRef fileRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeFileSpec, (PGPFileSpecRef fileRef), (fileRef))

//PGPError PGPFreeFilter(PGPFilterRef filter);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeFilter, (PGPFilterRef filter), (filter))

//PGPError PGPFreeKeyDB(PGPKeyDBRef keyDBRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeKeyDB, (PGPKeyDBRef keyDBRef), (keyDBRef))

//PGPError PGPFreeKeyIter(PGPKeyIterRef iter);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeKeyIter, (PGPKeyIterRef iter), (iter))

//PGPError PGPFreeKeyList(PGPKeyListRef keys);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeKeyList, (PGPKeyListRef keys), (keys))

//PGPError PGPFreeKeySet(PGPKeySetRef keys);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFreeKeySet, (PGPKeySetRef keys), (keys))

//PGPError 	PGPFindKeyByKeyID( PGPKeyDBRef keyDBRef, const PGPKeyID * keyID, PGPKeyDBObjRef *keyRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPFindKeyByKeyID, (PGPKeyDBRef keyDBRef, const PGPKeyID * keyID, PGPKeyDBObjRef *keyRef), (keyDBRef, keyID, keyRef))

//PGPError PGPGetErrorString(PGPError theError, PGPSize bufferSize, char * theString);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPGetErrorString, (PGPError theError, PGPSize bufferSize, char * theString), (theError, bufferSize, theString))

//PGPError 	PGPGetKeyDBObjAllocatedDataProperty( PGPKeyDBObjRef key, PGPKeyDBObjProperty whichProperty, void **buffer, PGPSize *dataSize);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPGetKeyDBObjAllocatedDataProperty, (PGPKeyDBObjRef key, PGPKeyDBObjProperty whichProperty, void **buffer, PGPSize *dataSize), (key, whichProperty, buffer, dataSize))

//PGPError PGPGetKeyDBObjNumericProperty(PGPKeyDBObjRef key, PGPKeyDBObjProperty whichProperty, PGPInt32 *prop);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPGetKeyDBObjNumericProperty, (PGPKeyDBObjRef key, PGPKeyDBObjProperty whichProperty, PGPInt32 *prop), (key, whichProperty, prop))

//PGPError PGPGetKeyID(PGPKeyDBObjRef key, PGPKeyID *keyID);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPGetKeyID, (PGPKeyDBObjRef key, PGPKeyID *keyID), (key, keyID))

//PGPError PGPGetKeyIDString(PGPKeyID const * ref, PGPKeyIDStringType type, char outString[ kPGPMaxKeyIDStringSize ]);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPGetKeyIDString, (PGPKeyID const * ref, PGPKeyIDStringType type, char outString[ kPGPMaxKeyIDStringSize ]), (ref, type, outString))

//PGPUInt32 PGPGetPGPsdkAPIVersion(void);
IMPORT_FUNCTION(sPGPSDKLoader, PGPUInt32, PGPGetPGPsdkAPIVersion, (void), ())

//PGPError PGPGetPrimaryUserIDName(PGPKeyDBObjRef key, void *buffer, PGPSize bufferSize, PGPSize *dataSize);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPGetPrimaryUserIDName, (PGPKeyDBObjRef key, void *buffer, PGPSize bufferSize, PGPSize *dataSize), (key, buffer, bufferSize, dataSize))

//PGPBoolean PGPGlobalRandomPoolHasMinimumEntropy(void);
IMPORT_FUNCTION(sPGPSDKLoader, PGPBoolean, PGPGlobalRandomPoolHasMinimumEntropy, (void), ())

//PGPError PGPIntersectFilters(PGPFilterRef filter1, PGPFilterRef filter2, PGPFilterRef *outFilter);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPIntersectFilters, (PGPFilterRef filter1, PGPFilterRef filter2, PGPFilterRef *outFilter), (filter1, filter2, outFilter))

//PGPError PGPKeyIterMove(PGPKeyIterRef iter, PGPInt32 relOffset, PGPKeyDBObjRef *outRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPKeyIterMove, (PGPKeyIterRef iter, PGPInt32 relOffset, PGPKeyDBObjRef *outRef), (iter, relOffset, outRef))

//PGPError PGPKeyIterNextKeyDBObj(PGPKeyIterRef iter, PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPKeyIterNextKeyDBObj, (PGPKeyIterRef iter, PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef), (iter, objectType, outRef))

//PGPError PGPNewContext(PGPUInt32 sdkAPIVersion, PGPContextRef *newContext);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewContext, (PGPUInt32 sdkAPIVersion, PGPContextRef *newContext), (sdkAPIVersion, newContext))

//PGPError PGPNewEmptyKeySet(PGPKeyDBRef keyDB, PGPKeySetRef *keySet);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewEmptyKeySet, (PGPKeyDBRef keyDB, PGPKeySetRef *keySet), (keyDB, keySet))

//PGPError PGPNewFileSpecFromFSSpec(PGPContextRef context, const struct FSSpec *spec, PGPFileSpecRef *ref);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewFileSpecFromFSSpec, (PGPContextRef context, const struct FSSpec *spec, PGPFileSpecRef *ref), (context, spec, ref))

//PGPError PGPNewKeyDBObjBooleanFilter(PGPContextRef context, PGPKeyDBObjProperty whichProperty, PGPBoolean match, PGPFilterRef *outFilter);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewKeyDBObjBooleanFilter, (PGPContextRef context, PGPKeyDBObjProperty whichProperty, PGPBoolean match, PGPFilterRef *outFilter), (context, whichProperty, match, outFilter))

//PGPError PGPNewKeyDBObjDataFilter(PGPContextRef context, PGPKeyDBObjProperty whichProperty, const void *matchData, PGPSize matchDataSize, PGPMatchCriterion matchCriteria, PGPFilterRef *outFilter);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewKeyDBObjDataFilter, (PGPContextRef context, PGPKeyDBObjProperty whichProperty, const void *matchData, PGPSize matchDataSize, PGPMatchCriterion matchCriteria, PGPFilterRef *outFilter), (context, whichProperty, matchData, matchDataSize, matchCriteria, outFilter))

//PGPError 	PGPNewKeyIDFromString( const char *string, PGPPublicKeyAlgorithm pkalg, PGPKeyID *id );
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewKeyIDFromString, (const char *string, PGPPublicKeyAlgorithm pkalg, PGPKeyID *id), (string, pkalg, id ))

//PGPError PGPNewKeyIter(PGPKeyListRef keys, PGPKeyIterRef *outRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewKeyIter, (PGPKeyListRef keys, PGPKeyIterRef *outRef), (keys, outRef))

//PGPError 	PGPNewKeyIterFromKeySet( PGPKeySetRef keys, PGPKeyIterRef *outRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewKeyIterFromKeySet, (PGPKeySetRef keys, PGPKeyIterRef *outRef), (keys, outRef))

//PGPError  	PGPNewOneKeySet( PGPKeyDBObjRef key, PGPKeySetRef *keySet );
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPNewOneKeySet, (PGPKeyDBObjRef key, PGPKeySetRef *keySet), (key, keySet))

//PGPError PGPOpenKeyDBFile(PGPContextRef context, PGPOpenKeyDBFileOptions options, PGPFileSpecRef pubKeysFileSpec, PGPFileSpecRef privKeysFileSpec, PGPKeyDBRef *keyDBRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPOpenKeyDBFile, (PGPContextRef context, PGPOpenKeyDBFileOptions options, PGPFileSpecRef pubKeysFileSpec, PGPFileSpecRef privKeysFileSpec, PGPKeyDBRef *keyDBRef), (context, options, pubKeysFileSpec, privKeysFileSpec, keyDBRef))

//PGPError PGPOrderKeySet(PGPKeySetRef src, PGPKeyOrdering order, PGPBoolean reverseOrder, PGPKeyListRef *outRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPOrderKeySet, (PGPKeySetRef src, PGPKeyOrdering order, PGPBoolean reverseOrder, PGPKeyListRef *outRef), (src, order, reverseOrder, outRef))

//PGPError PGPsdkCleanup(void);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPsdkCleanup, (void), ())

//PGPError PGPsdkInit(PGPFlags options);
IMPORT_FUNCTION(sPGPSDKLoader, PGPError, PGPsdkInit, (PGPFlags options), (options))

//PGPOptionListRef PGPOAllocatedOutputBuffer(PGPContextRef context, void **buffer, PGPSize maximumBufferSize, PGPSize *actualBufferSize);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOAllocatedOutputBuffer, (PGPContextRef context, void **buffer, PGPSize maximumBufferSize, PGPSize *actualBufferSize), (context, buffer, maximumBufferSize, actualBufferSize))

//PGPOptionListRef PGPOArmorOutput(PGPContextRef context, PGPBoolean armorOutput);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOArmorOutput, (PGPContextRef context, PGPBoolean armorOutput), (context, armorOutput))

//PGPOptionListRef PGPOClearSign(PGPContextRef context, PGPBoolean clearSign);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOClearSign, (PGPContextRef context, PGPBoolean clearSign), (context, clearSign))

//PGPOptionListRef PGPOCommentString(PGPContextRef context, char const *comment);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOCommentString, (PGPContextRef context, char const *comment), (context, comment))

//PGPOptionListRef PGPODataIsASCII(PGPContextRef context, PGPBoolean dataIsASCII);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPODataIsASCII, (PGPContextRef context, PGPBoolean dataIsASCII), (context, dataIsASCII))

//PGPOptionListRef PGPODetachedSig(PGPContextRef context, PGPOptionListRef firstOption, ...);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPODetachedSig, (PGPContextRef context, PGPOptionListRef firstOption, ...), (context, firstOption, __va_start(firstOption)))

//PGPOptionListRef PGPODiscardOutput(PGPContextRef context, PGPBoolean discardOutput);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPODiscardOutput, (PGPContextRef context, PGPBoolean discardOutput), (context, discardOutput))

//PGPOptionListRef PGPOEncryptToKeySet(PGPContextRef context, PGPKeySetRef keySetRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOEncryptToKeySet, (PGPContextRef context, PGPKeySetRef keySetRef), (context, keySetRef))

//PGPOptionListRef PGPOEventHandler(PGPContextRef context, PGPEventHandlerProcPtr eventHandler, PGPUserValue eventHandlerData);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOEventHandler, (PGPContextRef context, PGPEventHandlerProcPtr eventHandler, PGPUserValue eventHandlerData), (context, eventHandler, eventHandlerData))

//PGPOptionListRef PGPOInputBuffer(PGPContextRef context, void const *buffer, PGPSize bufferSize);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOInputBuffer, (PGPContextRef context, void const *buffer, PGPSize bufferSize), (context, buffer, bufferSize))

//PGPOptionListRef PGPOInputFile(PGPContextRef context, PGPFileSpecRef fileRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOInputFile, (PGPContextRef context, PGPFileSpecRef fileRef), (context, fileRef))

//PGPOptionListRef PGPOKeyDBRef(PGPContextRef context, PGPKeyDBRef keydbRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOKeyDBRef, (PGPContextRef context, PGPKeyDBRef keydbRef), (context, keydbRef))

//PGPOptionListRef PGPOLastOption(PGPContextRef context);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOLastOption, (PGPContextRef context), (context))

//PGPOptionListRef PGPOLocalEncoding(PGPContextRef context, PGPLocalEncodingFlags localEncode);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOLocalEncoding, (PGPContextRef context, PGPLocalEncodingFlags localEncode), (context, localEncode))

//PGPOptionListRef PGPOOutputFile(PGPContextRef context, PGPFileSpecRef fileRef);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOOutputFile, (PGPContextRef context, PGPFileSpecRef fileRef), (context, fileRef))

//PGPOptionListRef PGPOPassphrase(PGPContextRef context, const char *passphrase);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOPassphrase, (PGPContextRef context, const char *passphrase), (context, passphrase))

//PGPOptionListRef PGPOSignWithKey(PGPContextRef context, PGPKeyDBObjRef keyDBObjRef, PGPOptionListRef firstOption, ...);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOSignWithKey, (PGPContextRef context, PGPKeyDBObjRef keyDBObjRef, PGPOptionListRef firstOption, ...), (context, keyDBObjRef, firstOption, __va_start(firstOption)))

//PGPOptionListRef PGPOVersionString(PGPContextRef context, char const *version);
IMPORT_FUNCTION(sPGPSDKLoader, PGPOptionListRef, PGPOVersionString, (PGPContextRef context, char const *version), (context, version))

//PGPError PGPclOpenDefaultKeyrings(PGPContextRef context, PGPPrefRef prefRef, PGPOpenKeyDBFileOptions options, PGPKeyDBRef* pkeydbOut);
IMPORT_FUNCTION(sPGPClientSDKLoader, PGPError, PGPclOpenDefaultKeyrings, (PGPContextRef context, PGPPrefRef prefRef, PGPOpenKeyDBFileOptions options, PGPKeyDBRef* pkeydbOut), (context, prefRef, options, pkeydbOut))

void PGPSDK_LoadFuncPtrs();
void PGPSDK_LoadFuncPtrs()
{
	LOAD_FUNC_PTR(sPGPSDKLoader, PGPAddJobOptions);
	LOAD_FUNC_PTR(sPGPSDKLoader, PGPEncode);
	LOAD_FUNC_PTR(sPGPSDKLoader, PGPDecode);
	LOAD_FUNC_PTR(sPGPSDKLoader, PGPODetachedSig);
	LOAD_FUNC_PTR(sPGPSDKLoader, PGPOSignWithKey);
}