// unity crash log scanner
// by deuce
// v0.3

#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>

#define MAX_PATH_LEN 2048

typedef struct 
{
    char* keyword;
    char* explanation;
} CrashHint;

CrashHint hints[] = 
{
    {"d3d11", "Graphics: The DirectX 11 driver crashed. Update GPU drivers or check for overheating."},
    {"OutOfMemory", "Memory: The game ran out of RAM. Close background apps or check for a memory leak."},
    {"NullReferenceException", "Scripting: A C# object was 'null' when the game tried to use it. This is a developer bug."},
    {"Access Violation", "Hardware/Protection: The game tried to read protected memory. Check Antivirus or Overlays (Discord/Steam)."},
    {"Write to location 00000000", "Null Pointer: The engine tried to write to memory address zero. This is a severe native bug."},
    {"Read from location FFFFFFFF", "Memory Corruption: The engine is trying to read garbage data."},
    {"gameoverlayrenderer64", "Overlay Conflict: Steam Overlay detected. Try disabling Steam Overlay for this game."},
    {"nvspcap64", "Overlay Conflict: NVIDIA ShadowPlay/Share detected. Try disabling NVIDIA Overlay."},
    {"DLSSUnityPlugin", "Plugin Conflict: NVIDIA DLSS initialization failed. Try disabling DLSS in game settings."},
    {"FSR4UnityPlugin", "Plugin Conflict: AMD FSR initialization failed."},
    {"libxess", "Plugin Conflict: Intel XeSS initialization failed."},
    {"AMDUnityPlugin", "Driver Hook: AMD-specific Unity tools detected. Possible conflict with NVIDIA hardware or vice-versa."},
    {"SymGetSymFromAddr64", "Memory Protection: The crash handler couldn't read the error because another program (Antivirus or Overlay) is protecting the process memory."}
};

void PerformDeepAnalysis(const char* logPath, DWORD exitCode) 
{
    printf("\n--- DEEP ANALYSIS REPORT ---\n");
    
    if (exitCode == 0xC0000005) 
    {
        printf("[!] OS ANALYSIS: Access Violation (Memory Error). Usually caused by a conflict.\n");
    } else if (exitCode == 0xCFFFFFFF) 
    {
        printf("[!] OS ANALYSIS: Unity Hard-Intercept. The engine crashed itself to prevent corruption.\n");
    }

    FILE* file = fopen(logPath, "r");
    if (!file) 
    {
        printf("Error: Could not open log for analysis.\n");
        return;
    }

    char line[1024];
    int foundHint = 0;
    
    int hintTriggered[100] = 
    {0}; 

    while (fgets(line, sizeof(line), file)) 
    {
        for (int i = 0; i < sizeof(hints) / sizeof(CrashHint); i++) 
        {
            if (strstr(line, hints[i].keyword) && !hintTriggered[i]) 
            {
                printf("[!] LOG ANALYSIS: %s\n", hints[i].explanation);
                hintTriggered[i] = 1; 
                foundHint = 1;
            }
        }
    }
    fclose(file);

    if (!foundHint) 
    {
        printf("[?] No common patterns found in logs.\n");
    }
}

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

int main() 
{
    const char* gameExe = "Skate Style.exe";
    char logPath[MAX_PATH_LEN];
    char appDataPath[MAX_PATH_LEN];

    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath);
    snprintf(logPath, sizeof(logPath), "%sLow\\Zellah Games\\Skate Style\\Player.log", appDataPath);

    printf("Waiting for %s...\n", gameExe);
    DWORD pid = 0;
    while ((pid = GetPIDByName(gameExe)) == 0) Sleep(1000);

    printf("Monitoring PID: %lu\n", pid);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid);
    DWORD exitCode = 0;

    while (1) 
    {
        if (GetExitCodeProcess(hProcess, &exitCode) && exitCode != STILL_ACTIVE) 
        {
            PerformDeepAnalysis(logPath, exitCode);
            break;
        }
        Sleep(1000);
    }

    CloseHandle(hProcess);
    printf("\nAnalysis Complete. Press Enter to exit.");
    getchar();
    return 0;
}