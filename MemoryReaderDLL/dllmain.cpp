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
#include <cstdlib>   
#include <windows.h>
#include <cmath>
#include <psapi.h>
#include <d3dx9.h>
#include "smart_calculator.h" 
#include <vector>             
#include <sstream>            
#include <algorithm>         
#include "MinHook.h"


bool ShowMenu = false;
bool ImGui_Initialised = false;

namespace Process { 
    DWORD ID; HANDLE Handle; HWND Hwnd; HMODULE Module;
    WNDPROC WndProc; int WindowWidth; int WindowHeight;
    std::wstring Title; std::wstring ClassName; std::wstring Path;
}

DWORD WINAPI MemoryUpdateThread(LPVOID lpParameter) { 
    while (true) { GameData::Instance().Update(); Sleep(100); } return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_Initialised && ShowMenu) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
            return true;
        }
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) { 
            switch (uMsg) {
                // Mouse
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:  case WM_MOUSEHWHEEL:
                // Teclado
            case WM_KEYDOWN:     case WM_KEYUP:
            case WM_SYSKEYDOWN:  case WM_SYSKEYUP:
            case WM_CHAR:
                if ((io.WantCaptureMouse && (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)) ||
                    (io.WantCaptureKeyboard && (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST))) {
                    return true; // Consumido
                }
                break; 
            }
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
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = oReset(pDevice, pPresentationParameters);
    if (SUCCEEDED(hr)) ImGui_ImplDX9_CreateDeviceObjects();
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


bool IsImGuiTyping() {
    if (!ImGui_Initialised) return false;
    return ImGui::IsAnyItemActive() || ImGui::GetIO().WantTextInput;
}


void ParseWindString(const std::string& wind_str_in, float game_data_angle_fallback, float& strength_out, float& angle_deg_out) {
    strength_out = 0.0f;
    angle_deg_out = 0.0f;
    if (wind_str_in.empty() || wind_str_in == "CALM" || wind_str_in == "0m" || wind_str_in == "0.0m") return;
    std::string str_lower = wind_str_in;
    std::transform(str_lower.begin(), str_lower.end(), str_lower.begin(), ::tolower);
    size_t m_pos = str_lower.find('m');
    if (m_pos != std::string::npos) {
        try { strength_out = std::stof(wind_str_in.substr(0, m_pos)); }
        catch (...) { strength_out = 0.0f; }
        size_t bracket_open_pos = wind_str_in.find('(', m_pos);
        size_t bracket_close_pos = wind_str_in.find(')', bracket_open_pos);
        if (bracket_open_pos != std::string::npos && bracket_close_pos != std::string::npos && bracket_open_pos < bracket_close_pos) {
            try { angle_deg_out = std::stof(wind_str_in.substr(bracket_open_pos + 1, bracket_close_pos - bracket_open_pos - 1)); }
            catch (...) { angle_deg_out = game_data_angle_fallback; }
        }
        else { angle_deg_out = game_data_angle_fallback; }
    }
    else {
        try { strength_out = std::stof(wind_str_in); angle_deg_out = game_data_angle_fallback; }
        catch (...) { strength_out = 0.0f; angle_deg_out = 0.0f; }
    }
}
float ParseStringToFloat(const std::string& str_in, float default_val = 0.0f) {
    if (str_in.empty()) return default_val;
    try { return std::stof(str_in); }
    catch (...) { return default_val; }
}


HRESULT APIENTRY GHEndScene(IDirect3DDevice9* pDevice) {
    static Club::ExtraPower static_power_options_gui = { 0, 0, 0, 0, 0, 0 };
    static int static_player_pwr_slot_gui = 30;
    static std::vector<const char*> static_club_combo_items_calc;
    static int static_current_club_idx_calc = 0;
    static SHOT_TYPE static_selected_shot_type_gui = SHOT_TYPE::DUNK;
    static POWER_SHOT_FACTORY static_selected_ps_type_gui = POWER_SHOT_FACTORY::NO_POWER_SHOT;
    static float static_distance_calc_gui = 200.0f;
    static float static_height_calc_gui = 0.0f;
    static float static_wind_strength_calc_gui = 0.0f;
    static float static_wind_angle_calc_gui = 0.0f;
    static float static_ground_calc_gui = 100.0f;
    static float static_spin_calc_gui = 0.0f;
    static float static_curve_calc_gui = 0.0f;
    static float static_slope_break_calc_gui = 0.0f;
    static char static_result_text_buf[1024] = "Synchronize or fill values and click Calculate.";
    static bool static_first_sync_calc = true;

    // --------------------------------------------------------------------

    if (!ImGui_Initialised) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui_ImplWin32_Init(Process::Hwnd);
        ImGui_ImplDX9_Init(pDevice);
        ImGui_ImplDX9_CreateDeviceObjects();
        Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        initialize_enum_maps();

        if (static_club_combo_items_calc.empty() && !CLUB_INFO_ENUM_NAMES.empty()) {
            for (const std::string& name : CLUB_INFO_ENUM_NAMES) {
                static_club_combo_items_calc.push_back(name.c_str());
            }
        }
        ImGui_Initialised = true;
    }


    if (!IsImGuiTyping()) {
        if (GetAsyncKeyState(VK_F10) & 1) {
            ShowMenu = !ShowMenu;
        }
    }

    static bool ShowAngleTracker = false;
    static bool ShowLineWindow = false;
    static bool showDebugWindow = false;
    static bool showCalculadora = false;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = ShowMenu;

    static bool game_cursor_hidden = false;
    if (ShowMenu && !game_cursor_hidden) {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }
    else if (!ShowMenu && game_cursor_hidden) {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }

    if (ShowMenu) {
        ImGui::SetNextWindowSize(ImVec2(370, 220), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Gh0st Viewer"); // Main window title
        ImGui::Columns(2, nullptr, true);

        // Left Column: Information
        ImGui::Text("Distance: %.2f", GameData::Instance().distance);
        ImGui::Text("Height: %.2f", GameData::Instance().height);
        ImGui::Text("Wind: %s", GameData::Instance().wind.c_str());
        ImGui::Text("Angle: %.2f", GameData::Instance().angle);
        ImGui::Text("Break: %.2f", GameData::Instance().ballBreak);
        ImGui::Text("Terrain: %.d%", GameData::Instance().terrainPercent);
        ImGui::Text("Spin: %s", GameData::Instance().spinStr.c_str());
        ImGui::Text("Curve: %s", GameData::Instance().curveStr.c_str());
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PB: %.2f", GameData::Instance().pb);
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Map Status: %d", GameData::Instance().mapStatus);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Driver Mem: %d", GameData::Instance().driverMem);

        ImGui::NextColumn();

        if (ImGui::TreeNode("Features")) {
            ImGui::Checkbox("Angle Tracker", &ShowAngleTracker); 
            ImGui::Checkbox("Horizontal Line", &ShowLineWindow);
            ImGui::Checkbox("Debug Info", &showDebugWindow);
            ImGui::TreePop();
        }

        if (ImGui::Button("Open Calculator")) {
            showCalculadora = !showCalculadora;
        }

        ImGui::Columns(1);
        ImGui::End(); 
    }

    if (ShowAngleTracker) {
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Angle Tracker", &ShowAngleTracker);
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(); ImVec2 canvas_size = ImVec2(160, 160);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(canvas_p0.x + canvas_size.x / 2, canvas_p0.y + canvas_size.y / 2);
        float circleRadius = 60.0f; ImVec2 mousePos = ImGui::GetMousePos();
        float dx = mousePos.x - center.x; float dy = mousePos.y - center.y;
        float angle_tracker_val = fmod(450.0f + (atan2(dy, -dx) * (180.0f / M_PI)), 360.0f);
        draw_list->AddCircle(center, circleRadius, IM_COL32(255, 255, 255, 255), 64, 2.0f);
        draw_list->AddLine(center, mousePos, IM_COL32(255, 0, 0, 255), 0.6f);
        ImGui::Text("Angle: %.2f", angle_tracker_val); 
        ImGui::End();
    }

    if (ShowLineWindow) {
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Horizontal Line", &ShowLineWindow);
        ImVec2 pos = ImGui::GetCursorScreenPos(); float lineLength = 576.0f; float lineThickness = 2.0f;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 lineStart = pos; ImVec2 lineEnd = ImVec2(pos.x + lineLength, pos.y);
        draw_list->AddLine(lineStart, lineEnd, IM_COL32(255, 255, 255, 255), lineThickness);
        int numRedTicks = 11; float redTickLength = 10.0f; float redSpacing = lineLength / (numRedTicks - 1);
        for (int i = 0; i < numRedTicks; i++) {
            float x = pos.x + i * redSpacing;
            ImVec2 tickStart = ImVec2(x, pos.y); ImVec2 tickEnd = ImVec2(x, pos.y + redTickLength);
            draw_list->AddLine(tickStart, tickEnd, IM_COL32(255, 0, 0, 255), 1.0f);
        }
        int numYellowTicks = numRedTicks - 1; float yellowTickLength = 5.0f;
        for (int i = 0; i < numYellowTicks; i++) {
            float x = pos.x + (i + 0.5f) * redSpacing;
            ImVec2 tickStart = ImVec2(x, pos.y); ImVec2 tickEnd = ImVec2(x, pos.y + yellowTickLength);
            draw_list->AddLine(tickStart, tickEnd, IM_COL32(255, 255, 0, 255), 1.0f);
        }
        ImGui::End();
    }

    if (showDebugWindow) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Debug Info", &showDebugWindow, ImGuiWindowFlags_AlwaysAutoResize);    
        ImGuiIO& io = ImGui::GetIO(); ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
        ImGui::Text("Window (Total): %dx%d", Process::WindowWidth, Process::WindowHeight);
        RECT clientRect = { 0 };
        if (GetClientRect(Process::Hwnd, &clientRect)) {
            int clientWidth = clientRect.right - clientRect.left; int clientHeight = clientRect.bottom - clientRect.top;
            ImGui::Text("Client Area: %dx%d", clientWidth, clientHeight);
        }
        PROCESS_MEMORY_COUNTERS memCounter = { 0 };
        if (GetProcessMemoryInfo(Process::Handle, &memCounter, sizeof(memCounter))) {
            float memoryUsageMB = memCounter.WorkingSetSize / (1024.0f * 1024.0f);
            ImGui::Text("Memory Usage: %.2f MB", memoryUsageMB);
        }
        ImGui::End();
    }

    // --- Render Calculator Window ---
    if (showCalculadora) {
        ImGui::SetNextWindowSize(ImVec2(520, 700), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Smart Calculator##CalcWindow", &showCalculadora);

        // Sync Button
        if (ImGui::Button("Synchronize with GameData##SyncCalcBtn") || static_first_sync_calc) {
            static_distance_calc_gui = GameData::Instance().distance;
            static_height_calc_gui = GameData::Instance().height;

            float parsed_strength = 0.0f;
            float dummy_angle_from_string = 0.0f;
            ParseWindString(GameData::Instance().wind, 0.0f, parsed_strength, dummy_angle_from_string);
            static_wind_strength_calc_gui = parsed_strength;

            static_wind_angle_calc_gui = GameData::Instance().angle;


            static_ground_calc_gui = static_cast<float>(GameData::Instance().terrainPercent);
            if (static_ground_calc_gui == 0.0f) static_ground_calc_gui = 100.0f;
            static_spin_calc_gui = ParseStringToFloat(GameData::Instance().spinStr);
            static_curve_calc_gui = ParseStringToFloat(GameData::Instance().curveStr);
            static_slope_break_calc_gui = GameData::Instance().ballBreak;
            static_first_sync_calc = false;
        }
        ImGui::SameLine(); ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Loads current game values into the fields below.");
            ImGui::Text("Game Data (for reference):");
            ImGui::Text("  Dist: %.1fy, H: %.1fy, Wind: %s", GameData::Instance().distance, GameData::Instance().height, GameData::Instance().wind.c_str());
            ImGui::Text("  Angle (direct): %.2f", GameData::Instance().angle);
            ImGui::Text("  Break: %.2f, Terrain: %d%%", GameData::Instance().ballBreak, GameData::Instance().terrainPercent);
            ImGui::Text("  Spin: %s, Curve: %s", GameData::Instance().spinStr.c_str(), GameData::Instance().curveStr.c_str());
            ImGui::EndTooltip();
        }
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Power Options##CalcPowerOpts", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputDouble("Auxpart##Calc", &static_power_options_gui.auxpart, 1.0, 1.0, "%.0f");
            ImGui::InputDouble("Mascot##Calc", &static_power_options_gui.mascot, 1.0, 1.0, "%.0f");
            ImGui::InputDouble("Card##Calc", &static_power_options_gui.card, 1.0, 1.0, "%.0f");
            ImGui::InputDouble("Card PS##Calc", &static_power_options_gui.ps_card, 1.0, 1.0, "%.0f");
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::InputInt("Player Power Stat##Calc", &static_player_pwr_slot_gui);
            ImGui::PopStyleColor();
        }

        if (ImGui::CollapsingHeader("Shot Setup##CalcShotSetup", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Club:"); ImGui::SameLine(); ImGui::SetNextItemWidth(120);
            if (!static_club_combo_items_calc.empty()) {
                ImGui::Combo("##ClubSelectCalc", &static_current_club_idx_calc, static_club_combo_items_calc.data(), static_cast<int>(static_club_combo_items_calc.size()));
            }
            else { ImGui::Text("Error: Club list empty."); }

            const char* shot_type_display_names[] = { "Dunk", "Tomahawk", "Spike", "Cobra" };
            SHOT_TYPE shot_type_enum_values[] = { SHOT_TYPE::DUNK, SHOT_TYPE::TOMAHAWK, SHOT_TYPE::SPIKE, SHOT_TYPE::COBRA };
            ImGui::Text("Shot Type:");
            for (int i = 0; i < IM_ARRAYSIZE(shot_type_display_names); ++i) {
                if (i > 0) ImGui::SameLine();
                bool is_selected = (static_selected_shot_type_gui == shot_type_enum_values[i]);
                if (is_selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                if (ImGui::Button(shot_type_display_names[i])) static_selected_shot_type_gui = shot_type_enum_values[i];
                if (is_selected) ImGui::PopStyleColor();
            }

            const char* ps_type_display_names[] = { "No PS", "1 PS", "2 PS", "Item PS" };
            POWER_SHOT_FACTORY ps_type_enum_values[] = { POWER_SHOT_FACTORY::NO_POWER_SHOT, POWER_SHOT_FACTORY::ONE_POWER_SHOT, POWER_SHOT_FACTORY::TWO_POWER_SHOT, POWER_SHOT_FACTORY::ITEM_15_POWER_SHOT };
            ImGui::Text("Power Shot:");
            for (int i = 0; i < IM_ARRAYSIZE(ps_type_display_names); ++i) {
                if (i > 0) ImGui::SameLine();
                bool is_selected = (static_selected_ps_type_gui == ps_type_enum_values[i]);
                if (is_selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                if (ImGui::Button(ps_type_display_names[i])) static_selected_ps_type_gui = ps_type_enum_values[i];
                if (is_selected) ImGui::PopStyleColor();
            }
        }

        if (ImGui::CollapsingHeader("Shot Conditions##CalcConditions", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputFloat("Distance (y)##CalcDist", &static_distance_calc_gui, 1.0f, 10.0f, "%.1f");
            ImGui::InputFloat("Height (m)##CalcHeight", &static_height_calc_gui, 0.1f, 1.0f, "%.1f");
            ImGui::InputFloat("Wind Strength (m/s)##CalcWindS", &static_wind_strength_calc_gui, 1.0f, 10.0f, "%.0f");
            ImGui::InputFloat("Wind Angle (deg)##CalcWindA", &static_wind_angle_calc_gui, 1.0f, 10.0f, "%.2f");
            ImGui::InputFloat("Slope Break##CalcSlope", &static_slope_break_calc_gui, 0.01f, 0.1f, "%.2f");
            ImGui::InputFloat("Terrain (%)##CalcGround", &static_ground_calc_gui, 1.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("Spin##CalcSpin", &static_spin_calc_gui, 0.0f, 30.0f, "%.1f");
            ImGui::SliderFloat("Curve##CalcCurve", &static_curve_calc_gui, 0.0f, 30.0f, "%.1f");
        }
        ImGui::Separator();

        if (ImGui::Button("Calculate Shot##CalcExecute", ImVec2(-1, 0))) {
            if (static_club_combo_items_calc.empty() || static_current_club_idx_calc >= static_club_combo_items_calc.size()) {
                snprintf(static_result_text_buf, sizeof(static_result_text_buf), "Error: Club not selected or list empty.");
            }
            else {
                ClubInfo selected_club_info = getClubInfoFromString(static_club_combo_items_calc[static_current_club_idx_calc]);
                SlopeInputType slope_break_input_val = static_cast<double>(static_slope_break_calc_gui);
                double current_ground_val = static_cast<double>(static_ground_calc_gui);
                if (current_ground_val == 0.0) current_ground_val = 100.0;
                FindPowerResult f_initial = find_power( 
                    static_power_options_gui, static_player_pwr_slot_gui, selected_club_info,
                    static_selected_shot_type_gui, static_selected_ps_type_gui,
                    static_cast<double>(static_distance_calc_gui), static_cast<double>(static_height_calc_gui),
                    static_cast<double>(static_wind_strength_calc_gui), static_cast<double>(static_wind_angle_calc_gui), 
                    current_ground_val, static_cast<double>(static_spin_calc_gui), static_cast<double>(static_curve_calc_gui),
                    slope_break_input_val, std::nullopt, std::nullopt);
                std::vector<FindPowerResult> f_results;
                f_results.push_back(f_initial);
                int index_f = 0;
                if (f_results[0].found) {
                    do {
                        index_f++;
                        FindPowerResult next_f = find_power( 
                            static_power_options_gui, static_player_pwr_slot_gui, selected_club_info,
                            static_selected_shot_type_gui, static_selected_ps_type_gui,
                            static_cast<double>(static_distance_calc_gui), static_cast<double>(static_height_calc_gui),
                            static_cast<double>(static_wind_strength_calc_gui), static_cast<double>(static_wind_angle_calc_gui),
                            current_ground_val, static_cast<double>(static_spin_calc_gui), static_cast<double>(static_curve_calc_gui),
                            slope_break_input_val,
                            std::atan2(f_results[index_f - 1].desvio * 1.5, static_cast<double>(static_distance_calc_gui)),
                            f_results[index_f - 1].power);
                        f_results.push_back(next_f);
                    } while (index_f < 10 && f_results[index_f].found && f_results[index_f - 1].found &&
                        std::abs(f_results[index_f - 1].desvio - f_results[index_f].desvio) >= 0.0001);
                }
                if (f_results[index_f].found) { 
                    FindPowerResult final_result = f_results[index_f];
                    double power_percent = final_result.power * 100.0;
                    double power_yards = final_result.power_range * final_result.power;
                    double desvio_pb_real = desvioByDegree(final_result.desvio, static_cast<double>(static_distance_calc_gui)) / 0.2167;
                    double desvio_pb_raw = (final_result.desvio / 0.2167 * -1.0);
                    std::string smart_desvio_str = smartDesvio(final_result.smartData, 800.0, 600.0, true, 0.0, static_power_options_gui, static_player_pwr_slot_gui);
                    snprintf(static_result_text_buf, sizeof(static_result_text_buf),
                        "%.1f%%, %.1fy, %.2fpb Real (%.2fpb Raw), Smart (%s)",
                        power_percent, power_yards, desvio_pb_real, desvio_pb_raw, smart_desvio_str.c_str());
                }
                else {
                    snprintf(static_result_text_buf, sizeof(static_result_text_buf), "Failed to calculate shot.");
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("Result:");
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(static_result_text_buf);
        ImGui::PopTextWrapPos();

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
            Process::ID = GetCurrentProcessId(); Process::Handle = GetCurrentProcess();
            Process::Hwnd = GetForegroundWindow(); RECT TempRect; GetWindowRect(Process::Hwnd, &TempRect);
            Process::WindowWidth = TempRect.right - TempRect.left; Process::WindowHeight = TempRect.bottom - TempRect.top;
            wchar_t TempTitle[MAX_PATH]; GetWindowTextW(Process::Hwnd, TempTitle, MAX_PATH); Process::Title = TempTitle;
            wchar_t TempClassName[MAX_PATH]; GetClassNameW(Process::Hwnd, TempClassName, MAX_PATH); Process::ClassName = TempClassName;
            wchar_t TempPath[MAX_PATH]; GetModuleFileNameExW(Process::Handle, NULL, TempPath, MAX_PATH); Process::Path = TempPath;
            WindowFocus = true;
        } Sleep(50);
    } Sleep(500);
    bool InitHook = false;
    while (!InitHook) {
        if (DirectX9::Init() == true) {
            CreateHook(42, (void**)&oEndScene, GHEndScene); CreateHook(17, (void**)&oPresent, GHPresent);
            CreateHook(82, (void**)&oDrawIndexedPrimitive, GHDrawIndexedPrimitive); CreateHook(81, (void**)&oDrawPrimitive, GHDrawPrimitive);
            CreateHook(65, (void**)&oSetTexture, GHSetTexture); CreateHook(16, (void**)&oReset, GHReset);
            CreateHook(100, (void**)&oSetStreamSource, GHSetStreamSource); CreateHook(87, (void**)&oSetVertexDeclaration, GHSetVertexDeclaration);
            CreateHook(94, (void**)&oSetVertexShaderConstantF, GHSetVertexShaderConstantF); CreateHook(92, (void**)&oSetVertexShader, GHSetVertexShader);
            CreateHook(107, (void**)&oSetPixelShader, GHSetPixelShader);
            InitHook = true;
        } Sleep(50);
    }
    CreateThread(nullptr, 0, MemoryUpdateThread, nullptr, 0, nullptr); return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) { 
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        if (!LoadLibraryA("MinHook.x86.dll")) { MessageBoxA(NULL, "MinHook Error!", "Error", MB_ICONERROR); return FALSE; }
        Process::Module = hModule;
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        DisableAll();
        if (Process::Hwnd && Process::WndProc) SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Process::WndProc));
        if (ImGui_Initialised) { ImGui_ImplDX9_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext(); }
        break;
    } return TRUE;
}