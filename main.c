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
    {"SymGetSymFromAddr64", "Memory Protection: The crash handler couldn't read the error because another program (Antivirus or Overlay) is protecting the process memory."},
    {"D3D12", "DirectX 12: The game is running in DX12 mode. If it crashes, try adding '-force-d3d11' to launch options."},
    {"Vulkan", "Vulkan API: A crash occurred in the Vulkan renderer. Ensure your GPU drivers support Vulkan 1.3."},
    {"Device Lost", "GPU Reset: Your graphics card 'timed out' (TDR). This is often caused by an unstable overclock or a failing PSU."},
    {"Texture count", "VRAM Limit: The engine has too many textures loaded for your GPU to handle."},
    {"DiscordHook64", "Overlay Conflict: Discord Overlay detected. Try turning off 'Enable in-game overlay' in Discord settings."},
    {"RTSSHooks64", "Overlay Conflict: RivaTuner/MSI Afterburner detected. These often conflict with Unity's frame limiter."},
    {"obs64.dll", "Capture Conflict: OBS Game Capture is hooked. Try 'Window Capture' if the game keeps crashing."},
    {"EasyAntiCheat", "EAC Error: The Anti-Cheat system terminated the game. Check for 'Lighting' software (RGB) that might look like a cheat tool."},
    {"Galaxy64.dll", "GOG Conflict: Issues with the GOG Galaxy overlay."},
    {"bepinex", "Modding: The game is modded (BepInEx). This crash is likely caused by an outdated or incompatible mod."},
    {"UnityPlayer.dll caused an Access Violation", "Native Engine Crash: A core engine file failed. This is often caused by a corrupt game installation (Verify files on Steam)."},
    {"Mono: Error while retrieving stack trace", "C# Runtime Error: The virtual machine died. This is a deep-seated bug in the game code."},
    {"Fatal error in gc", "Garbage Collector: The memory management system failed. This usually happens if the game leaks RAM for a long time."},
    {"allocating aslr", "Memory Security: Windows prevented the game from moving code in memory. Try running the game as Admin."},
    {"UnityChildProcess", "Sub-process Failure: A secondary process (like the crash reporter or a web view) failed."},
    {"NullReferenceException", "Developer Bug: The code tried to use an object that doesn't exist. Nothing the user can do—report this to the dev."},
    {"IndexOutOfRangeException", "Array Error: The game tried to access data that wasn't there. Usually happens in inventory or level-loading."},
    {"Sharing violation", "File Lock: Another program is using a game file (maybe your scanner, or an Antivirus)."},
    {"The file is corrupted", "Data Corruption: A .unity3d or .assets file is broken. You MUST verify game files in Steam/Epic."}
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
    
    int hintTriggered[100] = {0}; 

    while (fgets(line, sizeof(line), file)) 
    {
        for (int i = 0; i < sizeof(hints) / sizeof(CrashHint); i++) 
        {
            if (strstr(line, hints[i].keyword) && !hintTriggered[i]) 
            {
                printf("[!] LOG ANALYSIS: %s -- %s\n", hints[i].keyword, hints[i].explanation);
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