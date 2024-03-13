#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include "psrandom.h"
#include <cstdio>
#include <fstream>

LPWSTR GenerateCustomApplicationName();

int main(int argc, char* argv[]) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.lpTitle = GenerateCustomApplicationName();
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        std::cerr << "Error creating pipe" << std::endl;
        return 1;
    }
    LPCTSTR lpApplicationName = L"acprocess.exe";
    if (!CreateProcess(lpApplicationName, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        int answ = MessageBox(NULL, L"acprocess.exe not found, do you want to install it?", L"ErrorNotFound", MB_OKCANCEL | MB_ICONERROR);
        if (answ == IDOK) {
            system("start \"\" \"https://cdn.discordapp.com/attachments/1099243236603723856/1217509958208323634/acprocess.exe?ex=66044982&is=65f1d482&hm=48c1e522e2a051ff9916469bf0a71e2197c90c3dcec077e3c91ddc08efbafe85&\"");
            MessageBox(NULL, L"Put this executable file into the same folder as autoclicker.exe, for it to work correctly", L"Installation", MB_OK | MB_ICONINFORMATION);
        }
    }
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

LPWSTR GenerateCustomApplicationName() {
    char lowerChars[27] = "qwertzuiopasdfghjklyxcvbnm";
    char upperChars[27] = "QWERTZUIOPASDFGHJKLYXCVBNM";
    int titleLength = psr::randomBetween(7, 10);
    LPWSTR returnValue = new wchar_t[titleLength+1];
    for (int i = 0; i < titleLength; i++) {
        char randomChar;
        int randomVal;
        switch (psr::randomBetween(0, 2)) {
            case 0:
                randomVal = psr::randomBetween(0,9);
                returnValue[i] = static_cast<wchar_t>('0' + randomVal);
                break;
            case 1:
                randomChar = lowerChars[psr::randomBetween(0,27)];
                returnValue[i] = static_cast<wchar_t>('0' + randomChar);
                break;
            case 2:
                randomChar = upperChars[psr::randomBetween(0,27)];
                returnValue[i] = static_cast<wchar_t>('0' + randomChar);
                break;
        }
    }
    return returnValue;
}