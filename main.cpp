#include <windows.h>  
#include "resource1.h"

#define WM_TRAYEVENT (WM_USER + 1)  // ���̻ص�
#define ID_TRAYICON 1  // ��������ͼ��ID  
#define Q_TIME_DUR 200 //��ס��Q��������룩

HWND hWnd; // ���ھ��
NOTIFYICONDATA nid; // ����ͼ�����ݽṹ  
BOOL g_bQQQFlag = FALSE;
BOOL g_bIsQDown = FALSE;
BOOL g_bIsCtrlDown = FALSE;
BOOL g_bIsShitfDown = FALSE;

HICON hIcon0, hIcon1; //����ͼ��

// ��ʱ��ID  
#define TIMER_ID 1

//��������ͼ��
void UpdateTrayIcon() {
    if (g_bQQQFlag) {
        nid.hIcon = hIcon1;
    }
    else {
        nid.hIcon = hIcon0;
    }
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//���������Ӻ���
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* kbInfo = (KBDLLHOOKSTRUCT*)lParam;
            if (kbInfo->vkCode == 'Q') {
                g_bIsQDown = TRUE; // ���Q��������
                if (g_bIsCtrlDown && g_bIsShitfDown) {
                    g_bQQQFlag = !g_bQQQFlag;
                    UpdateTrayIcon();
                }
            }
            else if (kbInfo->vkCode == VK_LSHIFT || kbInfo->vkCode == VK_RSHIFT) {
                g_bIsShitfDown = TRUE;
            }
            else if (kbInfo->vkCode == VK_LCONTROL || kbInfo->vkCode == VK_RCONTROL) {
                g_bIsCtrlDown = TRUE;
            }
        }
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            KBDLLHOOKSTRUCT* kbInfo = (KBDLLHOOKSTRUCT*)lParam;
            if (kbInfo->vkCode == 'Q') { // ����Ƿ���Q��  
                g_bIsQDown = FALSE; // ���Q�����ɿ� 
            }
            else if (kbInfo->vkCode == VK_LSHIFT || kbInfo->vkCode == VK_RSHIFT) {
                g_bIsShitfDown = FALSE;
            }
            else if (kbInfo->vkCode == VK_LCONTROL || kbInfo->vkCode == VK_RCONTROL) {
                g_bIsCtrlDown = FALSE;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ����ϵͳ���̲˵�����ʾ
void ShowSysTrayMenu(HWND hWnd, POINT pt)
{
    
    HMENU hMenu = CreatePopupMenu();
    if (g_bQQQFlag) {
        AppendMenu(hMenu, MF_STRING, 2, L"�ر�QQQ");
    }
    else {
        AppendMenu(hMenu, MF_STRING, 2, L"����QQQ");
    }
    AppendMenu(hMenu, MF_STRING, 3, L"����");
    AppendMenu(hMenu, MF_STRING, 1, L"�˳�");
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, NULL, hWnd, NULL);
    DestroyMenu(hMenu);
}

// ���ڹ��̺���  
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        // ��������ʱ���ö�ʱ��  
        SetTimer(hWnd, TIMER_ID, Q_TIME_DUR, NULL);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_TRAYEVENT:
       if (lParam == WM_RBUTTONDOWN) {
            // ��������¼�
            POINT pt;
            GetCursorPos(&pt);
            ShowSysTrayMenu(hWnd, pt);
        }
        break;
    case WM_TIMER:
        if (wParam == TIMER_ID && g_bQQQFlag && g_bIsQDown) {
            if (!g_bIsCtrlDown && !g_bIsShitfDown) {
                keybd_event('Q', 0, 0, 0);
                keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
            }
        }
        break;
    case WM_COMMAND:
        {
            //�˵�ID
            WORD  menuId = LOWORD(wParam);
            switch (menuId)
            {
            case 1:
                PostQuitMessage(0);
                break;
            case 2:
                g_bQQQFlag = !g_bQQQFlag;
                UpdateTrayIcon();
                break;
            case 3:
                MessageBox(NULL, L"QQQ����������ߣ�������ùر�:Ctrl+Shift+Q", L"��ʾ", MB_ICONINFORMATION | MB_OK);
                break;
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ��ʼ������ͼ��  
void InitTrayIcon(HINSTANCE hInstance) {
    //����ͼ��
    hIcon0 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
    hIcon1 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    
    // ��ʼ������ͼ������  
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYEVENT;
    if (g_bQQQFlag) {
        nid.hIcon = hIcon1;
    }
    else {
        nid.hIcon = hIcon0;
    }
    lstrcpyW(nid.szTip, L"QQQ");

    Shell_NotifyIcon(NIM_ADD, &nid); // �������ͼ��  
}

// ��������ͼ��  
void CleanupTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid); // ɾ������ͼ��  
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const WCHAR g_szClassName[] = L"TrayIconClass";
    WNDCLASSEX wc;
    MSG Msg;
    wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = g_szClassName;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"����ע��ʧ�ܣ�", L"����", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    hWnd = CreateWindowEx(0, g_szClassName, L"", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        MessageBox(NULL, L"���ڴ���ʧ�ܣ�", L"����", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    InitTrayIcon(hInstance);
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (hHook == NULL) {
        MessageBox(NULL, L"��װ����ʧ�ܣ�", L"����", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    CleanupTrayIcon();
    return Msg.lParam;
}