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


// Header for INET client class

// This is an abstract base class that can be used with IMAP, IMSP & ACAP protocols

#ifndef __CINETCLIENTERRORS__MULBERRY__
#define __CINETCLIENTERRORS__MULBERRY__

// consts
#if __dest_os == __mac_os || __dest_os == __mac_os_x
const	ResIDT	STRx_INETErrors = 999;
#endif
enum
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	str_BadParse = 1,
#elif __dest_os == __win32_os
	str_BadParse = 11001,
#elif __dest_os == __linux_os
	str_BadParse = 11001,
#else
	#error __dest_os
#endif
	str_ConnectionAborted,
	str_NoSSLPlugin,
	str_NoSSLError,
	str_NoSSLCertError,
	str_NoSSLCertNoAccept,
	str_OSErrOpen,
	str_NoBadOpen,
	str_OSErrLogon,
	str_NoBadLogon,
	str_OSErrLogout,
	str_NoBadLogout,
	str_OSErrStartTLS,
	str_NoBadStartTLS,
	str_OSErrTLSClientCert,
	str_NoBadTLSClientCert,
	str_FetchOutOfMemory,
	str_TryCreateOpen,
	str_TryCreateCopy,
	str_OSErrSelect,
	str_NoBadSelect,
	str_OSErrExpunge,
	str_NoBadExpunge,
	str_OSErrCreate,
	str_NoBadCreate,
	str_OSErrDelete,
	str_NoBadDelete,
	str_OSErrRename,
	str_NoBadRename,
	str_OSErrSubscribe,
	str_NoBadSubscribe,
	str_OSErrUnsubscribe,
	str_NoBadUnsubscribe,
	str_OSErrNamespace,
	str_NoBadNamespace,
	str_OSErrFindAll,
	str_NoBadFindAll,
	str_OSErrAppend,
	str_NoBadAppend,
	str_OSErrSearch,
	str_NoBadSearch,
	str_OSErrReadMsg,
	str_NoBadReadMsg,
	str_OSErrDeleteMsg,
	str_NoBadDeleteMsg,
	str_OSErrUndeleteMsg,
	str_NoBadUndeleteMsg,
	str_OSErrSeenMsg,
	str_NoBadSeenMsg,
	str_OSErrAnsweredMsg,
	str_NoBadAnsweredMsg,
	str_OSErrFlaggedMsg,
	str_NoBadFlaggedMsg,
	str_OSErrDraftMsg,
	str_NoBadDraftMsg,
	str_OSErrCopyMsg,
	str_NoBadCopyMsg,
	str_OSErrCheck,
	str_NoBadCheck,
	str_OSErrSort,
	str_NoBadSort,
	str_OSErrThread,
	str_NoBadThread,
	str_OSErrSetACL,
	str_NoBadSetACL,
	str_OSErrDeleteACL,
	str_NoBadDeleteACL,
	str_OSErrGetACL,
	str_NoBadGetACL,
	str_OSErrListRights,
	str_NoBadListRights,
	str_OSErrMyRights,
	str_NoBadMyRights,
	str_OSErrSetQuota,
	str_NoBadSetQuota,
	str_OSErrGetQuota,
	str_NoBadGetQuota,
	str_OSErrGetQuotaRoot,
	str_NoBadGetQuotaRoot,
	str_OSErrGet,
	str_NoBadGet,
	str_OSErrSet,
	str_NoBadSet,
	str_OSErrUnset,
	str_NoBadUnset,
	str_OSErrAddressBooks,
	str_NoBadAddressBooks,
	str_OSErrCreateAddressBook,
	str_NoBadCreateAddressBook,
	str_OSErrDeleteAddressBook,
	str_NoBadDeleteAddressBook,
	str_OSErrRenameAddressBook,
	str_NoBadRenameAddressBook,
	str_OSErrSearchAddress,
	str_NoBadSearchAddress,
	str_OSErrFetchAddress,
	str_NoBadFetchAddress,
	str_OSErrStoreAddress,
	str_NoBadStoreAddress,
	str_OSErrDeleteAddress,
	str_NoBadDeleteAddress,
	str_OSErrSetACLAddressBook,
	str_NoBadSetACLAddressBook,
	str_OSErrDeleteACLAddressBook,
	str_NoBadDeleteACLAddressBook,
	str_OSErrGetACLAddressBook,
	str_NoBadGetACLAddressBook,
	str_OSErrMyRightsAddressBook,
	str_NoBadMyRightsAddressBook,
	str_LDAPOpen,
	str_LDAPFail,
	str_OSErrHaveSpace,
	str_NoBadHaveSpace,
	str_OSErrPutScript,
	str_NoBadPutScript,
	str_OSErrListScripts,
	str_NoBadListScripts,
	str_OSErrSetActive,
	str_NoBadSetActive,
	str_OSErrGetScript,
	str_NoBadGetScript,
	str_OSErrDeleteScript,
	str_NoBadDeleteScript,
	str_OSErrListCalendars,
	str_NoBadListCalendars,
	str_OSErrCreateCalendar,
	str_NoBadCreateCalendar,
	str_OSErrDeleteCalendar,
	str_NoBadDeleteCalendar,
	str_OSErrRenameCalendar,
	str_NoBadRenameCalendar,
	str_OSErrReadCalendar,
	str_NoBadReadCalendar,
	str_OSErrWriteCalendar,
	str_NoBadWriteCalendar
};

#endif
