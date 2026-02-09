// unity crash log scanner
// by deuce
// v0.4

#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <stdio.h>

#define MAX_PATH_LEN 1024
#define LOG_LINES_TO_READ 15

// process id localization logic
DWORD GetPIDByName(const char* processName) 
{
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) 
    {
        do 
        {
            if (_stricmp(entry.szExeFile, processName) == 0) 
            {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return pid;
}

void DumpFinalLogs(const char* logPath) 
{
    printf("\n--- EXTRACTING FINAL LOG ENTRIES ---\n");
    FILE* file = fopen(logPath, "r");
    if (!file) 
    {
        printf("Could not open log for final dump.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    long readPos = fileSize > 2000 ? fileSize - 2000 : 0; // look at last 2kb
    fseek(file, readPos, SEEK_SET);

    char line[512];
    while (fgets(line, sizeof(line), file)) 
    {
        printf("  > %s", line);
    }
    fclose(file);
}

int main() 
{
    const char* gameExe = "Skate Style.exe"; 
    char logPath[MAX_PATH_LEN];
    char appDataPath[MAX_PATH_LEN];

    // automatic LocalLow localization in AppData
    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath);
    snprintf(logPath, MAX_PATH_LEN, "%sLow\\Zellah Games\\Skate Style\\Player.log", appDataPath);

    printf("Waiting for %s to start...\n", gameExe);
    DWORD pid = 0;
    while ((pid = GetPIDByName(gameExe)) == 0) Sleep(1000);

    printf("Target Found (PID: %lu). Monitoring...\n", pid);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid);
    DWORD exitCode = 0;

    while (1) 
    {
        if (GetExitCodeProcess(hProcess, &exitCode) && exitCode != STILL_ACTIVE) 
        {
            printf("\n======================================\n");
            printf("GAME TERMINATED\n");
            printf("Exit Code: 0x%08X\n", exitCode);

            if (exitCode != 0) 
            {
                printf("CRASH DETECTED! Analyzing log...\n");
                DumpFinalLogs(logPath);
            } else 
            {
                printf("Game closed normally.\n");
            }
            break;
        }
        Sleep(1000);
    }

    CloseHandle(hProcess);
    printf("\nPress Enter to exit scanner.");
    getchar();
    return 0;
}