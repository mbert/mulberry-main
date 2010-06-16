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
	CWebDAVDefinitions.cpp

	Author:			
	Description:	<describe the CWebDAVDefinitions class here>
*/

#include "CWebDAVDefinitions.h"

namespace http
{

namespace webdav 
{

// RFC2518 ¤ - WebDAV Properties

const char*	cNamespace = "DAV:";

const xmllib::XMLName	cElement_multistatus("multistatus", cNamespace);
const xmllib::XMLName	cElement_response("response", cNamespace);
const xmllib::XMLName	cElement_href("href", cNamespace);
const xmllib::XMLName	cElement_status("status", cNamespace);

const xmllib::XMLName	cElement_propname("propname", cNamespace);
const xmllib::XMLName	cElement_propfind("propfind", cNamespace);
const xmllib::XMLName	cElement_prop("prop", cNamespace);
const xmllib::XMLName	cElement_propstat("propstat", cNamespace);

const xmllib::XMLName	cElement_propertyupdate("propertyupdate", cNamespace);
const xmllib::XMLName	cElement_set("set", cNamespace);
const xmllib::XMLName	cElement_remove("remove", cNamespace);

const xmllib::XMLName	cProperty_getcontentlength("getcontentlength", cNamespace);
const xmllib::XMLName	cProperty_getcontenttype("getcontenttype", cNamespace);
const xmllib::XMLName	cProperty_resourcetype("resourcetype", cNamespace);
const xmllib::XMLName	cProperty_collection("collection", cNamespace);
const xmllib::XMLName	cProperty_getetag("getetag", cNamespace);
const xmllib::XMLName	cProperty_displayname("displayname", cNamespace);

// RFC3253 ¤ - DeltaV Properties
const xmllib::XMLName	cProperty_supported_report_set("supported-report-set", cNamespace);
const xmllib::XMLName	cElement_supported_report("supported-report", cNamespace);
const xmllib::XMLName	cElement_report("report", cNamespace);
	
	// RFC3744 ¤ - ACL Properties
const xmllib::XMLName	cProperty_supported_privilege_set("supported-privilege-set", cNamespace);
const xmllib::XMLName	cProperty_current_user_privilege_set("current-user-privilege-set", cNamespace);
const xmllib::XMLName	cProperty_principal_collection_set("principal-collection-set", cNamespace);
const xmllib::XMLName	cProperty_principal_URL("principal-URL", cNamespace);
const xmllib::XMLName	cProperty_alternate_URI_set("alternate-URI-set", cNamespace);
const xmllib::XMLName	cProperty_group_member_set("group-member-set", cNamespace);
const xmllib::XMLName	cProperty_group_membership("group-membership", cNamespace);
const xmllib::XMLName	cProperty_acl("acl", cNamespace);
	const xmllib::XMLName	cProperty_ace("ace", cNamespace);
		const xmllib::XMLName	cProperty_principal("principal", cNamespace);
		const xmllib::XMLName	cProperty_href("href", cNamespace);
		const xmllib::XMLName	cProperty_all("all", cNamespace);
		const xmllib::XMLName	cProperty_authenticated("authenticated", cNamespace);
		const xmllib::XMLName	cProperty_unauthenticated("unauthenticated", cNamespace);
		const xmllib::XMLName	cProperty_property("property", cNamespace);
		const xmllib::XMLName	cProperty_self("self", cNamespace);
		const xmllib::XMLName	cProperty_invert("invert", cNamespace);
		const xmllib::XMLName	cProperty_grant("grant", cNamespace);
		const xmllib::XMLName	cProperty_deny("deny", cNamespace);
		const xmllib::XMLName	cProperty_protected("protected", cNamespace);
		const xmllib::XMLName	cProperty_inherited("inherited", cNamespace);

const xmllib::XMLName	cProperty_privilege("privilege", cNamespace);
	const xmllib::XMLName	cProperty_privilege_read("read", cNamespace);
	const xmllib::XMLName	cProperty_privilege_write("write", cNamespace);
	const xmllib::XMLName	cProperty_privilege_write_properties("write-properties", cNamespace);
	const xmllib::XMLName	cProperty_privilege_write_content("write-content", cNamespace);
	const xmllib::XMLName	cProperty_privilege_unlock("unlock", cNamespace);
	const xmllib::XMLName	cProperty_privilege_read_acl("read-acl", cNamespace);
	const xmllib::XMLName	cProperty_privilege_read_current_user_privilege_set("read-current-user-privilege-set", cNamespace);
	const xmllib::XMLName	cProperty_privilege_write_acl("write-acl", cNamespace);
	const xmllib::XMLName	cProperty_privilege_bind("bind", cNamespace);
	const xmllib::XMLName	cProperty_privilege_unbind("unbind", cNamespace);
	const xmllib::XMLName	cProperty_privilege_all("all", cNamespace);

const xmllib::XMLName	cElement_principal_match("principal-match", cNamespace);

// Extended MKCOL
const xmllib::XMLName	cElement_mkcol("mkcol", cNamespace);
const xmllib::XMLName	cElement_mkcol_response("mkcol-response", cNamespace);

// current-user-principal
const xmllib::XMLName	cProperty_current_user_principal("current-user-principal", cNamespace);

// sync-collection REPORT
extern const xmllib::XMLName	cElement_sync_collection("sync-collection", cNamespace);
extern const xmllib::XMLName	cElement_sync_token("sync-token", cNamespace);
}

namespace caldav 
{

// Access
const char*	cNamespace = "urn:ietf:params:xml:ns:caldav";
const xmllib::XMLName	cProperty_caldavcalendar("calendar", cNamespace);

const xmllib::XMLName	cElement_calendarquery("calendar", cNamespace);
const xmllib::XMLName	cElement_calendarmultiget("calendar-multiget", cNamespace);
const xmllib::XMLName	cElement_freebusyquery("free-busy-query", cNamespace);

	const xmllib::XMLName	cElement_calendardata("calendar-data", cNamespace);

	const xmllib::XMLName	cElement_comp("comp", cNamespace);
		const xmllib::XMLName	cAttribute_name("name", cNamespace);
	const xmllib::XMLName	cElement_allcomp("allcomp", cNamespace);
	const xmllib::XMLName	cElement_allprop("allprop", cNamespace);
	const xmllib::XMLName	cElement_prop("prop", cNamespace);
		const xmllib::XMLName	cAttribute_novalue("novalue", cNamespace);
	const xmllib::XMLName	cElement_expandrecurrenceset("expand-recurrence-set", cNamespace);
		const xmllib::XMLName	cAttribute_start("start", cNamespace);
		const xmllib::XMLName	cAttribute_end("end", cNamespace);

	const xmllib::XMLName	cElement_filter("filter", cNamespace);
	const xmllib::XMLName	cElement_compfilter("comp-filter", cNamespace);
	const xmllib::XMLName	cElement_propfilter("prop-filter", cNamespace);
	const xmllib::XMLName	cElement_paramfilter("param-filter", cNamespace);

	const xmllib::XMLName	cElement_isnotdefined("is-not-defined", cNamespace);
	const xmllib::XMLName	cElement_textmatch("text-match", cNamespace);
		const xmllib::XMLName	cAttribute_caseless("caseless", cNamespace);
	const xmllib::XMLName	cElement_timerange("time-range", cNamespace);

	const xmllib::XMLName	cAttributeValue_yes("yes", cNamespace);
	const xmllib::XMLName	cAttributeValue_no("no", cNamespace);

const xmllib::XMLName	cProperty_privilege_readfreebusy("read-free-busy", cNamespace);

// Principal
const xmllib::XMLName	cProperty_calendar_home_set("calendar-home-set", cNamespace);

// Schedule
const xmllib::XMLName	cProperty_scheduleinbox("schedule-inbox", cNamespace);
const xmllib::XMLName	cProperty_scheduleoutbox("schedule-outbox", cNamespace);
const xmllib::XMLName	cProperty_scheduleinboxURL("schedule-inbox-URL", cNamespace);
const xmllib::XMLName	cProperty_scheduleoutboxURL("schedule-outbox-URL", cNamespace);

const xmllib::XMLName	cProperty_schedulefreebusyset("free-busy-set", cNamespace);

const char* cHeaderOriginator = "Originator";
const char* cHeaderRecipient = "Recipient";


const xmllib::XMLName	cElement_scheduleresponse("schedule-response", cNamespace);
const xmllib::XMLName	cElement_caldavresponse("response", cNamespace);
const xmllib::XMLName	cElement_recipient("recipient", cNamespace);
const xmllib::XMLName	cElement_requeststatus("request-status", cNamespace);

const xmllib::XMLName	cProperty_privilege_schedule("schedule", cNamespace);

}

namespace carddav 
{

const char*	cNamespace = "urn:ietf:params:xml:ns:carddav";
const xmllib::XMLName	cProperty_carddavadbk("addressbook", cNamespace);

const xmllib::XMLName	cElement_adbkquery("addressbook-query", cNamespace);
const xmllib::XMLName	cElement_adbkmultiget("addressbook-multiget", cNamespace);

	const xmllib::XMLName	cElement_adbkdata("address-data", cNamespace);

	const xmllib::XMLName	cElement_allprop("allprop", cNamespace);
	const xmllib::XMLName	cElement_prop("prop", cNamespace);
		const xmllib::XMLName	cAttribute_name("name", cNamespace);
		const xmllib::XMLName	cAttribute_novalue("novalue", cNamespace);

	const xmllib::XMLName	cElement_filter("filter", cNamespace);
	const xmllib::XMLName	cElement_propfilter("prop-filter", cNamespace);
	const xmllib::XMLName	cElement_paramfilter("param-filter", cNamespace);

	const xmllib::XMLName	cElement_isnotdefined("is-not-defined", cNamespace);
	const xmllib::XMLName	cElement_textmatch("text-match", cNamespace);
		const xmllib::XMLName	cAttribute_collation("collation", cNamespace);
		const xmllib::XMLName	cAttribute_negate_condition("negate-condition", cNamespace);
		const xmllib::XMLName	cAttribute_matchtype("match-type", cNamespace);

	const xmllib::XMLName	cAttributeValue_yes("yes", cNamespace);
	const xmllib::XMLName	cAttributeValue_no("no", cNamespace);

	const xmllib::XMLName	cAttributeValue_matchtype_starts("starts-with", cNamespace);
	const xmllib::XMLName	cAttributeValue_matchtype_ends("ends-with", cNamespace);
	const xmllib::XMLName	cAttributeValue_matchtype_contains("contains", cNamespace);
	const xmllib::XMLName	cAttributeValue_matchtype_is("equals", cNamespace);

	// Principal
	const xmllib::XMLName	cProperty_addressbook_home_set("addressbook-home-set", cNamespace);
	
}

namespace slide
{
const char*	cNamespace = "http://jakarta.apache.org/slide/";

	const xmllib::XMLName	cInheritable_comp("inheritable", cNamespace);
	const xmllib::XMLName	cNonInheritable_comp("non-inheritable", cNamespace);
}

namespace calendarserver
{
const char*	cNamespace = "http://calendarserver.org/ns/";

	const xmllib::XMLName	cProperty_getctag("getctag", cNamespace);
}

}
