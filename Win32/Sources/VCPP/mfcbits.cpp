
OLE_DATA _oleData;

OLE_DATA::OLE_DATA()
{
	// OLE 1.0 Clipboard formats
	cfNative = ::RegisterClipboardFormat(_T("Native"));
	cfOwnerLink = ::RegisterClipboardFormat(_T("OwnerLink"));
	cfObjectLink = ::RegisterClipboardFormat(_T("ObjectLink"));

	// OLE 2.0 Clipboard formats
	cfEmbeddedObject = ::RegisterClipboardFormat(_T("Embedded Object"));
	cfEmbedSource = ::RegisterClipboardFormat(_T("Embed Source"));
	cfLinkSource = ::RegisterClipboardFormat(_T("Link Source"));
	cfObjectDescriptor = ::RegisterClipboardFormat(_T("Object Descriptor"));
	cfLinkSourceDescriptor = ::RegisterClipboardFormat(_T("Link Source Descriptor"));
	cfFileName = ::RegisterClipboardFormat(_T("FileName"));
	cfFileNameW = ::RegisterClipboardFormat(_T("FileNameW"));
	cfRichTextFormat = ::RegisterClipboardFormat(_T("Rich Text Format"));
	cfRichTextAndObjects = ::RegisterClipboardFormat(_T("RichEdit Text and Objects"));
}

const TCHAR _afxWndControlBar[] = AFX_WNDCONTROLBAR;

void AFXAPI _AfxDrawBorders(CDC* pDC, CRect& rc, BOOL bBorder, BOOL bClientEdge)
{
	if (bBorder)
		::DrawEdge(pDC->m_hDC, &rc, (bClientEdge ? BDR_INNER : BDR_OUTER),
			BF_RECT | BF_ADJUST | (bClientEdge ? BF_FLAT : BF_MONO));

	if (bClientEdge)
		::DrawEdge(pDC->m_hDC, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
}

BOOL AFXAPI _AfxIsComboBoxControl(HWND hWnd, UINT nStyle)
{
	if (hWnd == NULL)
		return FALSE;
	// do cheap style compare first
	if ((UINT)(::GetWindowLong(hWnd, GWL_STYLE) & 0x0F) != nStyle)
		return FALSE;

	// do expensive classname compare next
	TCHAR szCompare[_countof("combobox")+1];
	::GetClassName(hWnd, szCompare, _countof(szCompare));
	return ::AfxInvariantStrICmp(szCompare, _T("combobox")) == 0;
}

HWND AFXAPI _AfxChildWindowFromPoint(HWND hWnd, POINT pt)
{
	ASSERT(hWnd != NULL);

	// check child windows
	::ClientToScreen(hWnd, &pt);
	HWND hWndChild = ::GetWindow(hWnd, GW_CHILD);
	for (; hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
	{
		if (_AfxGetDlgCtrlID(hWndChild) != (WORD)0 &&
			(::GetWindowLong(hWndChild, GWL_STYLE) & WS_VISIBLE))
		{
			// see if point hits the child window
			CRect rect;
			::GetWindowRect(hWndChild, rect);
			if (rect.PtInRect(pt))
				return hWndChild;
		}
	}

	return NULL;    // not found
}

BOOL __afxGotScrollLines = FALSE; // defined in wincore.cpp

UINT PASCAL _AfxGetMouseScrollLines()
{
	static UINT uCachedScrollLines;

	// if we've already got it and we're not refreshing,
	// return what we've already got

	if (__afxGotScrollLines)
		return uCachedScrollLines;

	// see if we can find the mouse window

	__afxGotScrollLines = TRUE;

	static UINT msgGetScrollLines;
	static WORD nRegisteredMessage;

	if (afxData.bWin95)
	{
		if (nRegisteredMessage == 0)
		{
			msgGetScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
			if (msgGetScrollLines == 0)
				nRegisteredMessage = 1;     // couldn't register!  never try again
			else
				nRegisteredMessage = 2;     // it worked: use it
		}

		if (nRegisteredMessage == 2)
		{
			HWND hwMouseWheel = NULL;
			hwMouseWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
			if (hwMouseWheel && msgGetScrollLines)
			{
				uCachedScrollLines = (UINT)
					::SendMessage(hwMouseWheel, msgGetScrollLines, 0, 0);
			}
		}
	}
	else
	{
		// couldn't use the window -- try system settings
		uCachedScrollLines = 3; // reasonable default
		if (!afxData.bWin95)
			::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);
		}

	return uCachedScrollLines;
}

