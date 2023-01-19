#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <winternl.h>
#include <stdio.h>


#define PROC_NAME "notepad.exe"
const char DLL_NAME[] = "C:\\Users\\SC106206\\source\\repos\\SnappHookInC\\x64\\Debug\\Lib.dll";

unsigned long GetTargetProcessIdFromProcname(char* procName);
//unsigned long GetTargetThreadIdFromProcname(char* procName);

unsigned char codeToInject[] =
{
        0x68, 0xAA, 0xAA, 0xAA, 0xAA, 0xC7, 0x44, 0x24, 0x04, 0xAA, 0xAA, 0xAA, 0xAA,
        0x9c, 0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57, 0x41, 0x50, 0x41, 0x51, 0x41,
        0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0xB9,
        0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0x48, 0xB8, 0xCC, 0xCC, 0xCC,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0xD0, 0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D,
        0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5D,
        0x5B, 0x5A, 0x59, 0x58, 0x9D, 0xC3
};

//int main() {
//    return 0;
//}
// construct a wstring from a string

int compareWchar(WCHAR* compare, char* toCompare)
{
    size_t len = strlen(toCompare);
    int cVal = 0;
    for (int i = 0; i < len; i++)
    {
        if (compare[i] == toCompare[i])
        {
            cVal++;
        }
    }
    if (cVal == len)
    {
        return 1;
    }
    return 0;
}

unsigned long GetTargetProcessIdFromProcname(char* procName)
{
    PROCESSENTRY32 pe;
    HANDLE thSnapshot;
    BOOL retval, ProcFound = 0;

    thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (thSnapshot == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Error: unable to create toolhelp snapshot", L"Loader", NULL);
        return 0;
    }

    pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

    while (retval)
    {
        if (compareWchar(pe.szExeFile, procName))
        {
            printf("Found!");
            printf("%d\n", pe.th32ProcessID);
            ProcFound = 1;
            break;
        }

        retval = Process32Next(thSnapshot, &pe);
        pe.dwSize = sizeof(PROCESSENTRY32);
    }

    CloseHandle(thSnapshot);
    return pe.th32ProcessID;
}

int GetLengthOfArrayOfPids()
{
    PROCESSENTRY32 pe;
    HANDLE thSnapshot;
    BOOL retval, ProcFound = 0;
    int index = 0;
    thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (thSnapshot == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Error: unable to create toolhelp snapshot", L"Loader", NULL);
        return 0;
    }

    pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

    while (retval)
    {
        retval = Process32Next(thSnapshot, &pe);
        pe.dwSize = sizeof(PROCESSENTRY32);
        index++;
    }

    CloseHandle(thSnapshot);
    return index;
}

unsigned long GetTargetThreadIdFromProcname(unsigned long pid)
{
    // Get List array of Owner Pids
    const int length = GetLengthOfArrayOfPids();
    unsigned long* threads;
    threads = (DWORD*)malloc(sizeof(DWORD) * length);

    HANDLE thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    printf("Snapshot GetThread - %ld\n", GetLastError());

    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);
    int i = 0;
    BOOL retval = Thread32First(thSnapshot, &te);
    printf("Snapshot GetThread - %ld\n", GetLastError());
    while (retval)
    {
        if (te.th32OwnerProcessID == pid)
        {
            threads[i] = (te.th32ThreadID);
            i++;
        }

        retval = Thread32Next(thSnapshot, &te);
    }

    CloseHandle(thSnapshot);
    printf("Close handle Thread snap - %ld\n", GetLastError());
    

    return threads[0];
}
int main()
{
    void* dllString;
    void* stub;
    DWORD wowID, threadID, stubLen, oldIP, oldprot;
    DWORD64 loadLibAddy;
    HANDLE hProcess, hThread;
    CONTEXT ctx;
    stubLen = sizeof(codeToInject);
    printf("%ld\n", GetLastError());
    HINSTANCE mod = LoadLibrary(L"kernel32.dll");
    loadLibAddy = (DWORD64)GetProcAddress((HMODULE)mod, "LoadLibraryA");
    printf("%ld\n", GetLastError());
    wowID = GetTargetProcessIdFromProcname(PROC_NAME);
    hProcess = OpenProcess((PROCESS_ALL_ACCESS), 0, wowID);

    dllString = VirtualAllocEx(hProcess, NULL, (strlen(DLL_NAME) + 1), MEM_COMMIT, (PAGE_READWRITE ));
    stub = VirtualAllocEx(hProcess, NULL, stubLen, MEM_COMMIT, (PAGE_EXECUTE_READWRITE ));
    WriteProcessMemory(hProcess, dllString, DLL_NAME, strlen(DLL_NAME), NULL);
    printf("After Write - %ld\n", GetLastError());
    threadID = GetTargetThreadIdFromProcname(wowID);
    //SuspendThread(hThread);
    printf("%ld\n", GetLastError());
    
    hThread = OpenThread((THREAD_ALL_ACCESS), 0, threadID);
    printf("Open Thread - %ld\n", GetLastError());
    SuspendThread(hThread);

    
    printf("Open Thread - %ld\n", GetLastError());
    ctx.ContextFlags = CONTEXT_CONTROL;
    GetThreadContext(hThread, &ctx);
    oldIP = (DWORD64)ctx.Rip;
    ctx.Rip = (DWORD64)stub;
    ctx.ContextFlags = CONTEXT_CONTROL;

    DWORD retHigh, retLow;
    /*
     * Insert the addresses into the local copy of the codeToInject before copying it to
     * the remote process
     */
    retHigh = (oldIP >> 32) & 0xFFFFFFFF;
    retLow = oldIP & 0xFFFFFFFF;
    memcpy(codeToInject + 1, &retLow, sizeof(retLow));
    memcpy(codeToInject + 9, &retHigh, sizeof(retHigh));
    memcpy(codeToInject + 39, &dllString, sizeof(dllString));
    memcpy(codeToInject + 49, &loadLibAddy, sizeof(loadLibAddy));
    int result = WriteProcessMemory(hProcess, stub, codeToInject, stubLen, NULL);
    if (!result)
    {
        MessageBoxA(NULL, "Could not write process memory.",
            "My Msg", MB_OK);
    }
    printf("Open Thread - %ld\n", GetLastError());
    result = SetThreadContext(hThread, &ctx);
    if (!result)
    {
        printf("Failed to set Context");
    }
    
    printf("Open Thread - %ld\n", GetLastError());
    DWORD suspend_result = ResumeThread(hThread);
    if (suspend_result == -1)
    {
        MessageBoxA(NULL, "Could not resume thread.",
            "My Msg", MB_OK);
    }
    
    printf("Open Thread - %ld\n", GetLastError());
    Sleep(5000);

    VirtualFreeEx(hProcess, dllString, strlen(DLL_NAME), MEM_DECOMMIT);
    VirtualFreeEx(hProcess, stub, stubLen, MEM_DECOMMIT);
    CloseHandle(hProcess);
    CloseHandle(hThread);

    return 0;
}
