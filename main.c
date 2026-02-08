// unity crash log scanner
// by deuce
// v0.1

#include <stdio.h>
#include <windows.h>
#include <shlobj.h> 

#define MAX_PATH_LEN 1024

void monitor_logs(const char* logPath) {
    printf("Monitoring: %s\n", logPath);
    printf("Searching for 'Error', 'Exception', and 'Crash'...\n\n");

    HANDLE hFile = CreateFileA(logPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error: Could not open log file. (Code: %lu)\n", GetLastError());
        return;
    }

    SetFilePointer(hFile, 0, NULL, FILE_END);

    char buffer[4096];
    DWORD bytesRead;

    while (1) {
        if (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            if (strstr(buffer, "Exception") || strstr(buffer, "Error") || strstr(buffer, "Crash")) {
                printf("[!] POTENTIAL ISSUE DETECTED:\n%s\n", buffer);
            }
        }
        Sleep(500); 
    }

    CloseHandle(hFile);
}

int main() {
    char logPath[MAX_PATH_LEN];
    char appDataPath[MAX_PATH_LEN];

    // Path to local app data with SHGetFolder
    if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath) != S_OK) {
        printf("Failed to find AppData path.\n");
        return 1;
    }


    snprintf(logPath, MAX_PATH_LEN, "%sLow\\Zellah Games\\Skate Style\\Player.log", appDataPath);

    DWORD dwAttrib = GetFileAttributesA(logPath);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        printf("Log file not found at: %s\n", logPath);
        printf("Please ensure the game has been run at least once.\n");
        return 1;
    }

    monitor_logs(logPath);

    return 0;
}