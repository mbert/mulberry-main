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

/* 
	CWebDAVDefinitions.h

	Author:			
	Description:	<describe the CWebDAVDefinitions class here>
*/

#ifndef CWebDAVDefinitions_H
#define CWebDAVDefinitions_H

#include "XMLName.h"

namespace http {

namespace webdav 
{

// RFC2518 ¤ - WebDAV Properties

extern const char*	cNamespace;

extern const xmllib::XMLName	cElement_multistatus;
extern const xmllib::XMLName	cElement_response;
extern const xmllib::XMLName	cElement_href;
extern const xmllib::XMLName	cElement_status;

extern const xmllib::XMLName	cElement_propname;
extern const xmllib::XMLName	cElement_propfind;
extern const xmllib::XMLName	cElement_prop;
extern const xmllib::XMLName	cElement_propstat;

extern const xmllib::XMLName	cElement_propertyupdate;
extern const xmllib::XMLName	cElement_set;
extern const xmllib::XMLName	cElement_remove;

extern const xmllib::XMLName	cProperty_getcontentlength;
extern const xmllib::XMLName	cProperty_getcontenttype;
extern const xmllib::XMLName	cProperty_resourcetype;
extern const xmllib::XMLName	cProperty_collection;
extern const xmllib::XMLName	cProperty_getetag;
extern const xmllib::XMLName	cProperty_displayname;

extern const xmllib::XMLName	cElement_error;

// RFC3253 ¤ - DeltaV Properties
extern const xmllib::XMLName	cProperty_supported_report_set;
	extern const xmllib::XMLName	cElement_supported_report;
		extern const xmllib::XMLName	cElement_report;

// RFC3744 ¤ - ACL Properties
extern const xmllib::XMLName	cProperty_supported_privilege_set;

extern const xmllib::XMLName	cProperty_current_user_privilege_set;

extern const xmllib::XMLName	cProperty_principal_collection_set;

extern const xmllib::XMLName	cProperty_principal_URL;
extern const xmllib::XMLName	cProperty_alternate_URI_set;
extern const xmllib::XMLName	cProperty_group_member_set;
extern const xmllib::XMLName	cProperty_group_membership;

extern const xmllib::XMLName	cProperty_acl;
	extern const xmllib::XMLName	cProperty_ace;
		extern const xmllib::XMLName	cProperty_principal;
		extern const xmllib::XMLName	cProperty_href;
		extern const xmllib::XMLName	cProperty_all;
		extern const xmllib::XMLName	cProperty_authenticated;
		extern const xmllib::XMLName	cProperty_unauthenticated;
		extern const xmllib::XMLName	cProperty_property;
		extern const xmllib::XMLName	cProperty_self;
		extern const xmllib::XMLName	cProperty_invert;
		extern const xmllib::XMLName	cProperty_grant;
		extern const xmllib::XMLName	cProperty_deny;
		extern const xmllib::XMLName	cProperty_protected;
		extern const xmllib::XMLName	cProperty_inherited;

extern const xmllib::XMLName	cProperty_privilege;
	extern const xmllib::XMLName	cProperty_privilege_read;
	extern const xmllib::XMLName	cProperty_privilege_write;
	extern const xmllib::XMLName	cProperty_privilege_write_properties;
	extern const xmllib::XMLName	cProperty_privilege_write_content;
	extern const xmllib::XMLName	cProperty_privilege_unlock;
	extern const xmllib::XMLName	cProperty_privilege_read_acl;
	extern const xmllib::XMLName	cProperty_privilege_read_current_user_privilege_set;
	extern const xmllib::XMLName	cProperty_privilege_write_acl;
	extern const xmllib::XMLName	cProperty_privilege_bind;
	extern const xmllib::XMLName	cProperty_privilege_unbind;
	extern const xmllib::XMLName	cProperty_privilege_all;

extern const xmllib::XMLName	cElement_principal_match;

// Extended MKCOL
extern const xmllib::XMLName	cElement_mkcol;
extern const xmllib::XMLName	cElement_mkcol_response;
	
// current-user-principal
extern const xmllib::XMLName	cProperty_current_user_principal;

// sync-collection REPORT
extern const xmllib::XMLName	cElement_sync_collection;
extern const xmllib::XMLName	cElement_sync_token;
extern const xmllib::XMLName	cElement_valid_sync_token;
}

namespace caldav 
{

// Access
extern const char*	cNamespace;

extern const xmllib::XMLName	cProperty_caldavcalendar;

extern const xmllib::XMLName	cElement_calendarquery;
extern const xmllib::XMLName	cElement_calendarmultiget;
extern const xmllib::XMLName	cElement_freebusyquery;

	extern const xmllib::XMLName	cElement_calendardata;

	extern const xmllib::XMLName	cElement_comp;
		extern const xmllib::XMLName	cAttribute_name;
	extern const xmllib::XMLName	cElement_allcomp;
	extern const xmllib::XMLName	cElement_allprop;
	extern const xmllib::XMLName	cElement_prop;
		extern const xmllib::XMLName	cAttribute_novalue;
	extern const xmllib::XMLName	cElement_expandrecurrenceset;
		extern const xmllib::XMLName	cAttribute_start;
		extern const xmllib::XMLName	cAttribute_end;

	extern const xmllib::XMLName	cElement_filter;
	extern const xmllib::XMLName	cElement_compfilter;
	extern const xmllib::XMLName	cElement_propfilter;
	extern const xmllib::XMLName	cElement_paramfilter;

	extern const xmllib::XMLName	cElement_isnotdefined;
	extern const xmllib::XMLName	cElement_textmatch;
		extern const xmllib::XMLName	cAttribute_caseless;
	extern const xmllib::XMLName	cElement_timerange;

	extern const xmllib::XMLName	cAttributeValue_yes;
	extern const xmllib::XMLName	cAttributeValue_no;

extern const xmllib::XMLName	cProperty_privilege_readfreebusy;

// Principal
extern const xmllib::XMLName	cProperty_calendar_home_set;

// Schedule
extern const xmllib::XMLName	cProperty_scheduleinbox;
extern const xmllib::XMLName	cProperty_scheduleoutbox;
extern const xmllib::XMLName	cProperty_scheduleinboxURL;
extern const xmllib::XMLName	cProperty_scheduleoutboxURL;

extern const xmllib::XMLName	cProperty_schedulefreebusyset;

extern const char* cHeaderOriginator;
extern const char* cHeaderRecipient;

extern const xmllib::XMLName	cElement_scheduleresponse;
extern const xmllib::XMLName	cElement_caldavresponse;
extern const xmllib::XMLName	cElement_recipient;
extern const xmllib::XMLName	cElement_requeststatus;

extern const xmllib::XMLName	cProperty_privilege_schedule;
}


namespace carddav 
{

extern const char*	cNamespace;

extern const xmllib::XMLName	cProperty_carddavadbk;

extern const xmllib::XMLName	cElement_adbkquery;
extern const xmllib::XMLName	cElement_adbkmultiget;

	extern const xmllib::XMLName	cElement_adbkdata;

	extern const xmllib::XMLName	cElement_allprop;
	extern const xmllib::XMLName	cElement_prop;
		extern const xmllib::XMLName	cAttribute_name;
		extern const xmllib::XMLName	cAttribute_novalue;

	extern const xmllib::XMLName	cElement_filter;
	extern const xmllib::XMLName	cElement_compfilter;
	extern const xmllib::XMLName	cElement_propfilter;
	extern const xmllib::XMLName	cElement_paramfilter;

	extern const xmllib::XMLName	cElement_isnotdefined;
	extern const xmllib::XMLName	cElement_textmatch;
		extern const xmllib::XMLName	cAttribute_collation;
		extern const xmllib::XMLName	cAttribute_negate_condition;
		extern const xmllib::XMLName	cAttribute_matchtype;

	extern const xmllib::XMLName	cAttributeValue_yes;
	extern const xmllib::XMLName	cAttributeValue_no;

	extern const xmllib::XMLName	cAttributeValue_matchtype_starts;
	extern const xmllib::XMLName	cAttributeValue_matchtype_ends;
	extern const xmllib::XMLName	cAttributeValue_matchtype_contains;
	extern const xmllib::XMLName	cAttributeValue_matchtype_is;

	// Principal
	extern const xmllib::XMLName	cProperty_addressbook_home_set;
	
}

namespace slide
{
extern const char*	cNamespace;

	extern const xmllib::XMLName	cInheritable_comp;
	extern const xmllib::XMLName	cNonInheritable_comp;
}

namespace calendarserver
{
extern const char*	cNamespace;

	extern const xmllib::XMLName	cProperty_getctag;
}

}	// namespace http

#endif	// CWebDAVRequest_H
