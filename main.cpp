#include <windows.h>  
#include "resource1.h"

#define WM_TRAYEVENT (WM_USER + 1)  // 托盘回调
#define ID_TRAYICON 1  // 定义托盘图标ID  
#define Q_TIME_DUR 200 //按住后Q间隔（毫秒）

HWND hWnd; // 窗口句柄
NOTIFYICONDATA nid; // 托盘图标数据结构  
BOOL g_bQQQFlag = FALSE;
BOOL g_bIsQDown = FALSE;
BOOL g_bIsCtrlDown = FALSE;
BOOL g_bIsShitfDown = FALSE;

HICON hIcon0, hIcon1; //托盘图标

// 定时器ID  
#define TIMER_ID 1

//更新托盘图标
void UpdateTrayIcon() {
    if (g_bQQQFlag) {
        nid.hIcon = hIcon1;
    }
    else {
        nid.hIcon = hIcon0;
    }
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//按键处理钩子函数
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* kbInfo = (KBDLLHOOKSTRUCT*)lParam;
            if (kbInfo->vkCode == 'Q') {
                g_bIsQDown = TRUE; // 标记Q键被按下
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
            if (kbInfo->vkCode == 'Q') { // 检查是否是Q键  
                g_bIsQDown = FALSE; // 标记Q键被松开 
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

// 创建系统托盘菜单并显示
void ShowSysTrayMenu(HWND hWnd, POINT pt)
{
    
    HMENU hMenu = CreatePopupMenu();
    if (g_bQQQFlag) {
        AppendMenu(hMenu, MF_STRING, 2, L"关闭QQQ");
    }
    else {
        AppendMenu(hMenu, MF_STRING, 2, L"启动QQQ");
    }
    AppendMenu(hMenu, MF_STRING, 3, L"关于");
    AppendMenu(hMenu, MF_STRING, 1, L"退出");
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, NULL, hWnd, NULL);
    DestroyMenu(hMenu);
}

// 窗口过程函数  
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        // 创建窗口时设置定时器  
        SetTimer(hWnd, TIMER_ID, Q_TIME_DUR, NULL);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_TRAYEVENT:
       if (lParam == WM_RBUTTONDOWN) {
            // 处理鼠标事件
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
            //菜单ID
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
                MessageBox(NULL, L"QQQ连续输出工具，快捷启用关闭:Ctrl+Shift+Q", L"提示", MB_ICONINFORMATION | MB_OK);
                break;
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 初始化托盘图标  
void InitTrayIcon(HINSTANCE hInstance) {
    //加载图标
    hIcon0 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
    hIcon1 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    
    // 初始化托盘图标数据  
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

    Shell_NotifyIcon(NIM_ADD, &nid); // 添加托盘图标  
}

// 清理托盘图标  
void CleanupTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid); // 删除托盘图标  
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
        MessageBox(NULL, L"窗口注册失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    hWnd = CreateWindowEx(0, g_szClassName, L"", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        MessageBox(NULL, L"窗口创建失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    InitTrayIcon(hInstance);
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (hHook == NULL) {
        MessageBox(NULL, L"安装钩子失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    CleanupTrayIcon();
    return Msg.lParam;
}