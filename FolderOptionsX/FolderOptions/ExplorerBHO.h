#ifndef _EXPLORERBHO_99B69ACD_5ADF_4FD0_863C_158A9D3E736E_
#define _EXPLORERBHO_99B69ACD_5ADF_4FD0_863C_158A9D3E736E_

#include "resource.h"       // main symbols
#include "FolderOptions.h"
#include "settings.h"

#include <shlobj.h>
#include <comdef.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <exdispid.h>

#pragma comment(lib,"atlthunk.lib") //resolves error LNK2019 'missing CComStdCallThunkHelper()' for x64 build


/////////////////////////////////////////////////////////////////////////////
// CExplorerBHO

class ATL_NO_VTABLE CExplorerBHO : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CExplorerBHO, &CLSID_ExplorerBHO>,
	public IObjectWithSiteImpl<CExplorerBHO>,
	public IDispatchImpl<IExplorerBHO, &IID_IExplorerBHO, &LIBID_FOLDEROPTIONSLib,/*wMajor =*/ 1 /*, wMinor = 0*/>
{
public:
	CExplorerBHO(): m_dwEventCookie(0xFEFEFEFE), m_bNavComplete2(FALSE), m_dwSettings(FO_DEFAULTSETTINGS) {}

	BEGIN_COM_MAP(CExplorerBHO)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IObjectWithSite)
	END_COM_MAP()

	DECLARE_NOT_AGGREGATABLE(CExplorerBHO) 
	DECLARE_REGISTRY_RESOURCEID(IDR_ExplorerBHO)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

	//IObjectWithSite override
	STDMETHOD(SetSite)(IUnknown *pUnkSite);

	//IDispatch override
	STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);

private:
	CComPtr<IWebBrowser2> m_pWebBrowser2;
	CComPtr<IShellBrowser> m_pShellBrowser;
	DWORD m_dwSettings;
	BOOL m_bNavComplete2;
	HWND FindShellLV(IShellBrowser* pshb, LPCTSTR lpszClass);

	//from _IDispEvent
	DWORD m_dwEventCookie;
	HRESULT DispEventAdvise(IUnknown* pUnk, const IID* piid)
	{
		ATLENSURE(m_dwEventCookie == 0xFEFEFEFE);
		return AtlAdvise(pUnk, reinterpret_cast<IUnknown*>(this), *piid, &m_dwEventCookie);
	}
	HRESULT DispEventUnadvise(IUnknown* pUnk, const IID* piid)
	{
		HRESULT hr = AtlUnadvise(pUnk, *piid, m_dwEventCookie);
		m_dwEventCookie = 0xFEFEFEFE;
		return hr;
	}

};

#endif//_EXPLORERBHO_99B69ACD_5ADF_4FD0_863C_158A9D3E736E_
