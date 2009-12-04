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


// CPPRegister.cp - routine to register all PowerPlant classes

#include "CPPRegister.h"

#include "CAboutDialog.h"
#include "CAboutPluginsDialog.h"
#include "CAcceptCertDialog.h"
#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CAdbkManagerTable.h"
#include "CAdbkManagerTitleTable.h"
#include "CAdbkManagerToolbar.h"
#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkPropDialog.h"
#include "CAdbkSearchTable.h"
#include "CAdbkSearchTitleTable.h"
#include "CAdbkSearchWindow.h"
#include "CAdbkServerPopup.h"
#include "CAddressBookTitleTable.h"
#include "CAddressBookToolbar.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAddressDisplay.h"
#include "CAddressContactPanel.h"
#include "CAddressNotesPanel.h"
#include "CAddressPaneOptions.h"
#include "CAddressPersonalPanel.h"
#include "CAddressPreview.h"
#include "CAddressTable.h"
#include "CAddressText.h"
#include "CAddressView.h"
#include "CAttendeeTable.h"
#include "CBetterScrollerX.h"
#include "CBodyTable.h"
#include "CBounceActionDialog.h"
#include "CBrowseMailboxDialog.h"
#include "CCacheMessageDialog.h"
#include "CCalendarAddressDisplay.h"
#include "CCalendarPaneOptions.h"
#include "CCalendarPopup.h"
#include "CCalendarPropDialog.h"
#include "CCalendarView.h"
#include "CCalendarStoreTable.h"
#include "CCalendarStoreTitleTable.h"
#include "CCalendarStoreToolbar.h"
#include "CCalendarStoreView.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarToolbar.h"
#include "CCalendarWindow.h"
#include "CCancelDialog.h"
#include "CCertManagerDialog.h"
#include "CChangePswdAcctDialog.h"
#include "CChangePswdDialog.h"
#include "CCheckboxTable.h"
#include "CChooseDateDialog.h"
#include "CChooseServersDialog.h"
#include "CColorPopup.h"
#include "CCreateAdbkDialog.h"
#include "CCreateAccountDialog.h"
#include "CCreateCalendarDialog.h"
#include "CCreateMailboxDialog.h"
#include "CDateControl.h"
#include "CDateTimeZoneSelect.h"
#include "CDayWeekPrintout.h"
#include "CDayWeekView.h"
#include "CDayWeekTable.h"
#include "CDayWeekTitleTable.h"
#include "CDictionaryPageScroller.h"
#include "CDisconnectDialog.h"
#include "CDurationSelect.h"
#include "CEditAddressDialog.h"
#include "CEditFormattedTextDisplay.h"
#include "CEditGroupDialog.h"
#include "CEditIdentities.h"
#include "CEditIdentityAddress.h"
#include "CEditIdentityOptions.h"
#include "CEditIdentityOutgoing.h"
#include "CEditIdentityDSN.h"
#include "CEditIdentitySecurity.h"
#include "CEditMacro.h"
#include "CEditMappingsTable.h"
#include "CEventPreview.h"
#include "CStyleToolbar.h"
#include "CErrorDialog.h"
#include "CFileTable.h"
#include "CFindReplaceWindow.h"
#include "CFocusX.h"
#include "CFontPopup.h"
#include "CFormattedTextDisplay.h"
#include "CForwardActionDialog.h"
#include "CForwardOptionsDialog.h"
#include "CFreeBusyPrintout.h"
#include "CFreeBusyView.h"
#include "CFreeBusyTable.h"
#include "CFreeBusyTitleTable.h"
#include "CGetPassphraseDialog.h"
#include "CGetStringDialog.h"
#include "CGroupPreview.h"
#include "CGroupTable.h"
#include "CHelpAttach.h"
#include "CIconTextTable.h"
#include "CIdentityPopup.h"
#include "CIMAPLabelsDialog.h"
#include "CKeyChoiceDialog.h"
#include "CLetterPartProp.h"
#include "CLetterTextDisplay.h"
#include "CLetterToolbar.h"
#include "CLetterView.h"
#include "CLetterWindow.h"
#include "CLoggingOptionsDialog.h"
#include "CMacroEditDialog.h"
#include "CMailboxPropDialog.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxInfoView.h"
#include "CMailboxInfoWindow.h"
#include "CMailboxTitleTable.h"
#include "CMailboxInfoTable.h"
#include "CMailboxPopup.h"
#include "CMailboxToolbarPopup.h"
#include "CMatchOptionsDialog.h"
#include "CMDNPromptDialog.h"
#include "CMessagePaneOptions.h"
#include "CMessagePartProp.h"
#include "CMessageSpeechEditDialog.h"
#include "CMessageSpeechTable.h"
#include "CMessageToolbar.h"
#include "CMessageSimpleView.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMonthIndicator.h"
#include "CMonthPopup.h"
#include "CMonthPrintout.h"
#include "CMonthView.h"
#include "CMonthTable.h"
#include "CMonthTitleTable.h"
#include "CMultiUserDialog.h"
#include "CNamespaceDialog.h"
#include "CNetworkResetError.h"
#include "CNewACLDialog.h"
#include "CNewAttendeeDialog.h"
#include "CNewComponentDetails.h"
#include "CNewComponentRepeat.h"
#include "CNewComponentAlarm.h"
#include "CNewComponentAttendees.h"
#include "CNewEventDialog.h"
#include "CNewEventTiming.h"
#include "CNewSearchDialog.h"
#include "CNewToDoDialog.h"
#include "CNewToDoTiming.h"
#include "CNewWildcardDialog.h"
#include "CNumberEdit.h"
#include "COpenMailboxDialog.h"
#include "CPickPopup.h"
#include "CPluginRegisterDialog.h"
#include "CPreferencesDialog.h"
#include "CPrefsAccount.h"
#include "CPrefsAccountExtras.h"
#include "CPrefsAccountIMAP.h"
#include "CPrefsAccountAuth.h"
#include "CPrefsAccountLocal.h"
#include "CPrefsAccountPOP3.h"
#include "CPrefsAccountRemoteCalendar.h"
#include "CPrefsAdbkAccount.h"
#include "CPrefsAddress.h"
#include "CPrefsAddressExpansion.h"
#include "CPrefsAddressCapture.h"
#include "CPrefsAddressIMSP.h"
#include "CPrefsAddressLDAPOptions.h"
#include "CPrefsAddressLDAP1.h"
#include "CPrefsAddressLDAP2.h"
#include "CPrefsAlerts.h"
#include "CPrefsAlertsAttachment.h"
#include "CPrefsAlertsMessage.h"
#include "CPrefsAttachments.h"
#include "CPrefsAttachmentsSend.h"
#include "CPrefsAttachmentsSave.h"
#include "CPrefsAuthAnonymous.h"
#include "CPrefsAuthKerberos.h"
#include "CPrefsAuthPlainText.h"
#include "CPrefsCalendar.h"
#include "CPrefsConfigMessageSpeech.h"
#include "CPrefsDisplay.h"
#include "CPrefsDisplayLabel.h"
#include "CPrefsDisplayMailbox.h"
#include "CPrefsDisplayMessage.h"
#include "CPrefsDisplayQuotes.h"
#include "CPrefsDisplayServer.h"
#include "CPrefsDisplayStyles.h"
#include "CPrefsEditCaption.h"
#include "CPrefsEditHeadFoot.h"
#include "CPrefsEditMappings.h"
#include "CPrefsEditMappingDialog.h"
#include "CPrefsFormatting.h"
#include "CPrefsIdentities.h"
#include "CPrefsLDAPAccount.h"
#include "CPrefsLetter.h"
#include "CPrefsLetterGeneral.h"
#include "CPrefsLetterOptions.h"
#include "CPrefsLetterStyled.h"
#include "CPrefsLocalAccount.h"
#include "CPrefsMailAccount.h"
#include "CPrefsMailbox.h"
#include "CPrefsMailboxBasic.h"
#include "CPrefsMailboxAdvanced.h"
#include "CPrefsMailboxDisconnected.h"
#include "CPrefsMailboxOptions.h"
#include "CPrefsMessage.h"
#include "CPrefsMessageGeneral.h"
#include "CPrefsMessageOptions.h"
#include "CPrefsPOP3Account.h"
#include "CPrefsRemoteAccount.h"
#include "CPrefsRemoteCalendarAccount.h"
#include "CPrefsRemoteOptions.h"
#include "CPrefsRemoteSets.h"
#include "CPrefsSecurity.h"
#include "CPrefsSimple.h"
#include "CPrefsSIEVEAccount.h"
#include "CPrefsSMTPAccount.h"
#include "CPrefsSMTPOptions.h"
#include "CPrefsSpeech.h"
#include "CPrefsSpelling.h"
#include "CPrefsSpellingOptions.h"
#include "CPrefsSpellingSuggestions.h"
#include "CPrefsSubTab.h"
#include "CPrintText.h"
#include "CProgress.h"
#include "CPropAdbkACL.h"
#include "CPropAdbkGeneral.h"
#include "CPropCalendarACL.h"
#include "CPropCalendarGeneral.h"
#include "CPropCalendarOptions.h"
#include "CPropCalendarServer.h"
#include "CPropCalendarWebcal.h"
#include "CPropMailboxACL.h"
#include "CPropMailboxGeneral.h"
#include "CPropMailboxOptions.h"
#include "CPropMailboxQuota.h"
#include "CPropMailboxServer.h"
#include "CQuickSearchSavedPopup.h"
#include "CQuickSearchText.h"
#include "CQuotaTable.h"
#include "CRecurrenceDialog.h"
#include "CRegistrationDialog.h"
#include "CRejectActionDialog.h"
#include "CReplyActionDialog.h"
#include "CReplyChooseDialog.h"
#include "CRulesActionLocal.h"
#include "CRulesActionSIEVE.h"
#include "CRulesDialog.h"
#include "CRulesTarget.h"
#include "CRulesTable.h"
#include "CRulesWindow.h"
#include "CSaveDraftDialog.h"
#include "CScriptsTable.h"
#include "CSearchCriteriaContainer.h"
#include "CSearchCriteriaLocal.h"
#include "CSearchCriteriaSIEVE.h"
#include "CSearchOptionsDialog.h"
#include "CSearchWindow.h"
#include "CServerBrowseTable.h"
#include "CServerTable.h"
#include "CServerTitleTable.h"
#include "CServerToolbar.h"
#include "CServerView.h"
#include "CServerViewPopup.h"
#include "CServerWindow.h"
#include "CSimpleTitleTable.h"
#include "CSizePopup.h"
#include "CSMTPTable.h"
#include "CSMTPToolbar.h"
#include "CSMTPView.h"
#include "CSMTPWindow.h"
#include "CSoundPopup.h"
#include "CSpellAddDialog.h"
#include "CSpellCheckDialog.h"
#include "CSpellChooseDictDialog.h"
#include "CSpellEditDialog.h"
#include "CSpellOptionsDialog.h"
#include "CSpellPlugin.h"
#include "CSplashScreen.h"
#include "CSplitDivider.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CStatusWindow.h"
#include "CSummaryPrintout.h"
#include "CSummaryView.h"
#include "CSummaryTable.h"
#include "CSummaryTitleTable.h"
#include "CSynchroniseDialog.h"
#include "CTabController.h"
#include "CTabsX.h"
#include "CTargetsDialog.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CTextHierarchy.h"
#include "CTextListChoice.h"
#include "CTextTable.h"
#include "CTimezonePopup.h"
#include "CToDoView.h"
#include "CToDoTable.h"
#include "CToggleIconButton.h"
#include "CToolbarButton.h"
#include "CToolbarButtonImp.h"
#include "CToolbarView.h"
#include "CTooltip.h"
#include "CUploadScriptDialog.h"
#include "CUserActionOptions.h"
#include "CUserPswdDialog.h"
#include "CVacationActionDialog.h"
#include "CViewCertDialog.h"
#include "CVisualProgress.h"
#include "CWebKitControl.h"
#include "CWebKitControlImp.h"
#include "CWindowOptionsDialog.h"
#include "CYearPopup.h"
#include "CYearPrintout.h"
#include "CYearView.h"
#include "CYearTable.h"
#include "C3PaneAddress.h"
#include "C3PaneAddressBook.h"
#include "C3PaneAccounts.h"
#include "C3PaneCalendar.h"
#include "C3PaneCalendarAccount.h"
#include "C3PaneContactsAccount.h"
#include "C3PaneEvent.h"
#include "C3PaneItems.h"
#include "C3PaneItemsTabs.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneMailAccount.h"
#include "C3PaneMailbox.h"
#include "C3PaneMainPanel.h"
#include "C3PaneMessage.h"
#include "C3PanePreview.h"
#include "C3PaneWindow.h"
#include "HCmdButtonAttachment.h"

#include <LActiveScroller.h>
#include <LMultiPanelView.h>
#include <LToggleButton.h>

#include <UControlRegistry.h>

// Dont dead strip these
void RegisterClasses(void)
{
		// Register classes for objects created from 'PPob' resources
		// For PowerPlant classes, you can copy the necessary RegisterClass
		// calls from PPobClasses.cp
		//
		// For your own classes, you must use the same four-character ID as
		// you specify in the 'PPob' resource (or in Constructor).
		// PowerPlant reserves all ID's composed entirely of lower case
		// letters.

	RegisterClass_(LActiveScroller);
	RegisterClass_(LBorderAttachment);
	RegisterClass_(LButton);
	RegisterClass_(LCaption);
	RegisterClass_(LColorEraseAttachment);
	RegisterClass_(LDialogBox);
	RegisterClass_(LEditField);
	RegisterClass_(LEraseAttachment);
	RegisterClass_(LListBox);
	RegisterClass_(LMultiPanelView);
	RegisterClass_(LPane);
	RegisterClass_(LPicture);
	RegisterClass_(LPlaceHolder);
	RegisterClass_(LPrintout);
	RegisterClass_(LRadioGroup);
	RegisterClass_(LScroller);
	RegisterClass_(LStdButton);
	RegisterClass_(LStdCheckBox);
	RegisterClass_(LStdRadioButton);
	RegisterClass_(LStdPopupMenu);
	RegisterClass_(LTabGroup);
	RegisterClass_(LTabGroupView);
	RegisterClass_(LTableView);
	RegisterClass_(LToggleButton);
	RegisterClass_(LView);
	RegisterClass_(LWindow);

	// AM classes
	UControlRegistry::RegisterClasses();
	RegisterClassID_(CToolbarButtonImp,		 CToolbarButton::imp_class_ID);

	// Derived views

	RegisterClass_(CAboutDialog);
	RegisterClass_(CAboutPluginsDialog);
	RegisterClass_(CAcceptCertDialog);
	RegisterClass_(CACLStylePopup);
	RegisterClass_(CACLTable);
	RegisterClass_(CAdbkManagerTable);
	RegisterClass_(CAdbkManagerTitleTable);
	RegisterClass_(CAdbkManagerToolbar);
	RegisterClass_(CAdbkManagerView);
	RegisterClass_(CAdbkManagerWindow);
	RegisterClass_(CAdbkPropDialog);
	RegisterClass_(CAdbkSearchTable);
	RegisterClass_(CAdbkSearchTitleTable);
	RegisterClass_(CAdbkSearchWindow);
	RegisterClass_(CAdbkServerPopup);
	RegisterClass_(CAddressBookTitleTable);
	RegisterClass_(CAddressBookToolbar);
	RegisterClass_(CAddressBookView);
	RegisterClass_(CAddressBookWindow);
	RegisterClass_(CAddressDisplay);
	RegisterClass_(CAddressContactPanel);
	RegisterClass_(CAddressNotesPanel);
	RegisterClass_(CAddressPaneOptions);
	RegisterClass_(CAddressPersonalPanel);
	RegisterClass_(CAddressPreview);
	RegisterClass_(CAddressTable);
	RegisterClass_(CAddressText);
	RegisterClass_(CAddressView);
	RegisterClass_(CAttendeeTable);
	RegisterClass_(CBarPane);
	RegisterClass_(CBetterScrollerX);
	RegisterClass_(CBodyTable);
	RegisterClass_(CBounceActionDialog);
	RegisterClass_(CBrowseMailboxDialog);
	RegisterClass_(CCacheMessageDialog);
	RegisterClass_(CCalendarAddressDisplay);
	RegisterClass_(CCalendarPaneOptions);
	RegisterClass_(CCalendarPopup);
	RegisterClass_(CCalendarPropDialog);
	RegisterClass_(CCalendarView);
	RegisterClass_(CCalendarStoreTable);
	RegisterClass_(CCalendarStoreTitleTable);
	RegisterClass_(CCalendarStoreToolbar);
	RegisterClass_(CCalendarStoreView);
	RegisterClass_(CCalendarStoreWindow);
	RegisterClass_(CCalendarToolbar);
	RegisterClass_(CCalendarWindow);
	RegisterClass_(CCancelDialog);
	RegisterClass_(CCertManagerDialog);
	RegisterClass_(CChangePswdAcctDialog);
	RegisterClass_(CChangePswdDialog);
	RegisterClass_(CCheckboxTable);
	RegisterClass_(CChooseDateDialog);
	RegisterClass_(CChooseServersDialog);
	RegisterClass_(CColorPopup);
	RegisterClass_(CCreateAdbkDialog);
	RegisterClass_(CCreateAccountDialog);
	RegisterClass_(CCreateCalendarDialog);
	RegisterClass_(CCreateMailboxDialog);
	RegisterClass_(CDateControl);
	RegisterClass_(CDateTimeZoneSelect);
	RegisterClass_(CDayWeekPrintout);
	RegisterClass_(CDayWeekView);
	RegisterClass_(CDayWeekTable);
	RegisterClass_(CDayWeekTitleTable);
	RegisterClass_(CDictionaryPageScroller);
	RegisterClass_(CDisconnectDialog);
	RegisterClass_(CDurationSelect);
	RegisterClass_(CEditAddressDialog);
	RegisterClass_(CEditFormattedTextDisplay);
	RegisterClass_(CEditGroupDialog);
	RegisterClass_(CEditIdentities);
	RegisterClass_(CEditIdentityAddress);
	RegisterClass_(CEditIdentityOptions);
	RegisterClass_(CEditIdentityOutgoing);
	RegisterClass_(CEditIdentityDSN);
	RegisterClass_(CEditIdentitySecurity);
	RegisterClass_(CEditMacro);
	RegisterClass_(CEditMappingsTable);
	RegisterClass_(CStyleToolbar);
	RegisterClass_(CErrorDialog);
	RegisterClass_(CEventPreview);
	RegisterClass_(CFileTable);
	RegisterClass_(CFindReplaceWindow);
	RegisterClass_(CFocusX);
	RegisterClass_(CFontPopup);
	RegisterClass_(CFormattedTextDisplay);
	RegisterClass_(CForwardActionDialog);
	RegisterClass_(CForwardOptionsDialog);
	RegisterClass_(CFreeBusyPrintout);
	RegisterClass_(CFreeBusyView);
	RegisterClass_(CFreeBusyTable);
	RegisterClass_(CFreeBusyTitleTable);
	RegisterClass_(CGetPassphraseDialog);
	RegisterClass_(CGetStringDialog);
	RegisterClass_(CGroupPreview);
	RegisterClass_(CGroupTable);
	RegisterClass_(CIconTextTable);
	RegisterClass_(CIdentityPopup);
	RegisterClass_(CIMAPLabelsDialog);
	RegisterClass_(CKeyChoiceDialog);
	RegisterClass_(CLetterPartProp);
	RegisterClass_(CLetterTextDisplay);
	RegisterClass_(CLetterToolbar);
	RegisterClass_(CLetterView);
	RegisterClass_(CLetterWindow);
	RegisterClass_(CLoggingOptionsDialog);
	RegisterClass_(CMacroEditDialog);
	RegisterClass_(CMacroEditTable);
	RegisterClass_(CMailboxInfoTable);
	RegisterClass_(CMailboxInfoToolbar);
	RegisterClass_(CMailboxInfoView);
	RegisterClass_(CMailboxInfoWindow);
	RegisterClass_(CMailboxPopup);
	RegisterClass_(CMailboxToolbarPopup);
	RegisterClass_(CMailboxPropDialog);
	RegisterClass_(CMailboxTitleTable);
	RegisterClass_(CMatchOptionsDialog);
	RegisterClass_(CMDNPromptDialog);
	RegisterClass_(CMessagePaneOptions);
	RegisterClass_(CMessagePartProp);
	RegisterClass_(CMessageSpeechEditDialog);
	RegisterClass_(CMessageSpeechTable);
	RegisterClass_(CMessageToolbar);
	RegisterClass_(CMessageSimpleView);
	RegisterClass_(CMessageView);
	RegisterClass_(CMessageWindow);
	RegisterClass_(CMonthIndicator);
	RegisterClass_(CMonthPopup);
	RegisterClass_(CMonthPrintout);
	RegisterClass_(CMonthView);
	RegisterClass_(CMonthTable);
	RegisterClass_(CMonthTitleTable);
	RegisterClass_(CMultiUserDialog);
	RegisterClass_(CNamespaceDialog);
	RegisterClass_(CNamespaceTable);
	RegisterClass_(CNetworkResetError);
	RegisterClass_(CNewACLDialog);
	RegisterClass_(CNewAttendeeDialog);
	RegisterClass_(CNewComponentDetails);
	RegisterClass_(CNewComponentRepeat);
	RegisterClass_(CNewComponentAlarm);
	RegisterClass_(CNewComponentAttendees);
	RegisterClass_(CNewEventDialog);
	RegisterClass_(CNewEventTiming);
	RegisterClass_(CNewSearchDialog);
	RegisterClass_(CNewToDoDialog);
	RegisterClass_(CNewToDoTiming);
	RegisterClass_(CNewWildcardDialog);
	RegisterClass_(CNumberEdit);
	RegisterClass_(COpenMailboxDialog);
	RegisterClass_(CPickPopup);
	RegisterClass_(CPluginRegisterDialog);
	RegisterClass_(CPreferencesDialog);
	RegisterClass_(CPrefsAccount);
	RegisterClass_(CPrefsAccountExtras);
	RegisterClass_(CPrefsAccountIMAP);
	RegisterClass_(CPrefsAccountAuth);
	RegisterClass_(CPrefsAccountLocal);
	RegisterClass_(CPrefsAccountPOP3);
	RegisterClass_(CPrefsAccountRemoteCalendar);
	RegisterClass_(CPrefsAdbkAccount);
	RegisterClass_(CPrefsAddress);
	RegisterClass_(CPrefsAddressExpansion);
	RegisterClass_(CPrefsAddressCapture);
	RegisterClass_(CPrefsAddressIMSP);
	RegisterClass_(CPrefsAddressLDAPOptions);
	RegisterClass_(CPrefsAddressLDAP1);
	RegisterClass_(CPrefsAddressLDAP2);
	RegisterClass_(CPrefsAlerts);
	RegisterClass_(CPrefsAlertsAttachment);
	RegisterClass_(CPrefsAlertsMessage);
	RegisterClass_(CPrefsAttachments);
	RegisterClass_(CPrefsAttachmentsSave);
	RegisterClass_(CPrefsAttachmentsSend);
	RegisterClass_(CPrefsAuthAnonymous);
	RegisterClass_(CPrefsAuthKerberos);
	RegisterClass_(CPrefsAuthPlainText);
	RegisterClass_(CPrefsCalendar);
	RegisterClass_(CPrefsConfigMessageSpeech);
	RegisterClass_(CPrefsDisplay);
	RegisterClass_(CPrefsDisplayLabel);
	RegisterClass_(CPrefsDisplayMailbox);
	RegisterClass_(CPrefsDisplayMessage);
	RegisterClass_(CPrefsDisplayQuotes);
	RegisterClass_(CPrefsDisplayServer);
	RegisterClass_(CPrefsDisplayStyles);
	RegisterClass_(CPrefsEditCaption);
	RegisterClass_(CPrefsEditHeadFoot);
	RegisterClass_(CPrefsEditMappings);
	RegisterClass_(CPrefsEditMappingDialog);
	RegisterClass_(CPrefsFormatting);
	RegisterClass_(CPrefsIdentities);
	RegisterClass_(CPrefsLDAPAccount);
	RegisterClass_(CPrefsLetter);
	RegisterClass_(CPrefsLetterGeneral);
	RegisterClass_(CPrefsLetterOptions);
	RegisterClass_(CPrefsLetterStyled);
	RegisterClass_(CPrefsLocalAccount);
	RegisterClass_(CPrefsRemoteCalendarAccount);
	RegisterClass_(CPrefsMailAccount);
	RegisterClass_(CPrefsMailbox);
	RegisterClass_(CPrefsMailboxBasic);
	RegisterClass_(CPrefsMailboxAdvanced);
	RegisterClass_(CPrefsMailboxDisconnected);
	RegisterClass_(CPrefsMailboxOptions);
	RegisterClass_(CPrefsMessage);
	RegisterClass_(CPrefsMessageGeneral);
	RegisterClass_(CPrefsMessageOptions);
	RegisterClass_(CPrefsPOP3Account);
	RegisterClass_(CPrefsRemoteAccount);
	RegisterClass_(CPrefsRemoteCalendarAccount);
	RegisterClass_(CPrefsRemoteOptions);
	RegisterClass_(CPrefsRemoteSets);
	RegisterClass_(CPrefsSecurity);
	RegisterClass_(CPrefsSimple);
	RegisterClass_(CPrefsSIEVEAccount);
	RegisterClass_(CPrefsSMTPAccount);
	RegisterClass_(CPrefsSMTPOptions);
	RegisterClass_(CPrefsSpeech);
	RegisterClass_(CPrefsSpelling);
	RegisterClass_(CPrefsSpellingOptions);
	RegisterClass_(CPrefsSpellingSuggestions);
	RegisterClass_(CPrefsSubTab);
	RegisterClass_(CPrintText);
	RegisterClass_(CProgressDialog);
	RegisterClass_(CPropAdbkACL);
	RegisterClass_(CPropAdbkGeneral);
	RegisterClass_(CPropCalendarACL);
	RegisterClass_(CPropCalendarGeneral);
	RegisterClass_(CPropCalendarOptions);
	RegisterClass_(CPropCalendarServer);
	RegisterClass_(CPropCalendarWebcal);
	RegisterClass_(CPropMailboxACL);
	RegisterClass_(CPropMailboxGeneral);
	RegisterClass_(CPropMailboxOptions);
	RegisterClass_(CPropMailboxQuota);
	RegisterClass_(CPropMailboxServer);
	RegisterClass_(CQuickSearchSavedPopup);
	RegisterClass_(CQuickSearchText);
	RegisterClass_(CQuotaTable);
	RegisterClass_(CRecurrenceDialog);
	RegisterClass_(CRegistrationDialog);
	RegisterClass_(CRejectActionDialog);
	RegisterClass_(CReplyActionDialog);
	RegisterClass_(CReplyChooseDialog);
	RegisterClass_(CReplyChooseTable);
	RegisterClass_(CRulesActionLocal);
	RegisterClass_(CRulesActionSIEVE);
	RegisterClass_(CRulesDialog);
	RegisterClass_(CRulesTarget);
	RegisterClass_(CRulesTable);
	RegisterClass_(CRulesWindow);
	RegisterClass_(CSaveDraftDialog);
	RegisterClass_(CScriptsTable);
	RegisterClass_(CSearchCriteriaContainer);
	RegisterClass_(CSearchCriteriaLocal);
	RegisterClass_(CSearchCriteriaSIEVE);
	RegisterClass_(CSearchOptionsDialog);
	RegisterClass_(CSearchWindow);
	RegisterClass_(CServerBrowseTable);
	RegisterClass_(CServerTable);
	RegisterClass_(CServerTitleTable);
	RegisterClass_(CServerToolbar);
	RegisterClass_(CServerView);
	RegisterClass_(CServerViewPopup);
	RegisterClass_(CServerWindow);
	RegisterClass_(CSimpleTitleTable);
	RegisterClass_(CSizePopup);
	RegisterClass_(CSMTPTable);
	RegisterClass_(CSMTPToolbar);
	RegisterClass_(CSMTPView);
	RegisterClass_(CSMTPWindow);
	RegisterClass_(CSoundPopup);
	RegisterClass_(CSpellAddDialog);
	RegisterClass_(CSpellCheckDialog);
	RegisterClass_(CSpellChooseDictDialog);
	RegisterClass_(CSpellEditDialog);
	RegisterClass_(CSpellOptionsDialog);
	RegisterClass_(CSplashScreen);
	RegisterClass_(CSplitDivider);
	RegisterClass_(CSplitterView);
	RegisterClass_(CStaticText);
	RegisterClass_(CStatusWindow);
	RegisterClass_(CSummaryPrintout);
	RegisterClass_(CSummaryView);
	RegisterClass_(CSummaryTable);
	RegisterClass_(CSummaryTitleTable);
	RegisterClass_(CSynchroniseDialog);
	RegisterClass_(CTabController);
	RegisterClass_(CTabsX);
	RegisterClass_(CTargetsDialog);
	RegisterClass_(CTextDisplay);
	RegisterClass_(CTextFieldX);
	RegisterClass_(CTextFieldMultiX);
	RegisterClass_(CTextHierarchy);
	RegisterClass_(CTextListChoice);
	RegisterClass_(CTextTable);
	RegisterClass_(CTimezonePopup);
	RegisterClass_(CToDoView);
	RegisterClass_(CToDoTable);
	RegisterClass_(CToggleIconButton);
	RegisterClass_(CToolbarButton);
	RegisterClass_(CToolbarView);
	RegisterClass_(CTooltip);
	RegisterClass_(CUploadScriptDialog);
	RegisterClass_(CUserActionOptions);
	RegisterClass_(CUserPswdDialog);
	RegisterClass_(CVacationActionDialog);
	RegisterClass_(CViewCertDialog);
	RegisterClass_(CWebKitControl);
	RegisterClass_(CWindowOptionsDialog);
	RegisterClass_(CYearPopup);
	RegisterClass_(CYearPrintout);
	RegisterClass_(CYearView);
	RegisterClass_(CYearTable);
	RegisterClass_(C3PaneAddress);
	RegisterClass_(C3PaneAddressBook);
	RegisterClass_(C3PaneAccounts);
	RegisterClass_(C3PaneCalendar);
	RegisterClass_(C3PaneCalendarAccount);
	RegisterClass_(C3PaneContactsAccount);
	RegisterClass_(C3PaneEvent);
	RegisterClass_(C3PaneItems);
	RegisterClass_(C3PaneItemsTabs);
	RegisterClass_(C3PaneAdbkToolbar);
	RegisterClass_(C3PaneCalendarToolbar);
	RegisterClass_(C3PaneMailboxToolbar);
	RegisterClass_(C3PaneMailAccount);
	RegisterClass_(C3PaneMailbox);
	RegisterClass_(C3PaneMainPanel);
	RegisterClass_(C3PaneMessage);
	RegisterClass_(C3PanePreview);
	RegisterClass_(C3PaneWindow);
	RegisterClass_(HCmdButtonAttachment);

	RegisterClassID_(CWebKitControlImp,		 CWebKitControl::imp_class_ID);

	// Derived attachments
	RegisterClass_(CHelpAttach);
	RegisterClass_(CHelpPaneAttach);
	RegisterClass_(CHelpControlAttach);
}
