#include "framework.h"
#include "ac_process.h"
#include <windows.h>
#include "pseudorandom.h"
#include <sstream>
#include <thread>
#include <vector>
#include <conio.h>
#include <Lmcons.h>
#include <cstdlib>
#include <mutex>
#include <filesystem>
#include <fstream>

#define MAX_LOADSTRING 100
#define ENABLEAC_ID 1001
#define CONSOLE_LOG_ID 1002
#define MS_EDIT_ID 1003
#define SAVE_SETTING_ID 1004
#define QUIT_APPLICATION_ID 1005
#define SELF_DESTRUCT_ID 1006
#define SAVE_ROFFSET_ID 1007
#define SAVE_CONFIG_SETTINGS 1008
#define LOAD_CONFIG_SETTINGS 1009

using namespace psr;

HWND acButton;
HWND acButton2;
HWND acButton3;
HWND acButtonLabel1;
HWND acButton4;
HWND acButton5;
HWND acButton6;
HWND acButton7;
HWND acButton8;
HWND acButton9;
HWND acButton10;
HWND acButton11;

RECT labelRect2;
HWND hWnd;
HWND g_hWndMain = nullptr;

HWND g_hWnd = NULL;
HHOOK g_hHook = nullptr;
char acKey = 'R';
LPCWSTR labelTextPaint = L"Current key: R";
std::wstring logMessage;
int random_offset = 20;
int textLength3;
wchar_t* buffer2;
int textLength4;

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
    SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)L"\r\n----------------------------- \r\n");
}

void SaveConfig() {
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    GetUserName((TCHAR*)username, &size);
    std::wstring strconvert(&username[0]);
    std::string usern(strconvert.begin(), strconvert.end());

    const std::string filename = "config.json";
    const std::string dirname = "Autoclicker";
    std::filesystem::path fullpath = std::filesystem::path("C:\\Users") / usern / "AppData\\Roaming" / dirname / filename;

    std::ofstream file(fullpath);
    if (file.is_open()) {
        file << "{\n";
        file << "\t\"clickspeed\": " << acDelay << ",\n";
        file << "\t\"randomoffset\": " << random_offset << ",\n";
        file << "\t\"triggerkey\": \"" << acKey << "\"\n";
        file << "}";
        file.close();
        LogMessage(L"Config saved successfully");
    }
    else {
        LogMessage(L"Failed to save the config. Try reloading the application");
    }
}

void ConfigCheck() {
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    GetUserName((TCHAR*)username, &size);
    std::wstring sconv(&username[0]);
    std::string usn(sconv.begin(), sconv.end());

    std::filesystem::path appdata = std::filesystem::path("C:\\Users") / usn / "AppData\\Roaming";
    const std::string filename = "config.json";
    const std::string dirname = "Autoclicker";

    LogMessage(L"Checking config file...");
    bool configfound = false;

    if (std::filesystem::exists(std::filesystem::path(appdata) / dirname / filename)) {
        configfound = true;
        LogMessage(L"Config file found");
    }

    /*for (const auto& entry : std::filesystem::directory_iterator(appdata)) {
        size_t pos = entry.path().string().find(dirname);
        if (pos != std::string::npos) {
            configfound = true;
            break;
        }
    }
    */
    std::wstring cfgwstr = configfound ? L"true" : L"false";
    if (configfound == false) {
        LogMessage(L"Config file not found, creating new config...");
        std::filesystem::path fullDirPath = appdata / dirname;
        bool createDir = std::filesystem::create_directory(fullDirPath);
        std::filesystem::path fullFilePath = fullDirPath / filename;
        std::ofstream file(fullFilePath);
        if (file.is_open()) {
            file << "{\n\t\"clickspeed\": 100, \n\t\"randomoffset\" : 20, \n\t\"triggerkey\" : \"r\"\n}";
            file.close();
            LogMessage(L"Config successfully created.");
        }
        else {
            LogMessage(L"Failed to create config file. Please start the application with administrator privileges");
        }
    }
    else {
        std::filesystem::path fullFilePath = appdata / dirname / filename;
        std::ifstream file(fullFilePath);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find("clickspeed");
                if (pos != std::string::npos) {
                    for (size_t i = 0; i < line.length(); i++) {
                        if (std::isdigit(line[i])) {
                            size_t start = i;
                            while (i < line.length() && std::isdigit(line[i])) {
                                ++i;
                            }
                            std::string numstr = line.substr(start, i - start);
                            int num = std::stoi(numstr);
                            acDelay = num;
                            SendMessage(acButton3, WM_SETTEXT, 0, (LPARAM)std::to_wstring(num).c_str());
                            std::string str1 = "Loaded acdelay: ";
                            std::wstring wstr1;
                            wstr1.assign(str1.begin(), str1.end());

                            std::wstring wstr = std::to_wstring(acDelay);
                            LogMessage((wstr1 + wstr).c_str());
                            continue;
                        }
                    }
                }
                size_t pos2 = line.find("randomoffset");
                if (pos2 != std::string::npos) {
                    for (size_t i = 0; i < line.length(); i++) {
                        if (std::isdigit(line[i])) {
                            size_t start = i;
                            while (i < line.length() && std::isdigit(line[i])) {
                                ++i;
                            }
                            std::string numstr = line.substr(start, i - start);
                            int num = std::stoi(numstr);
                            random_offset = num;
                            SendMessage(acButton8, WM_SETTEXT, 0, (LPARAM)std::to_wstring(num).c_str());
                            std::string str1 = "Loaded randomoffset: ";
                            std::wstring wstr1;
                            wstr1.assign(str1.begin(), str1.end());

                            std::wstring wstr = std::to_wstring(random_offset);
                            LogMessage((wstr1 + wstr).c_str());
                            continue;
                        }
                    }
                }
                size_t pos3 = line.find("triggerkey");
                if (pos3 != std::string::npos) {
                    for (size_t i = 0; i < line.length(); i++) {
                        if (line[i] == ':') {
                            std::string keyline = line.substr(i + 1);
                            for (size_t j = 0; j < keyline.length(); j++) {
                                if (keyline[j] == '"') {
                                    if (j + 1 < keyline.length() && keyline[j + 1] != '"') {
                                        char foundtriggerkey = keyline[j + 1];
                                        acKey = toupper(foundtriggerkey);
                                        std::wstring wstr(1, static_cast<wchar_t>(toupper(foundtriggerkey)));
                                        std::string currentstr = "Current key: ";
                                        std::wstring currentwstr;
                                        currentwstr.assign(currentstr.begin(), currentstr.end());
                                        SendMessage(acButton6, WM_SETTEXT, 0, (LPARAM)(currentwstr + wstr).c_str());

                                        std::string str1 = "Loaded triggerkey: ";
                                        std::wstring wstr1;
                                        wstr1.assign(str1.begin(), str1.end());
                                        
                                        LogMessage((wstr1 + wstr).c_str());
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

void SelfDestruct() {
    TCHAR szFilePath[MAX_PATH];
    GetModuleFileName(NULL, szFilePath, MAX_PATH);

    WCHAR command[MAX_PATH + 4];
    wcscpy_s(command, L"del ");
    wcscat_s(command, MAX_PATH + 4, szFilePath);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
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
        int sleeptime = rand() % ((acDelay + random_offset) - (acDelay - random_offset) + 1) + acDelay - random_offset;
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
            else if (vkCode == VK_INSERT) {
                if (!closed) {
                    ShowWindow(hWnd, SW_HIDE);
                    closed = true;
                    Sleep(50);
                }
                else {
                    ShowWindow(hWnd, SW_SHOW);
                    closed = false;
                    Sleep(50);
                }
            }
            else if (vkCode == VK_END) {
                PostQuitMessage(0);
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
    int titleLength = rand() % (12 - 6 + 1) +6;
    for (int i = 0; i < titleLength; i++) {
        char randomChar;
        int randomVal;
        switch (rand() % 3 + 0) {
        case 0:
            randomChar = rand() % 10 + '0';
            std::cout << "digit: " << randomChar << std::endl;
            break;
        case 1:
            randomChar = lowerChars[rand() % 26];
            std::cout << "lowercase: " << randomChar << std::endl;
            break;
        case 2:
            randomChar = upperChars[rand() % 26];
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
    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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

        int buttonWidth8 = 100;
        int buttonHeight8 = 30;
        int buttonX8 = (clientRect.left + 15);
        int buttonY8 = (clientRect.bottom - clientRect.top - buttonHeight4) - 15;
        acButton7 = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Self-destruct", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, buttonX8, buttonY8, buttonWidth8, buttonHeight8, hWnd, (HMENU)SELF_DESTRUCT_ID, hInst, NULL);

        int buttonWidth9 = 100;
        int buttonHeight9 = 30;
        int buttonX9 = (clientRect.right - clientRect.left - buttonWidth3) / 1.5;
        int buttonY9 = (clientRect.bottom - clientRect.top - buttonHeight3) / 2.5;
        acButton8 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"20", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, buttonX9, buttonY9, buttonWidth9, buttonHeight9, hWnd, NULL, hInst, NULL);

        int buttonWidth10 = 50;
        int buttonHeight10 = 30;
        int buttonX10 = (clientRect.right - clientRect.left - buttonWidth4) / 1.1;
        int buttonY10 = (clientRect.bottom - clientRect.top - buttonHeight4) / 2.5;
        acButton9 = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Set", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, buttonX10, buttonY10, buttonWidth10, buttonHeight10, hWnd, (HMENU)SAVE_ROFFSET_ID, hInst, NULL);

        SendMessage(acButton9, WM_SETFONT, (WPARAM)hFont, TRUE);

        int buttonWidth11 = 80;
        int buttonHeight11 = 30;
        int buttonX11 = (clientRect.right - clientRect.left - buttonWidth11) / 1.6;
        int buttonY11 = (clientRect.bottom - clientRect.top - buttonHeight11) / 2.1;
        acButton10 = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, buttonX11, buttonY11, buttonWidth11, buttonHeight11, hWnd, (HMENU)SAVE_CONFIG_SETTINGS, hInst, NULL);

        SendMessage(acButton10, WM_SETFONT, (WPARAM)hFont, TRUE);

        ConfigCheck();
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
        case SELF_DESTRUCT_ID:
            PostQuitMessage(0);
            SelfDestruct();
            break;
        case SAVE_ROFFSET_ID:
            textLength3 = GetWindowTextLength(acButton8);
            buffer2 = new wchar_t[textLength3 + 1];
            GetWindowText(acButton8, buffer2, textLength3 + 1);
            random_offset = _wtoi(buffer2);
            delete[] buffer2;
            logMessage = L"Random offset has been set to ";
            logMessage += std::to_wstring(random_offset);
            textLength4 = GetWindowTextLength(acButton2);
            SendMessage(acButton2, EM_SETSEL, textLength4, textLength4);
            SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)logMessage.c_str());
            SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)L"\r\n----------------- \r\n");
            break;
        case SAVE_CONFIG_SETTINGS:
            SaveConfig();
            break;
        case SAVE_SETTING_ID:
            int textLength = GetWindowTextLength(acButton3);
            wchar_t* buffer = new wchar_t[textLength + 1];
            GetWindowText(acButton3, buffer, textLength + 1);
            acDelay = _wtoi(buffer);
            delete[] buffer;
            logMessage = L"Autoclicker delay has been set to ";
            logMessage += std::to_wstring(acDelay);
            int textLength2 = GetWindowTextLength(acButton2);
            SendMessage(acButton2, EM_SETSEL, textLength2, textLength2);
            SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)logMessage.c_str());
            SendMessage(acButton2, EM_REPLACESEL, TRUE, (LPARAM)L"\r\n----------------- \r\n");
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

        RECT labelRect2 = {
            (clientRect.right - clientRect.left - 130) / 1.5 + 20,
            (clientRect.bottom - clientRect.top - 30) / 2.5 - 55,
            (clientRect.right - clientRect.left - 130) / 1.5 + 130,
            (clientRect.bottom - clientRect.top - 30) / 2.5
        };
        DrawText(hdc, L"Random offset:", -1, &labelRect2, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

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

        int buttonWidth8 = 100;
        int buttonHeight8 = 20;
        int buttonX8 = (clientRect.right - clientRect.left - 100) / 5;
        int buttonY8 = (clientRect.bottom - clientRect.top - buttonHeight4) / 3;
        MoveWindow(acButton6, buttonX8, buttonY8, buttonWidth8, buttonHeight8, TRUE);

        int buttonWidth9 = 100;
        int buttonHeight9 = 30;
        int buttonX9 = (clientRect.right - clientRect.left - buttonWidth3) / 1.5;
        int buttonY9 = (clientRect.bottom - clientRect.top - buttonHeight3) / 2.8;
        MoveWindow(acButton8, buttonX9, buttonY9, buttonWidth9, buttonHeight9, TRUE);

        int buttonWidth10 = 50;
        int buttonHeight10 = 30;
        int buttonX10 = (clientRect.right - clientRect.left - buttonWidth4) / 1.1;
        int buttonY10 = (clientRect.bottom - clientRect.top - buttonHeight4) / 2.8;
        MoveWindow(acButton9, buttonX10, buttonY10, buttonWidth10, buttonHeight10, TRUE);

        int buttonWidth11 = 100;
        int buttonHeight11 = 30;
        int buttonX11 = (clientRect.left + 15);
        int buttonY11 = (clientRect.bottom - clientRect.top - buttonHeight4) - 15;
        MoveWindow(acButton7, buttonX11, buttonY11, buttonWidth11, buttonHeight11, TRUE);
    }
    break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_END:
            PostQuitMessage(0);
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



