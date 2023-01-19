#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>



extern "C"  __declspec(dllexport) void Share()
    {
        MessageBox(NULL, L"Share function", L"Share", NULL);
    }

BOOL APIENTRY DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    BOOL bSuccess = TRUE;

    //  Perform global initialization.
    switch (nReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDllHandle);
        Share();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH: 
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return bSuccess;

}