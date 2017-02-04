#include "stdafx.h"
#include "ExplorerBHO.h"
#include "resource.h"
#include <initguid.h>
#include "FolderOptions.h"

#include "FolderOptions_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ExplorerBHO, CExplorerBHO)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_FOLDEROPTIONSLib);
    }
    else
		if (dwReason == DLL_PROCESS_DETACH)
			_Module.Term();
return TRUE;
}

STDAPI DllCanUnloadNow(void){return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv){return _Module.GetClassObject(rclsid, riid, ppv);}

STDAPI DllRegisterServer(void){return _Module.RegisterServer(TRUE);}

STDAPI DllUnregisterServer(void){return _Module.UnregisterServer(TRUE);}


