#ifndef _REGISTRY_78FC5EFB_5023_42B3_8152_168077A8FC1D_
#define _REGISTRY_78FC5EFB_5023_42B3_8152_168077A8FC1D_
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

//HKCU (per-user settings)
#define FO_APP_KEY _T("Software\\T800 Productions\\{0AE87E97-08ED-4D43-ADA3-ADD3166FC4D2}")

#define FO_HEADERS  0x00000001
#define FO_NOFULLROWSELECT  0x00000002
#define FO_CUSTOMORDERING 0x00000004
#define FO_LVFOCUS  0x00000008
#define FO_DEFAULTSETTINGS  (FO_HEADERS|FO_NOFULLROWSELECT|FO_CUSTOMORDERING|FO_LVFOCUS)



inline void LoadFolderSettings(DWORD* pdwSettings)
{
	*pdwSettings=FO_DEFAULTSETTINGS;
	DWORD dwValue;
	DWORD dwType;
	DWORD pcbData=sizeof(DWORD);
	LSTATUS ls=SHGetValue(HKEY_CURRENT_USER, FO_APP_KEY, _T("FolderFlags"), &dwType, (LPBYTE)(&dwValue), &pcbData);
	if (ls==ERROR_SUCCESS && dwType==REG_DWORD)
		*pdwSettings=dwValue;
}

inline BOOL ApplyFolderSettings(DWORD dwSettings)
{
return (ERROR_SUCCESS==SHSetValue(HKEY_CURRENT_USER, FO_APP_KEY, _T("FolderFlags"), REG_DWORD, (LPCVOID)(&dwSettings), sizeof(DWORD)));
}

#endif//_REGISTRY_78FC5EFB_5023_42B3_8152_168077A8FC1D_



