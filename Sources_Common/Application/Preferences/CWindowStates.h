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


// Header for CWindowStates class

#ifndef __CWINDOWSTATES__MULBERRY__
#define __CWINDOWSTATES__MULBERRY__

#include "CAddressListFwd.h"
#include "CCalendarViewTypes.h"
#include "CDayWeekViewTimeRange.h"
#include "CFutureItems.h"
#include "CMatchItem.h"
#include "CMboxFwd.h"
#include "CMessageListFwd.h"
#include "CScreenDimensions.h"
#include "C3PaneWindowFwd.h"

#include "CWindowStatesFwd.h"

#if __dest_os == __linux_os
#include "JRect.h"
#endif

class CWindowState;
typedef std::vector<CWindowState*> CWindowStateArray;


class CWindowState
{
public:
	typedef CScreenDimensions<Rect> CScreenWindowSize;
	typedef prefsvector<CScreenWindowSize> CScreenMap;

	CWindowState();
	CWindowState(const char* name, Rect* bounds, EWindowState state);	// Build from window
	CWindowState(const CWindowState& copy)								// Copy constructor
		{ _copy(copy); }
	virtual ~CWindowState() {}

	int operator==(const char* name) const								// Compare with name
		{ return (mName == name); }

	CWindowState& operator=(const CWindowState& copy)					// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	const cdstring& GetName(void) const
		{ return mName; }
	cdstring& GetName(void)
		{ return mName; }

	const Rect& GetBestRect(const CWindowState& default_state);

	EWindowState GetState() const
		{ return mState; }

	virtual bool MergeToList(CWindowStateArray& list, const CWindowState& default_state, bool& list_changed);

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	cdstring		mName;
	CScreenMap		mBounds;
	EWindowState	mState;
	CFutureItems	mFuture;

	virtual void _copy(const CWindowState& copy);
	
	virtual void ResetRect(void);								// Set default size
	CScreenMap::const_iterator GetExactRect(const CScreenWindowSize& comp) const;
	CScreenMap::const_iterator GetClosestRect(const CScreenWindowSize& comp) const;
	void AddUniqueRect(CScreenWindowSize& item);

	virtual CWindowState* FindInList(const CWindowStateArray& list) const;
	virtual void PruneDefaults(const CWindowState& default_state);
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CTableWindowState : public CWindowState
{
public:
	typedef CScreenDimensions<CColumnInfoArray> CScreenColumnInfo;
	typedef prefsvector<CScreenColumnInfo> CColumnMap;

	CTableWindowState() { }
	CTableWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info);					// Build from window
	CTableWindowState(const CTableWindowState& copy)					// Copy constructor
		{ _copy(copy); }
	virtual ~CTableWindowState() {}

	CTableWindowState& operator=(const CTableWindowState& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	const CColumnInfoArray& GetBestColumnInfo(const CTableWindowState& default_state);

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs) = 0;

protected:
	CColumnMap	mColumnMap;

	virtual void _copy(const CWindowState& copy);
	
	virtual void ResetColumns(void) = 0;								// Set default columns
	CColumnMap::const_iterator GetExactColumnInfo(const CScreenColumnInfo& comp) const;
	CColumnMap::const_iterator GetClosestColumnInfo(const CScreenColumnInfo& comp) const;
	void AddUniqueColumnInfo(CScreenColumnInfo& item);

	virtual void PruneDefaults(const CWindowState& default_state);
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CStatusWindowState : public CWindowState
{
public:
	CStatusWindowState();
	CStatusWindowState(const char* name, Rect* bounds, EWindowState state, bool closed);	// Build from window
	CStatusWindowState(const CStatusWindowState& copy)					// Copy constructor
		{ _copy(copy); }
	virtual ~CStatusWindowState() {}

	CStatusWindowState& operator=(const CStatusWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	bool GetClosed(void) const
		{ return mClosed; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	bool mClosed;

	virtual void _copy(const CWindowState& copy);

	virtual bool PartialCompare(const CWindowState& default_state);
};

class CMDIWindowState : public CWindowState
{
public:
	CMDIWindowState();
	CMDIWindowState(const char* name, Rect* bounds, EWindowState state);// Build from window
	CMDIWindowState(const CMDIWindowState& copy);						// Copy constructor
	virtual ~CMDIWindowState() {}

protected:
	virtual void ResetRect(void);										// Set default size

	virtual bool PartialCompare(const CWindowState& default_state);
};

class C3PaneWindowState : public CWindowState
{
public:
	C3PaneWindowState();
	C3PaneWindowState(const char* name, Rect* bounds, EWindowState state,	// Build from window
						N3Pane::EGeometry geometry, bool show_list,
						bool show_items, bool show_preview, bool show_status,
						long splitter1_size, long splitter2_size,
						bool show_button_toolbar);
	C3PaneWindowState(const C3PaneWindowState& copy);						// Copy constructor
	virtual ~C3PaneWindowState() {}

	N3Pane::EGeometry GetGeometry() const
		{ return mGeometry; }
	bool GetShowList() const
		{ return mShowList; }
	bool GetShowItems() const
		{ return mShowItems; }
	bool GetShowPreview() const
		{ return mShowPreview; }
	bool GetShowStatus() const
		{ return mShowStatus; }
	long GetSplitter1Size() const
		{ return mSplitter1Size; }
	long GetSplitter2Size() const
		{ return mSplitter2Size; }
	bool GetShowButtonToolbar() const
		{ return mShowButtonToolbar; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	N3Pane::EGeometry mGeometry;
	bool	mShowList;
	bool	mShowItems;
	bool	mShowPreview;
	bool	mShowStatus;
	long	mSplitter1Size;
	long	mSplitter2Size;
	bool	mShowButtonToolbar;

	virtual void ResetRect(void);										// Set default size

	virtual bool PartialCompare(const CWindowState& default_state);
};

class CServerWindowState : public CTableWindowState
{
public:
	CServerWindowState();
	CServerWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info);					// Build from window
	CServerWindowState(const CServerWindowState& copy)					// Copy constructor
		{ _copy(copy); }
	virtual ~CServerWindowState() {}

	CServerWindowState& operator=(const CServerWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CMailboxWindowState : public CTableWindowState
{
public:
	CMailboxWindowState();
	CMailboxWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info,
						ESortMessageBy sort_by,
						EShowMessageBy show_by,
						NMbox::EViewMode view_mode,
						CMatchItem* match,
						long splitter);									// Build from window
	CMailboxWindowState(const CMailboxWindowState& copy)				// Copy constructor
		{ _copy(copy); }
	virtual ~CMailboxWindowState() {}

	int operator==(const CMailboxWindowState& state) const;				// Compare with same type

	CMailboxWindowState& operator=(const CMailboxWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	ESortMessageBy GetSortBy() const
		{ return mSortBy; }
	EShowMessageBy GetShowBy() const
		{ return mShowBy; }
	NMbox::EViewMode GetViewMode() const
		{ return mViewMode; }
	CMatchItem GetMatchItem() const
		{ return mMatchItem; }
	long GetSplitterSize() const
		{ return mSplitterSize; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	ESortMessageBy		mSortBy;
	EShowMessageBy		mShowBy;
	NMbox::EViewMode	mViewMode;
	CMatchItem			mMatchItem;
	long				mSplitterSize;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CMessageWindowState : public CWindowState
{
public:
	CMessageWindowState();
	CMessageWindowState(const char* name, Rect* bounds, EWindowState state,
						long split_change,
						bool parts_twisted,
						bool collapsed,
						bool flat,
						bool in_line);									// Build from window
	CMessageWindowState(const CMessageWindowState& copy)				// Copy constructor
		{ _copy(copy); }
	virtual ~CMessageWindowState() {}

	CMessageWindowState& operator=(const CMessageWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	long GetSplitChange() const
		{ return mSplitChange; }
	bool GetPartsTwisted() const
		{ return mPartsTwisted; }
	bool GetCollapsed() const
		{ return mCollapsed; }
	bool GetFlat() const
		{ return mFlat; }
	bool GetInline() const
		{ return mInline; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	long	mSplitChange;
	bool	mPartsTwisted;
	bool	mCollapsed;
	bool	mFlat;
	bool	mInline;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CLetterWindowState : public CWindowState
{
public:
	CLetterWindowState();
	CLetterWindowState(const char* name, Rect* bounds, EWindowState state,
						long split_change,
						bool parts_twisted,
						bool collapsed);							// Build from window
	CLetterWindowState(const CLetterWindowState& copy)				// Copy constructor
		{ _copy(copy); }
	virtual ~CLetterWindowState() {}

	CLetterWindowState& operator=(const CLetterWindowState& copy)	// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	long GetSplitChange() const
		{ return mSplitChange; }
	bool GetPartsTwisted() const
		{ return mPartsTwisted; }
	bool GetCollapsed() const
		{ return mCollapsed; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	long	mSplitChange;
	bool	mPartsTwisted;
	bool	mCollapsed;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CAddressBookWindowState : public CWindowState
{
public:
	CAddressBookWindowState();
	CAddressBookWindowState(const char* name, Rect* bounds, EWindowState state,
						ESortAddressBy sort,
						EShowAddressBy show,
#if __dest_os == __mac_os || __dest_os == __mac_os_x
						long split_change,
#elif __dest_os == __win32_os || __dest_os == __linux_os
						long address_size,
						long group_size,
#else
#error __dest_os
#endif
						bool addressShow,
						bool groupShow,
						bool visible);										// Build from window
	CAddressBookWindowState(const CAddressBookWindowState& copy)				// Copy constructor
		{ _copy(copy); }
	virtual ~CAddressBookWindowState() {}

	CAddressBookWindowState& operator=(const CAddressBookWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	ESortAddressBy GetSort() const
		{ return mSort; }
	EShowAddressBy GetShow() const
		{ return mShow; }
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	long GetSplitChange() const
		{ return mSplitChange; }
#elif __dest_os == __win32_os || __dest_os == __linux_os
	long GetAddressSize() const
		{ return mAddressSize; }
	long GetGroupSize() const
		{ return mGroupSize; }
#else
#error __dest_os
#endif
	bool GetAddressShow() const
		{ return mAddressShow; }
	bool GetGroupShow() const
		{ return mGroupShow; }
	bool GetVisible() const
		{ return mVisible; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	ESortAddressBy	mSort;
	EShowAddressBy	mShow;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	long			mSplitChange;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	long			mAddressSize;
	long			mGroupSize;
#else
#error __dest_os
#endif
	bool			mAddressShow;
	bool			mGroupShow;
	bool			mVisible;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CNewAddressBookWindowState : public CTableWindowState
{
public:
	CNewAddressBookWindowState();
	CNewAddressBookWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info,
						ESortAddressBy sort,
						long address_size,
						long group_size,
						bool addressShow,
						bool groupShow,
						long splitter);												// Build from window
	CNewAddressBookWindowState(const CNewAddressBookWindowState& copy)					// Copy constructor
		{ _copy(copy); }
	virtual ~CNewAddressBookWindowState() {}

	CNewAddressBookWindowState& operator=(const CNewAddressBookWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	ESortAddressBy GetSort() const
		{ return mSort; }
	long GetAddressSize() const
		{ return mAddressSize; }
	long GetGroupSize() const
		{ return mGroupSize; }
	bool GetAddressShow() const
		{ return mAddressShow; }
	bool GetGroupShow() const
		{ return mGroupShow; }
	long GetSplitterSize() const
		{ return mSplitterSize; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	ESortAddressBy	mSort;
	long			mAddressSize;
	long			mGroupSize;
	bool			mAddressShow;
	bool			mGroupShow;
	long			mSplitterSize;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CAdbkManagerWindowState : public CTableWindowState
{
public:
	CAdbkManagerWindowState();
	CAdbkManagerWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info,
						bool hide,
						cdstrvect* twisted);									// Build from window
	CAdbkManagerWindowState(const CAdbkManagerWindowState& copy)				// Copy constructor
		{ _copy(copy); }
	virtual ~CAdbkManagerWindowState() {}

	CAdbkManagerWindowState& operator=(const CAdbkManagerWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	bool GetHide() const
		{ return mHide; }
	const cdstrvect& GetTwisted() const
		{ return mTwisted; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	bool				mHide;
	cdstrvect			mTwisted;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CAdbkSearchWindowState : public CTableWindowState
{
public:
	CAdbkSearchWindowState();
	CAdbkSearchWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info,
						EAdbkSearchMethod method,
						EAddrColumn field,
						EAdbkSearchCriteria criteria,
						bool hide);												// Build from window
	CAdbkSearchWindowState(const CAdbkSearchWindowState& copy)					// Copy constructor
		{ _copy(copy); }
	virtual ~CAdbkSearchWindowState() {}

	CAdbkSearchWindowState& operator=(const CAdbkSearchWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	EAdbkSearchMethod GetMethod() const
		{ return mMethod; }
	EAddrColumn GetField() const
		{ return mField; }
	EAdbkSearchCriteria GetCriteria() const
		{ return mCriteria; }
	bool GetHide() const
		{ return mHide; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	EAdbkSearchMethod		mMethod;
	EAddrColumn				mField;
	EAdbkSearchCriteria		mCriteria;
	bool					mHide;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CFindReplaceWindowState : public CWindowState
{
public:
	CFindReplaceWindowState();
	CFindReplaceWindowState(const char* name, Rect* bounds, EWindowState state,
		bool case_sensitive, bool backwards, bool wrap, bool entire_word);	// Build from window
	CFindReplaceWindowState(const CFindReplaceWindowState& copy)			// Copy constructor
		{ _copy(copy); }
	virtual ~CFindReplaceWindowState() {}

	CFindReplaceWindowState& operator=(const CFindReplaceWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	bool GetCaseSensitive() const
		{ return mCaseSensitive; }
	bool GetBackwards() const
		{ return mBackwards; }
	bool GetWrap() const
		{ return mWrap; }
	bool GetEntireWord() const
		{ return mEntireWord; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	bool mCaseSensitive;
	bool mBackwards;
	bool mWrap;
	bool mEntireWord;

	virtual void _copy(const CWindowState& copy);
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CSearchWindowState : public CWindowState
{
public:
	CSearchWindowState();
	CSearchWindowState(const char* name, Rect* bounds, EWindowState state, bool expanded);	// Build from window
	CSearchWindowState(const CSearchWindowState& copy)					// Copy constructor
		{ _copy(copy); }
	virtual ~CSearchWindowState() {}

	CSearchWindowState& operator=(const CSearchWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	bool GetExpanded() const
		{ return mExpanded; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	bool mExpanded;

	virtual void _copy(const CWindowState& copy);
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CRulesWindowState : public CTableWindowState
{
public:
	CRulesWindowState();
	CRulesWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info,
						long split_pos, bool show_triggers, bool hide);			// Build from window
	CRulesWindowState(const CRulesWindowState& copy)							// Copy constructor
		{ _copy(copy); }
	virtual ~CRulesWindowState() {}

	CRulesWindowState& operator=(const CRulesWindowState& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	long GetSplitPos() const
		{ return mSplitPos; }
	bool GetShowTriggers() const
		{ return mShowTriggers; }
	bool GetHide() const
		{ return mHide; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	long				mSplitPos;
	bool				mShowTriggers;
	bool				mHide;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CCalendarStoreWindowState : public CTableWindowState
{
public:
	CCalendarStoreWindowState();
	CCalendarStoreWindowState(const char* name, Rect* bounds, EWindowState state,
						CColumnInfoArray* col_info,
						cdstrvect* twisted);										// Build from window
	CCalendarStoreWindowState(const CCalendarStoreWindowState& copy) :				// Copy constructor
		CTableWindowState(copy)
		{ _copy(copy); }
	virtual ~CCalendarStoreWindowState() {}

	CCalendarStoreWindowState& operator=(const CCalendarStoreWindowState& copy)		// Assignment with same type
		{ CTableWindowState::operator=(copy); if (this != &copy) _copy(copy); return *this; }

	const cdstrvect& GetTwisted() const
		{ return mTwisted; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	virtual bool SetInfo_Old(cdstring& info, NumVersion vers_prefs);

protected:
	cdstrvect			mTwisted;

	virtual void _copy(const CWindowState& copy);

	virtual void ResetRect(void);										// Set default size
	virtual void ResetColumns(void);									// Set default columns
	virtual bool PartialCompare(const CWindowState& default_state);
};

class CCalendarWindowState: public CWindowState
{
public:
	CCalendarWindowState();
	CCalendarWindowState(const char* name, Rect* bounds, EWindowState state,
						NCalendarView::EViewType type, bool show_todo,
						NCalendarView::EYearLayout year_layout,
						CDayWeekViewTimeRange::ERanges	dayweek_range,
						unsigned long day_week_scale,
						NCalendarView::ESummaryType summary_type,
						NCalendarView::ESummaryRanges summary_range,
						long splitter);											// Build from window
	CCalendarWindowState(const CCalendarWindowState& copy) :									// Copy constructor
		CWindowState(copy)
		{ _copy(copy); }
	virtual ~CCalendarWindowState() {}

	CCalendarWindowState& operator=(const CCalendarWindowState& copy)							// Assignment with same type
		{ CWindowState::operator=(copy); if (this != &copy) _copy(copy); return *this; }

	NCalendarView::EViewType GetType() const
	{
		return mType;
	}
	
	bool GetShowToDo() const
	{
		return mShowToDo;
	}

	NCalendarView::EYearLayout GetYearLayout() const
	{
		return mYearLayout;
	}
	
	CDayWeekViewTimeRange::ERanges GetDayWeekRange() const
	{
		return mDayWeekRange;
	}
	
	unsigned long GetDayWeekScale() const
	{
		return mDayWeekScale;
	}

	NCalendarView::ESummaryType GetSummaryType() const
	{
		return mSummaryType;
	}

	NCalendarView::ESummaryRanges GetSummaryRange() const
	{
		return mSummaryRange;
	}

	long GetSplitterSize() const
		{ return mSplitterSize; }

	virtual bool Merge(const CWindowState& state);

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& txt, NumVersion vers_prefs);

protected:
	NCalendarView::EViewType 		mType;
	bool							mShowToDo;
	NCalendarView::EYearLayout		mYearLayout;
	CDayWeekViewTimeRange::ERanges	mDayWeekRange;
	unsigned long					mDayWeekScale;
	NCalendarView::ESummaryType		mSummaryType;
	NCalendarView::ESummaryRanges	mSummaryRange;
	long							mSplitterSize;

private:
	void _copy(const CWindowState& copy);
	virtual bool PartialCompare(const CWindowState& default_state);
};

#endif
