#include "Menu.h"
#include "DirectX9.h"
#include "MemoryReader.h"
#include "GameData.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <string>
#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <ctime>
#include <windows.h>
#include <cmath>
#include <psapi.h>
#include <d3dx9.h>
#include "LogConsole.h"
#include "GameConsole.h"


bool ShowMenu = false;
bool ImGui_Initialised = false;

namespace Process {
    DWORD ID;
    HANDLE Handle;
    HWND Hwnd;
    HMODULE Module;
    WNDPROC WndProc;       // Armazena o WndProc original
    int WindowWidth;
    int WindowHeight;
    std::wstring Title;
    std::wstring ClassName;   
    std::wstring Path;        
}

DWORD WINAPI MemoryUpdateThread(LPVOID lpParameter) {
    while (true) {
        GameData::Instance().Update();
        Sleep(100); // 0.1 segundo de intervalo
    }
    return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Se o menu estiver ativo, repassa as mensagens para o ImGui.
    if (ShowMenu) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
            return true;
    }
    return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}
*/
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return 0;

    if (ShowMenu && (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_CHAR))
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantTextInput)
        {
            if (wParam == VK_RETURN)
                return 0;
            return 0;
        }
    }

    return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}


HRESULT APIENTRY GHPresent(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) {
    return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT APIENTRY GHDrawIndexedPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
    return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT APIENTRY GHDrawPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
    return oDrawPrimitive(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT APIENTRY GHSetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9* pTexture) {
    return oSetTexture(pDevice, Sampler, pTexture);
}

HRESULT APIENTRY GHReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    // Invalida os objetos do ImGui e recria-os após o reset do dispositivo.
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = oReset(pDevice, pPresentationParameters);
    if (SUCCEEDED(hr)) {
        ImGui_ImplDX9_CreateDeviceObjects();
    }
    return hr;
}

HRESULT APIENTRY GHSetStreamSource(IDirect3DDevice9* pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride) {
    return oSetStreamSource(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

HRESULT APIENTRY GHSetVertexDeclaration(IDirect3DDevice9* pDevice, IDirect3DVertexDeclaration9* pdecl) {
    return oSetVertexDeclaration(pDevice, pdecl);
}

HRESULT APIENTRY GHSetVertexShaderConstantF(IDirect3DDevice9* pDevice, UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
    return oSetVertexShaderConstantF(pDevice, StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY GHSetVertexShader(IDirect3DDevice9* pDevice, IDirect3DVertexShader9* veShader) {
    return oSetVertexShader(pDevice, veShader);
}

HRESULT APIENTRY GHSetPixelShader(IDirect3DDevice9* pDevice, IDirect3DPixelShader9* piShader) {
    return oSetPixelShader(pDevice, piShader);
}

void PressSpace() {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = VK_SPACE;

    // Pressionar a tecla
    SendInput(1, &input, sizeof(INPUT));

    // Soltar a tecla
    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = VK_SPACE;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

bool IsImGuiTyping() {
    return ImGui::IsAnyItemActive() || ImGui::GetIO().WantTextInput;
}

IDirect3DTexture9* g_MyTexture = nullptr;

HRESULT APIENTRY GHEndScene(IDirect3DDevice9* pDevice) {
    if (!ImGui_Initialised) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui_ImplWin32_Init(Process::Hwnd);
        ImGui_ImplDX9_Init(pDevice);
        ImGui_ImplDX9_CreateDeviceObjects();

        Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
        ImGui_Initialised = true;
    }

    if (!IsImGuiTyping()) {
        if (GetAsyncKeyState(VK_F10) & 1) {
            ShowMenu = !ShowMenu;
        }
    }

    // Variáveis que controlam a exibição das janelas extras
    static bool ShowAngleTracker = false;
    static bool ShowLineWindow = false;
    static bool showDebugWindow = false;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::GetIO().MouseDrawCursor = ShowMenu;

    if (ShowMenu) {
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Gh0st Viewer");

        ImGui::Columns(2, nullptr, true);

        // --- Coluna Esquerda: Informações ---
        ImGui::Text("Distance: %.2f", GameData::Instance().distance);
        ImGui::Text("Height: %.2f", GameData::Instance().height);
        ImGui::Text("Wind: %s", GameData::Instance().wind.c_str());
        ImGui::Text("Angle: %.2f", GameData::Instance().angle);
        ImGui::Text("Break: %.2f", GameData::Instance().ballBreak);
        ImGui::Text("Terrain: %d%%", GameData::Instance().terrainPercent);
        ImGui::Text("Spin: %s", GameData::Instance().spinStr.c_str());
        ImGui::Text("Curve: %s", GameData::Instance().curveStr.c_str());
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PB: %.2f", GameData::Instance().pb);
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Map Status: %d", GameData::Instance().mapStatus);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Driver Mem: %d", GameData::Instance().driverMem);

        // Muda para a segunda coluna (lado direito)
        ImGui::NextColumn();

        // --- Coluna Direita: Tree Features ---
        if (ImGui::TreeNode("Features")) {
            ImGui::Checkbox("Angle", &ShowAngleTracker);
            ImGui::Checkbox("Horizontal", &ShowLineWindow);
            ImGui::Checkbox("Debug Info", &showDebugWindow);
            ImGui::TreePop();
        }

        static bool showConsole = false;
        if (ImGui::Button("Abrir Console")) {
            showConsole = !showConsole;
        }
        if (showConsole) {
            GameConsole::Instance().Draw("Console", &showConsole);
        }

        static bool showLogWindow = false;
        if (ImGui::Button("Abrir LOG")) {
            showLogWindow = !showLogWindow;
        }
        if (showLogWindow) {
            LogConsole::Instance().Draw("Console LOG", &showLogWindow);
        }

        ImGui::Columns(1);
        ImGui::End();
    }

    // --- Janela Angle Tracker ---
    if (ShowAngleTracker) {
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Angle Tracker");

        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImVec2(160, 160);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 center = ImVec2(canvas_p0.x + canvas_size.x / 2, canvas_p0.y + canvas_size.y / 2);
        float circleRadius = 60.0f;

        ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - center.x;
        float dy = mousePos.y - center.y;

        // Cálculo do ângulo (ajuste conforme necessário)
        float angle = fmod(450.0f + (atan2(dy, -dx) * (180.0f / M_PI)), 360.0f);

        draw_list->AddCircle(center, circleRadius, IM_COL32(255, 255, 255, 255), 64, 2.0f);
        draw_list->AddLine(center, mousePos, IM_COL32(255, 0, 0, 255), 0.6f);

        ImGui::Text("Angle: %.2f", angle);
        ImGui::End();
    }

    // --- Janela Horizontal Line ---
    if (ShowLineWindow) {
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Horizontal Line");

        ImVec2 pos = ImGui::GetCursorScreenPos();

        float lineLength = 576.0f;
        float lineThickness = 2.0f;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Linha horizontal
        ImVec2 lineStart = pos;
        ImVec2 lineEnd = ImVec2(pos.x + lineLength, pos.y);
        draw_list->AddLine(lineStart, lineEnd, IM_COL32(255, 255, 255, 255), lineThickness);

        // Ticks vermelhos (11 ticks)
        int numRedTicks = 11;
        float redTickLength = 10.0f;
        float redSpacing = lineLength / (numRedTicks - 1);
        for (int i = 0; i < numRedTicks; i++) {
            float x = pos.x + i * redSpacing;
            ImVec2 tickStart = ImVec2(x, pos.y);
            ImVec2 tickEnd = ImVec2(x, pos.y + redTickLength);
            draw_list->AddLine(tickStart, tickEnd, IM_COL32(255, 0, 0, 255), 1.0f);
        }

        // Ticks amarelos (entre os ticks vermelhos)
        int numYellowTicks = numRedTicks - 1;
        float yellowTickLength = 5.0f;
        for (int i = 0; i < numYellowTicks; i++) {
            float x = pos.x + (i + 0.5f) * redSpacing;
            ImVec2 tickStart = ImVec2(x, pos.y);
            ImVec2 tickEnd = ImVec2(x, pos.y + yellowTickLength);
            draw_list->AddLine(tickStart, tickEnd, IM_COL32(255, 255, 0, 255), 1.0f);
        }

        ImGui::End();
    }

    // --- Janela de Depuração (Debug Info) ---
    if (showDebugWindow)
    {
        // Posiciona a janela no canto superior esquerdo (ajuste conforme preferir)
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Debug Info", &showDebugWindow, ImGuiWindowFlags_AlwaysAutoResize);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);

        // Se você está usando GetWindowRect para obter as dimensões, pode estar pegando o tamanho total da janela (incluindo bordas e título)
        ImGui::Text("Window (Total): %dx%d", Process::WindowWidth, Process::WindowHeight);

        // Se desejar obter o tamanho da client area, utilize GetClientRect:
        RECT clientRect = { 0 };
        if (GetClientRect(Process::Hwnd, &clientRect))
        {
            int clientWidth = clientRect.right - clientRect.left;
            int clientHeight = clientRect.bottom - clientRect.top;
            ImGui::Text("Client Area: %dx%d", clientWidth, clientHeight);
        }

        // Obter informações de uso de memória
        PROCESS_MEMORY_COUNTERS memCounter = { 0 };
        if (GetProcessMemoryInfo(Process::Handle, &memCounter, sizeof(memCounter)))
        {
            float memoryUsageMB = memCounter.WorkingSetSize / (1024.0f * 1024.0f);
            ImGui::Text("Memory Usage: %.2f MB", memoryUsageMB);
        }

        // Você pode adicionar outras informações que desejar
        // Exemplo: tempo de execução, número de draw calls, etc.

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
    bool WindowFocus = false;
    while (!WindowFocus) {
        DWORD ForegroundWindowProcessID;
        GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
        if (GetCurrentProcessId() == ForegroundWindowProcessID) {
            Process::ID = GetCurrentProcessId();
            Process::Handle = GetCurrentProcess();
            Process::Hwnd = GetForegroundWindow();

            RECT TempRect;
            GetWindowRect(Process::Hwnd, &TempRect);
            Process::WindowWidth = TempRect.right - TempRect.left;
            Process::WindowHeight = TempRect.bottom - TempRect.top;

            wchar_t TempTitle[MAX_PATH];
            GetWindowTextW(Process::Hwnd, TempTitle, MAX_PATH);
            Process::Title = TempTitle;

            wchar_t TempClassName[MAX_PATH];
            GetClassNameW(Process::Hwnd, TempClassName, MAX_PATH);
            Process::ClassName = TempClassName;

            wchar_t TempPath[MAX_PATH];
            GetModuleFileNameExW(Process::Handle, NULL, TempPath, MAX_PATH);
            Process::Path = TempPath;

            WindowFocus = true;
        }
    }

    bool InitHook = false;
    while (!InitHook) {
        if (DirectX9::Init() == true) {
            CreateHook(42, (void**)&oEndScene, GHEndScene);
            CreateHook(17, (void**)&oPresent, GHPresent);
            CreateHook(82, (void**)&oDrawIndexedPrimitive, GHDrawIndexedPrimitive);
            CreateHook(81, (void**)&oDrawPrimitive, GHDrawPrimitive);
            CreateHook(65, (void**)&oSetTexture, GHSetTexture);
            CreateHook(16, (void**)&oReset, GHReset);
            CreateHook(100, (void**)&oSetStreamSource, GHSetStreamSource);
            CreateHook(87, (void**)&oSetVertexDeclaration, GHSetVertexDeclaration);
            CreateHook(94, (void**)&oSetVertexShaderConstantF, GHSetVertexShaderConstantF);
            CreateHook(92, (void**)&oSetVertexShader, GHSetVertexShader);
            CreateHook(107, (void**)&oSetPixelShader, GHSetPixelShader);
            InitHook = true;
        }
    }

    // Cria a thread para atualizar os valores da memória a cada 1 segundo
    CreateThread(nullptr, 0, MemoryUpdateThread, nullptr, 0, nullptr);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        if (CheckDirectXVersion(DirectXVersion.D3D9) == true) {
            Process::Module = hModule;
            CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
            CreateThread(nullptr, 0, MemoryUpdateThread, nullptr, 0, nullptr);
        }
        break;
    case DLL_PROCESS_DETACH:
        DisableAll();
        if (Process::Hwnd && Process::WndProc) {
            SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Process::WndProc));
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    default:
        break;
    }
    return TRUE;
}