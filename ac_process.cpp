#include "framework.h"
#include "ac_process.h"
#include <windows.h>
#include "pseudorandom.h"
#include <sstream>
#include <thread>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <mutex>

#define MAX_LOADSTRING 100
#define ENABLEAC_ID 1001
#define CONSOLE_LOG_ID 1002
#define MS_EDIT_ID 1003
#define SAVE_SETTING_ID 1004
#define QUIT_APPLICATION_ID 1005

using namespace psr;

HWND acButton;
HWND acButton2;
HWND acButton3;
HWND acButtonLabel1;
HWND acButton4;
HWND acButton5;
HWND acButton6;

RECT labelRect2;
HWND hWnd;

HWND g_hWnd = NULL;
HHOOK g_hHook = nullptr;
char acKey = 'R';
LPCWSTR labelTextPaint = L"Current key: R";

bool closed = false;
bool acEnabled = false;
bool waitingForKey = false;
std::vector<std::thread> acThreads;
std::mutex acMutex;

int acDelay = 100;

void PressKey(DWORD key1, DWORD key2) {
    mouse_event(key1, 0, 0, 0, 0);
    mouse_event(key2, 0, 0, 0, 0);
}

void LogMessage(const wchar_t* logContent) {
    int textLength = GetWindowTextLength(acButton2);
    SendMessage(acButton2, EM_SETSEL, textLength, textLength);
    SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)logContent);
    SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)L"\r\n----------------- \r\n");
}

void StartAutoClicker(int delay) {
    LogMessage(L"Starting autoclicker...");
    while (true) {
        {
            std::lock_guard<std::mutex> lock(acMutex);
            if (!acEnabled) {
                LogMessage(L"Stopping autoclicker...");
                break;
            }
        }
        PressKey(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
        //between acDelay-20 and acDelay+20
        int sleeptime = rand() % ((acDelay + 20) - (acDelay - 20) + 1) + acDelay - 20;
        Sleep(sleeptime);
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
            KBDLLHOOKSTRUCT* pKeyboardHook = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD vkCode = pKeyboardHook->vkCode;
            if (vkCode == acKey) {
                if (!waitingForKey) {
                    if (!acEnabled) {
                        {
                            std::lock_guard<std::mutex> lock(acMutex);
                            acEnabled = true;
                            Sleep(100);
                        }
                        acThreads.emplace_back(StartAutoClicker, acDelay);
                    }
                    else {
                        {
                            std::lock_guard<std::mutex> lock(acMutex);
                            acEnabled = false;
                            Sleep(100);
                        }
                    }
                }
            }
            else if (vkCode == VK_ESCAPE) {
                if (waitingForKey) {
                    waitingForKey = false;
                    SetWindowText(acButton, L"AC key");
                }
            }
            else {
                if (waitingForKey) {
                    wchar_t charPressed[2];
                    charPressed[0] = static_cast<wchar_t>(vkCode);
                    charPressed[1] = L'\0';
                    const wchar_t* consoleOutput = L"AC key set to: ";
                    std::wstring fullOutput = consoleOutput + std::wstring(charPressed);
                    LogMessage(fullOutput.c_str());
                    acKey = static_cast<char>(vkCode);
                    SetWindowText(acButton, L"AC key");
                    waitingForKey = false;
                    WCHAR newLabelText[50];
                    swprintf_s(newLabelText, L"Current key: %c", acKey);
                    SetWindowText(acButton6, newLabelText);
                }
            }
        }
    }

    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

void InstallKeyboardHook()
{
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
}

void UninstallKeyboardHook()
{
    if (g_hHook != nullptr)
    {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = nullptr;
    }
}

std::string GenerateCustomApplicationName() {
    srand(time(NULL));
    char lowerChars[27] = "qwertzuiopasdfghjklyxcvbnm";
    char upperChars[27] = "QWERTZUIOPASDFGHJKLYXCVBNM";
    std::string appName;
    int titleLength = rand() % 10 + 5;
    for (int i = 0; i < titleLength; i++) {
        char randomChar;
        int randomVal;
        switch (rand() % 3 + 0) {
        case 0:
            randomChar = rand() % 10 + '0';
            std::cout << "digit: " << randomChar << std::endl;
            break;
        case 1:
            randomChar = lowerChars[rand() % 26 + 1];
            std::cout << "lowercase: " << randomChar << std::endl;
            break;
        case 2:
            randomChar = upperChars[rand() % 26 + 1];
            std::cout << "uppercase: " << randomChar << std::endl;
            break;
        }
        appName += randomChar;
    }
    return appName;
}


HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];


ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDC_ACPROCESS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    InstallKeyboardHook();

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ACPROCESS));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UninstallKeyboardHook();

    return (int)msg.wParam;
}




ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ACPROCESS);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    WCHAR szTitle[MAX_LOADSTRING];
    std::string result = GenerateCustomApplicationName();
    std::wstring converter = std::wstring(result.begin(), result.end());
    const wchar_t* appName = converter.c_str();
    wcscpy_s(szTitle, MAX_LOADSTRING, appName);
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 500, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
        HFONT hFont2 = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Calibri");
        int buttonWidth = 100;
        int buttonHeight = 50;
        int buttonX5 = (clientRect.right - clientRect.left - buttonWidth) / 5;
        int buttonY5 = (clientRect.bottom - clientRect.top - buttonHeight) / 5;
        acButton = CreateWindow(L"BUTTON", L"AC key", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, buttonX5, buttonY5, buttonWidth, buttonHeight, hWnd, (HMENU)ENABLEAC_ID, hInst, NULL);

        int buttonWidth2 = 150;
        int buttonHeight2 = 180;
        int buttonX2 = clientRect.right - buttonWidth2 - 10;
        int buttonY2 = clientRect.bottom - buttonHeight2 - 10;
        acButton2 = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL | ES_READONLY, buttonX2, buttonY2, buttonWidth2, buttonHeight2, hWnd, (HMENU)CONSOLE_LOG_ID, hInst, NULL);

        SendMessage(acButton, WM_SETFONT, (WPARAM)hFont, TRUE);


        int buttonWidth3 = 100;
        int buttonHeight3 = 30;
        int buttonX3 = (clientRect.right - clientRect.left - buttonWidth3) / 1.5;
        int buttonY3 = (clientRect.bottom - clientRect.top - buttonHeight3) / 4.5;
        acButton3 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"100", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, buttonX3, buttonY3, buttonWidth3, buttonHeight3, hWnd, (HMENU)MS_EDIT_ID, hInst, NULL);

        int buttonWidth4 = 50;
        int buttonHeight4 = 30;
        int buttonX4 = (clientRect.right - clientRect.left - buttonWidth4) / 1.1;
        int buttonY4 = (clientRect.bottom - clientRect.top - buttonHeight4) / 4.5;
        acButton4 = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, buttonX4, buttonY4, buttonWidth4, buttonHeight4, hWnd, (HMENU)SAVE_SETTING_ID, hInst, NULL);

        SendMessage(acButton4, WM_SETFONT, (WPARAM)hFont, TRUE);

        int buttonWidth6 = 50;
        int buttonHeight6 = 30;
        int buttonX6 = (clientRect.right - clientRect.left - buttonWidth4) / 5;
        int buttonY6 = (clientRect.bottom - clientRect.top - buttonHeight4) / 1.3;
        acButton5 = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Quit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, buttonX6, buttonY6, buttonWidth6, buttonHeight6, hWnd, (HMENU)QUIT_APPLICATION_ID, hInst, NULL);

        int buttonWidth7 = 100;
        int buttonHeight7 = 20;
        int buttonX7 = (clientRect.right - clientRect.left - 100) / 5;
        int buttonY7 = (clientRect.bottom - clientRect.top - buttonHeight4) / 3;
        acButton6 = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", labelTextPaint, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, buttonX7, buttonY7, buttonWidth7, buttonHeight7, hWnd, NULL, hInst, NULL);

        SendMessage(acButton6, WM_SETFONT, (WPARAM)hFont2, TRUE);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ENABLEAC_ID:
            SetWindowText(acButton, L"ESC to cancel");
            waitingForKey = true;
            break;
        case QUIT_APPLICATION_ID:
            PostQuitMessage(0);
            break;
        case SAVE_SETTING_ID:
            int textLength = GetWindowTextLength(acButton3);
            wchar_t* buffer = new wchar_t[textLength + 1];
            GetWindowText(acButton3, buffer, textLength + 1);
            acDelay = _wtoi(buffer);
            delete[] buffer;
            std::wstring logMessage = L"Autoclicker delay has been set to ";
            logMessage += std::to_wstring(acDelay);
            LogMessage(logMessage.c_str());
            break;
        }      
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        RECT labelRect = {
            (clientRect.right - clientRect.left - 100) / 1.5,
            (clientRect.bottom - clientRect.top - 30) / 4.5 - 50,
            (clientRect.right - clientRect.left - 100) / 1.5 + 100,
            (clientRect.bottom - clientRect.top - 30) / 4.5 + 20
        };
        DrawText(hdc, L"Delay (ms):", -1, &labelRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

        EndPaint(hWnd, &ps);
    }
    break;


    case WM_SIZE:
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int buttonWidth = 100;
        int buttonHeight = 50;
        int buttonX5 = (clientRect.right - clientRect.left - buttonWidth) / 5;
        int buttonY5 = (clientRect.bottom - clientRect.top - buttonHeight) / 5;
        MoveWindow(acButton, buttonX5, buttonY5, buttonWidth, buttonHeight, TRUE);

        int buttonWidth2 = 150;
        int buttonHeight2 = 180;
        int buttonX2 = clientRect.right - buttonWidth2 - 10;
        int buttonY2 = clientRect.bottom - buttonHeight2 - 10;
        MoveWindow(acButton2, buttonX2, buttonY2, buttonWidth2, buttonHeight2, TRUE);

        int buttonWidth3 = 100;
        int buttonHeight3 = 30;
        int buttonX3 = (clientRect.right - clientRect.left - buttonWidth) / 1.5;
        int buttonY3 = (clientRect.bottom - clientRect.top - buttonHeight) / 4.5;
        MoveWindow(acButton3, buttonX3, buttonY3, buttonWidth3, buttonHeight3, TRUE);

        int buttonWidth4 = 50;
        int buttonHeight4 = 30;
        int buttonX4 = (clientRect.right - clientRect.left - buttonWidth4) / 1.1;
        int buttonY4 = (clientRect.bottom - clientRect.top - buttonHeight4) / 4.5;
        MoveWindow(acButton4, buttonX4, buttonY4, buttonWidth4, buttonHeight4, TRUE);

        int buttonWidth7 = 50;
        int buttonHeight7 = 30;
        int buttonX7 = (clientRect.right - clientRect.left - buttonWidth4) / 5;
        int buttonY7 = (clientRect.bottom - clientRect.top - buttonHeight4) / 1.3;
        MoveWindow(acButton5, buttonX7, buttonY7, buttonWidth7, buttonHeight7, TRUE);
    }
    break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_END:
            PostQuitMessage(0);
            break;
        case VK_INSERT:
            if (!closed) {
                ShowWindow(hWnd, SW_HIDE);
            }
            else {
                ShowWindow(hWnd, SW_SHOW);
            }
            break;
        }
        break;
    case WM_DESTROY:
        UninstallKeyboardHook();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);

    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



