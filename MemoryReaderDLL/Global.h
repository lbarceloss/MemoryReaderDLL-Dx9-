#include <Windows.h>
#include <stdint.h>
#include <assert.h>
#include <Psapi.h>
#include "MinHook.h"

#if defined _M_X64
typedef uint64_t uintx_t;
#elif defined _M_IX86
typedef uint32_t uintx_t;
#endif

static uintx_t* MethodsTable = NULL;

struct _DirectXVersion {
    int Unknown = 0;
    int D3D9 = 1;
    int D3D10 = 2;
    int D3D11 = 3;
    int D3D12 = 4;
} DirectXVersion;

bool CheckDirectXVersion(int _DirectXVersion) {
    if (_DirectXVersion == DirectXVersion.D3D12) {
        if (GetModuleHandle(L"d3d12.dll") != NULL) {
            return true;
        }
    }
    if (_DirectXVersion == DirectXVersion.D3D11) {
        if (GetModuleHandle(L"d3d11.dll") != NULL) {
            return true;
        }
    }
    if (_DirectXVersion == DirectXVersion.D3D10) {
        if (GetModuleHandle(L"d3d10.dll") != NULL) {
            return true;
        }
    }
    if (_DirectXVersion == DirectXVersion.D3D9) {
        if (GetModuleHandle(L"d3d9.dll") != NULL) {
            return true;
        }
    }
    return false;
}

WNDCLASSEX WindowClass;
HWND WindowHwnd;

bool InitWindow() {
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = DefWindowProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = GetModuleHandle(NULL);
    WindowClass.hIcon = NULL;
    WindowClass.hCursor = NULL;
    WindowClass.hbrBackground = NULL;
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = L"GH"; // Deve ser um LPCWSTR
    WindowClass.hIconSm = NULL;

    RegisterClassEx(&WindowClass);

    WindowHwnd = CreateWindow(WindowClass.lpszClassName, L"DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);

    if (WindowHwnd == NULL) {
        return false;
    }
    return true;
}

bool DeleteWindow() {
    DestroyWindow(WindowHwnd);
    UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
    if (WindowHwnd != NULL) {
        return false;
    }
    return true;
}

bool CreateHook(uint16_t Index, void** Original, void* Function) {
    assert(Index >= 0 && Original != NULL && Function != NULL);
    void* target = (void*)MethodsTable[Index];
    if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
        //WriteLog("Failed to create/enable hook at index %d.", Index);
        return false;
    }
    //WriteLog("Hook created at index %d.", Index);
    return true;
}

void DisableHook(uint16_t Index) {
    assert(Index >= 0);
    MH_DisableHook((void*)MethodsTable[Index]);
    //WriteLog("Hook disabled at index %d.", Index);
}

void DisableAll() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    free(MethodsTable);
    MethodsTable = NULL;
    //WriteLog("All hooks disabled and MinHook uninitialized.");
}
