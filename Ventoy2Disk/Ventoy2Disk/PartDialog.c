/******************************************************************************
 * PartDialog.c
 *
 * Copyright (c) 2020, longpanda <admin@ventoy.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Windows.h>
#include <commctrl.h>
#include "resource.h"
#include "Language.h"
#include "Ventoy2Disk.h"

static BOOL g_enable_reserve_space = FALSE;
static BOOL g_enable_reserve_space_tmp = FALSE;
static int g_unit_sel = 1;
static int g_unit_sel_tmp = 1;
static int g_reserve_space = -1;

int GetReservedSpaceInMB(void)
{
    if (g_enable_reserve_space)
    {
        if (g_unit_sel == 0)
        {
            return g_reserve_space;
        }
        else
        {
            return g_reserve_space * 1024;
        }
    }
    else
    {
        return 0;
    }
}


static VOID UpdateControlStatus(HWND hWnd)
{
	HWND hComobox;
	HWND hCheckbox;
	HWND hEdit;

	hCheckbox = GetDlgItem(hWnd, IDC_CHECK_RESERVE_SPACE);
	hComobox = GetDlgItem(hWnd, IDC_COMBO_SPACE_UNIT);
	hEdit = GetDlgItem(hWnd, IDC_EDIT_RESERVE_SPACE_VAL);

	SendMessage(hComobox, CB_SETCURSEL, g_unit_sel, 1);
	SendMessage(hCheckbox, BM_SETCHECK, g_enable_reserve_space_tmp ? BST_CHECKED : BST_UNCHECKED, 0);
	EnableWindow(hEdit, g_enable_reserve_space_tmp);
	EnableWindow(hComobox, g_enable_reserve_space_tmp);
}

static BOOL PartInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hComobox;
    HWND hCheckbox;
    HWND hEdit;
    WCHAR buf[64];

    hCheckbox = GetDlgItem(hWnd, IDC_CHECK_RESERVE_SPACE);
    hComobox = GetDlgItem(hWnd, IDC_COMBO_SPACE_UNIT);
    hEdit = GetDlgItem(hWnd, IDC_EDIT_RESERVE_SPACE_VAL);

    SetWindowText(hCheckbox, _G(STR_PRESERVE_SPACE));
    SetWindowText(GetDlgItem(hWnd, ID_PART_OK), _G(STR_BTN_OK));
    SetWindowText(GetDlgItem(hWnd, ID_PART_CANCEL), _G(STR_BTN_CANCEL));

	SendMessage(hEdit, EM_LIMITTEXT, 9, 0);

    SendMessage(hComobox, CB_ADDSTRING, 0, (LPARAM)TEXT(" MB"));
    SendMessage(hComobox, CB_ADDSTRING, 0, (LPARAM)TEXT(" GB"));
    
	if (g_reserve_space >= 0)
    {
		swprintf_s(buf, 64, L"%lld", (long long)g_reserve_space);
    }
    else
    {
        buf[0] = 0;
    }

    SetWindowText(hEdit, buf);

	g_enable_reserve_space_tmp = g_enable_reserve_space;
	g_unit_sel_tmp = g_unit_sel;

	UpdateControlStatus(hWnd);

    return TRUE;
}

static VOID OnPartBtnOkClick(HWND hWnd)
{
	HWND hEdit;
	ULONG SpaceVal = 0;
	CHAR Value[64] = { 0 };

	hEdit = GetDlgItem(hWnd, IDC_EDIT_RESERVE_SPACE_VAL);

	GetWindowTextA(hEdit, Value, sizeof(Value) - 1);
	
	SpaceVal = strtoul(Value, NULL, 10);

    if (g_enable_reserve_space_tmp)
    {
        if (g_unit_sel_tmp == 0)
        {
            if (SpaceVal == 0 || SpaceVal >= 1024 * 1024 * 2)
            {
                MessageBox(hWnd, _G(STR_SPACE_VAL_INVALID), _G(STR_ERROR), MB_OK | MB_ICONERROR);
                return;
            }
        }
        else
        {
            if (SpaceVal == 0 || SpaceVal >= 1024 * 2)
            {
                MessageBox(hWnd, _G(STR_SPACE_VAL_INVALID), _G(STR_ERROR), MB_OK | MB_ICONERROR);
                return;
            }
        }
    }

	g_reserve_space = (int)SpaceVal;
	g_enable_reserve_space = g_enable_reserve_space_tmp;
	g_unit_sel = g_unit_sel_tmp;

	SendMessage(hWnd, WM_CLOSE, 0, 0);
}

static VOID OnPartBtnCancelClick(HWND hWnd)
{
	SendMessage(hWnd, WM_CLOSE, 0, 0);
}

INT_PTR CALLBACK PartDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	INT index;
    WORD NotifyCode;
    WORD CtrlID;

    switch (Message)
    {
        case WM_COMMAND:
        {
            NotifyCode = HIWORD(wParam);
            CtrlID = LOWORD(wParam);
            
            if (CtrlID == ID_PART_OK && NotifyCode == BN_CLICKED)
            {
				OnPartBtnOkClick(hWnd);
            }
            else if (CtrlID == ID_PART_CANCEL && NotifyCode == BN_CLICKED)
            {
				OnPartBtnCancelClick(hWnd);
            }
			else if (CtrlID == IDC_CHECK_RESERVE_SPACE && NotifyCode == BN_CLICKED)
			{
				g_enable_reserve_space_tmp = !g_enable_reserve_space_tmp;
				UpdateControlStatus(hWnd);
			}
			else if (CtrlID == IDC_COMBO_SPACE_UNIT && NotifyCode == CBN_SELCHANGE)
			{
				index = (INT)SendMessage(GetDlgItem(hWnd, IDC_COMBO_SPACE_UNIT), CB_GETCURSEL, 0, 0);
				if (index != CB_ERR)
				{
					g_unit_sel_tmp = index;
				}
			}
            else
            {
                return TRUE;
            }

            break;
        }
        case WM_INITDIALOG:
        {
            PartInitDialog(hWnd, wParam, lParam);
            return FALSE;
        }        
        case WM_CLOSE:
        {            
            EndDialog(hWnd, 0);
            return TRUE;
        }
    }

    return 0;
}
