#include "stdafx.h"
#include "FolderOptions.h"
#include "ExplorerBHO.h"

#include "helpers.h"

HRESULT CExplorerBHO::FinalConstruct()
{
	ATLTRACE("CExplorerBHO::FinalConstruct\n");

	//don't load BHO in IE7/8
	TCHAR szExePath[MAX_PATH];
	if (!GetModuleFileName(NULL, szExePath, MAX_PATH)) return HRESULT_FROM_WIN32(GetLastError());
	if (StrCmpI(PathFindFileName(szExePath), _T("iexplore.exe"))==0) return E_ABORT;

return S_OK;
}
void CExplorerBHO::FinalRelease()
{
	ATLTRACE("CExplorerBHO::FinalRelease\n");
}


HRESULT CExplorerBHO::SetSite(IUnknown *pUnkSite)
{
	IObjectWithSiteImpl<CExplorerBHO>::SetSite(pUnkSite);

	if (pUnkSite)
	{
		LoadFolderSettings(&m_dwSettings);//settings apply on every new window/site

		CComPtr<IServiceProvider> psp;
		if SUCCEEDED(pUnkSite->QueryInterface(IID_IServiceProvider,(void**)&psp))
		{
			if (SUCCEEDED(psp->QueryService( SID_SWebBrowserApp, IID_IWebBrowser2, (void**)&m_pWebBrowser2)) &&
				SUCCEEDED(psp->QueryService(SID_SShellBrowser, IID_IShellBrowser, (void**)&m_pShellBrowser)))
			{
				if (m_dwEventCookie==0xFEFEFEFE)
					DispEventAdvise( m_pWebBrowser2, &IID_IDispatch);

				//must set FVO_CUSTOMPOSITION here for fullrowselect/headers to work properly
				//see remarks in http://msdn.microsoft.com/en-us/library/bb775546%28v=vs.85%29.aspx
				if (m_dwSettings & (FO_HEADERS|FO_NOFULLROWSELECT|FO_CUSTOMORDERING))
				{
					CComPtr<IFolderViewOptions> pfvo;
					if SUCCEEDED(m_pShellBrowser->QueryInterface(IID_IFolderViewOptions, (void**)&pfvo))//query on IShellBrowser, not IShellView
					{
						//seems that FVO_CUSTOMPOSITION has same effect as FVO_CUSTOMORDERING
						pfvo->SetFolderViewOptions(FVO_CUSTOMPOSITION|FVO_CUSTOMORDERING,FVO_CUSTOMPOSITION|FVO_CUSTOMORDERING);
					}
				}
			}
		}
	}
	else
	{
		ATLTRACE("SetSite:: NULL\n");
		if ((m_dwEventCookie != 0xFEFEFEFE) && m_pWebBrowser2)
			DispEventUnadvise(m_pWebBrowser2, &IID_IDispatch);

		m_pWebBrowser2.Release();
		m_pShellBrowser.Release();
	}
return S_OK;//SetSite allways returns S_OK
}


HWND CExplorerBHO::FindShellLV(IShellBrowser* pshb, LPCTSTR lpszClass)
{
	ATLASSERT(pshb);
	CComPtr<IOleWindow> pow;
	if (S_OK!=pshb->QueryInterface(IID_IOleWindow, (void**)&pow)) return NULL;
	HWND hw=NULL;
	if (S_OK!=pow->GetWindow(&hw)) return NULL;
	//IShellBrowser gives ShellTabWindowClass hwnd
	if (!hw) return NULL;
	
	hw=FindWindowEx(hw, NULL, _T("DUIViewWndClassName"), NULL);
	if (hw)
	{
		hw=FindWindowEx(hw, NULL, _T("DirectUIHWND"), NULL);
		if (hw)
		{
			//local class callback trick
			class CEnumChildWindows
			{
			public:
				CEnumChildWindows(LPCTSTR lpszClass): hwChild(NULL), m_lpszClass(lpszClass) {}
				
				void EnumChildWindows(HWND hWnd) { ::EnumChildWindows(hWnd, EnumChildWindowProcImpl, (LPARAM)this); }
				
				BOOL EnumChildWindowProc(HWND hwnd)
				{
					//find CtrlNotifySink >>> SHELLDLL_DefView >>> SysListView32 or DirectUIHWND
					HWND hw=::FindWindowEx(hwnd, NULL, _T("SHELLDLL_DefView"), NULL);
					if (hw)
					{
						hwChild=::FindWindowEx(hw, NULL, m_lpszClass, NULL);
						if (hwChild) return FALSE;//stop
					}
				return TRUE;//continue
				}
				
			protected:
				static BOOL CALLBACK EnumChildWindowProcImpl(HWND hwnd, LPARAM lParam)
				{
					return ((CEnumChildWindows*)lParam)->EnumChildWindowProc(hwnd);
				}
				
			public:
				HWND hwChild;
			private:
				LPCTSTR m_lpszClass;

			} ecw(lpszClass);
			
			ecw.EnumChildWindows(hw);
			
			if (ecw.hwChild) return ecw.hwChild;	
		}
	}
return NULL;
}


HRESULT CExplorerBHO::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
					DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	HRESULT hr=IDispatchImpl<IExplorerBHO, &IID_IExplorerBHO,
								&LIBID_FOLDEROPTIONSLib,1>::Invoke(dispidMember,
													riid, lcid,wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

	if (dispidMember==DISPID_ONQUIT)
		if((m_dwEventCookie!=0xFEFEFEFE) && m_pWebBrowser2)
			return DispEventUnadvise(m_pWebBrowser2, &IID_IDispatch);

	if (m_pShellBrowser==NULL) return hr;


	HWND hwLV=NULL;
	if (dispidMember==DISPID_DOCUMENTCOMPLETE)
		hwLV=FindShellLV(m_pShellBrowser,
					(m_dwSettings & (FO_HEADERS|FO_NOFULLROWSELECT|FO_CUSTOMORDERING)) ? _T("SysListView32") : _T("DirectUIHWND"));
	ATLTRACE("CExplorerBHO:: hwLV=%x", hwLV);

	//set flags in all events to get correct vertical scrollbar
	CComPtr<IShellView> psv;
	if SUCCEEDED(m_pShellBrowser->QueryActiveShellView(&psv))
	{
		CComPtr<IFolderView2> pfv2;
		if SUCCEEDED(psv->QueryInterface(IID_IFolderView2, (void**)&pfv2))
		{
			if (m_dwSettings & FO_NOFULLROWSELECT) pfv2->SetCurrentFolderFlags(FWF_FULLROWSELECT,0);

			if (m_dwSettings & FO_HEADERS)
			{
				if (S_OK==pfv2->SetCurrentFolderFlags(FWF_NOCOLUMNHEADER|FWF_NOHEADERINALLVIEWS,0))//both flags for headers
				{
					//fix scrollbar offset
					if (dispidMember==DISPID_NAVIGATECOMPLETE2) m_bNavComplete2=TRUE;//chain of events
					if (dispidMember==DISPID_DOCUMENTCOMPLETE && m_bNavComplete2) //navigation could be canceled
					{
						m_bNavComplete2=FALSE;

						if (hwLV)
						{
							HWND hwHDR=::FindWindowEx(hwLV, NULL, _T("SysHeader32"), NULL);
							if (hwHDR)
							{
								RECT rh;
								if (::GetWindowRect(hwHDR, &rh))
									ListView_Scroll(hwLV, 0, (rh.top-rh.bottom));
							}
						}
					}
				}
			}
		}
	}


	if ((m_dwSettings & FO_LVFOCUS) && (dispidMember==DISPID_DOCUMENTCOMPLETE))
	{
		if (hwLV)
		{
			if ((GetFocus()!=hwLV) && !(GetAsyncKeyState(VK_SHIFT)<0))
				SetFocus(hwLV);
		}
	}

return hr;
}

