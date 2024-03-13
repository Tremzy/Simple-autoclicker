#include <iostream>
#include <windows.h>
#include <conio.h>

#define MOUSE1_DELAY 55
#define MOUSE2_DELAY 30
#define KEY_TO_PRESS 0x52 // RBTN
#define KEY_TO_PRESS2 0x58 // XBTN

bool mouse1_enabled = false;
bool mouse2_enabled = false;
bool showcmd = true;
HWND hwnd;
void PressKey(DWORD key1, DWORD key2);

int main(int argc, char* argv[]) {
    MessageBoxW(hwnd, L"R - Left AC\nX - Right AC\nINSERT - Show/hide cmd window\nEND - Quit AC", L"Controlls", MB_OK | MB_ICONINFORMATION);
    while(true) {
        if (GetAsyncKeyState(KEY_TO_PRESS)) {
            mouse1_enabled = true;
            Sleep(150);
            std::cout << "Autoclicker enabled" << std::endl;
        }
        else if (GetAsyncKeyState(VK_END)) {
            system("taskkill /F /IM vnjsoa.exe");
        }
        else if (GetAsyncKeyState(KEY_TO_PRESS2)) {
            mouse2_enabled = true;
            Sleep(150);
            std::cout << "Autoclicker enabled" << std::endl;
        }
        else if (GetAsyncKeyState(VK_INSERT)) {
            if (showcmd) {
                showcmd = false;
                ShowWindow(GetConsoleWindow(), SW_HIDE);
                Sleep(150);
            }
            else {
                showcmd = true;
                ShowWindow(GetConsoleWindow(), SW_SHOW);
                Sleep(150);
            }
        }
        while(mouse1_enabled == true) {
            PressKey(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
            int sleeptime = rand()%((MOUSE1_DELAY+50)-(MOUSE1_DELAY-50) + 1) + MOUSE1_DELAY-50;
            std::cout << sleeptime << " MS delay" << std::endl;
            Sleep(sleeptime);
            if (GetAsyncKeyState(KEY_TO_PRESS)) {
                mouse1_enabled = false;
                Sleep(150);
                std::cout << "Autoclicker disabled" << std::endl;
            }
        }
        while (mouse2_enabled == true) {
            PressKey(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
            int sleeptime = rand()%((MOUSE2_DELAY+20)-(MOUSE2_DELAY-20) + 1) + MOUSE2_DELAY-20;
            std::cout << sleeptime << " MS delay" << std::endl;
            Sleep(sleeptime);
            if (GetAsyncKeyState(KEY_TO_PRESS2)) {
                mouse2_enabled = false;
                Sleep(150);
                std::cout << "Autoclicker disabled" << std::endl;
            }
        }
    }
}

void PressKey(DWORD key1, DWORD key2) {
    mouse_event(key1, 0, 0, 0, 0);
    mouse_event(key2, 0, 0, 0, 0);
}