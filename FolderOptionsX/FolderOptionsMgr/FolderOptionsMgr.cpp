#include "stdafx.h"
#include <Commctrl.h>
#pragma comment(lib,"Comctl32.lib")
#include "resource.h"
#include "helpers.h"
#include "..\FolderOptions\settings.h"


void OnApplySettings(HWND hDlg)
{
	DWORD dwSettings=0;

	dwSettings|=(BST_CHECKED==Button_GetCheck(GetDlgItem(hDlg, IDC_NOFULLROWSELECT))) ? FO_NOFULLROWSELECT : 0;
	dwSettings|=(BST_CHECKED==Button_GetCheck(GetDlgItem(hDlg, IDC_HEADERS))) ? FO_HEADERS : 0;
	dwSettings|=(BST_CHECKED==Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMORDERING))) ? FO_CUSTOMORDERING : 0;
	dwSettings|=(BST_CHECKED==Button_GetCheck(GetDlgItem(hDlg, IDC_FOCUS))) ? FO_LVFOCUS : 0;

	if (!ApplyFolderSettings(dwSettings)) MessageBeep(MB_ICONERROR);
}

//window proc for "framed white background" control
WNDPROC pOldWndProc;
LRESULT CALLBACK StaticBkgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg==WM_PAINT)
	{
		if (GetUpdateRect(hWnd, NULL, FALSE))
		{
			PAINTSTRUCT pPaint;
			HDC hDc=BeginPaint(hWnd, &pPaint);
			if (hDc)
			{
				RECT rc;
				if (GetClientRect(hWnd, &rc))
				{
					FillRect(hDc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
					FrameRect(hDc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
				}
			}
			EndPaint(hWnd, &pPaint);
		}
	return 0;
	}
return CallWindowProc(pOldWndProc, hWnd, uMsg, wParam, lParam);
}


INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			DBGTRACE("WM_INITDIALOG\n");

			//set dlg icon
			HICON hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FOLDEROPTIONS), 
				IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
			if (hIcon) SendMessage(hDlg, WM_SETICON, TRUE, (LPARAM)hIcon);
			HICON hIconSmall = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FOLDEROPTIONS), 
				IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
			if (hIconSmall) SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)hIconSmall);

			//set checkboxes
			DWORD dwsettings;
			LoadFolderSettings(&dwsettings);
			Button_SetCheck(GetDlgItem(hDlg, IDC_NOFULLROWSELECT), ((dwsettings & FO_NOFULLROWSELECT) ? BST_CHECKED : BST_UNCHECKED));
			Button_SetCheck(GetDlgItem(hDlg, IDC_HEADERS), ((dwsettings & FO_HEADERS) ? BST_CHECKED : BST_UNCHECKED));
			Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMORDERING), ((dwsettings & FO_CUSTOMORDERING) ? BST_CHECKED : BST_UNCHECKED));
			Button_SetCheck(GetDlgItem(hDlg, IDC_FOCUS), ((dwsettings & FO_LVFOCUS) ? BST_CHECKED : BST_UNCHECKED));

			pOldWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_STATICBKG), GWLP_WNDPROC, (LONG_PTR)StaticBkgProc);

			CenterWindow(hDlg);
		}
		return (INT_PTR)TRUE;

	case WM_CTLCOLORSTATIC:
		switch (GetDlgCtrlID((HWND)lParam))
		{
			//controls on white bkg
			case IDC_COPYRIGHT:
				SetTextColor((HDC)wParam,GetSysColor(COLOR_GRAYTEXT));
			case IDC_STATICBKG:
			case IDC_GROUP1:
			case IDC_NOFULLROWSELECT:
			case IDC_HEADERS:
			case IDC_CUSTOMORDERING:
			case IDC_FOCUS:
			case IDC_CUSTOM1:
			case IDC_CUSTOM2:
			case IDC_CUSTOM3:
			case IDC_CUSTOM4:
				return (UINT_PTR)GetStockObject(WHITE_BRUSH);
		default:break;
		}
		break;

	case WM_COMMAND:
		switch LOWORD(wParam)
		{

			//CHECKBOX (not AUTOCHECKBOX) buttons need settings state manualy
			case IDC_HEADERS:
			case IDC_NOFULLROWSELECT:
			case IDC_CUSTOMORDERING:
				if (HIWORD(wParam)==BN_CLICKED)
				{
					BOOL bstate=!((BOOL)Button_GetCheck(GetDlgItem(hDlg, LOWORD(wParam))));
					Button_SetCheck(GetDlgItem(hDlg, LOWORD(wParam)), bstate);

					//fullrowselect and headers depend on custom ordering
					if ((LOWORD(wParam)!=IDC_CUSTOMORDERING) && (bstate==BST_CHECKED))
						Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMORDERING), bstate);
					if ((LOWORD(wParam)==IDC_CUSTOMORDERING) && (bstate==BST_UNCHECKED))
					{
						Button_SetCheck(GetDlgItem(hDlg, IDC_HEADERS), BST_UNCHECKED);
						Button_SetCheck(GetDlgItem(hDlg, IDC_NOFULLROWSELECT), BST_UNCHECKED);				
					}
				}
				break;

			case IDC_APPLY:
				OnApplySettings(hDlg);
				break;
			case IDOK:
				OnApplySettings(hDlg);//apply and fallthrough
			case IDCANCEL:
				DBGTRACE("EndDialog\n");
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
		default:break;
		}

	break;
	}
return (INT_PTR)FALSE;
}


//main
#ifdef _ATL_MIN_CRT //release build
int WINAPI WinMainCRTStartup(void)//no crt
#else
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
#endif
{
	//needed for XP, Win7/Vista loads common controls automatically?
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAINDLG), HWND_DESKTOP, DlgProc);
#ifdef _ATL_MIN_CRT
	ExitProcess(0);
#endif
return 0;
}



