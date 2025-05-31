#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <algorithm>   
#include <cctype>       
#include "GameCalc.h"   
#include <windows.h>    
#include <fstream>      
#include <iomanip>      
#include <chrono>       
#include <ctime>        

// =====================================================================================
// Função de Log em Arquivo Simples
// =====================================================================================
static std::ofstream g_logFile; 
static bool g_logFileInitialized = false;

inline void InitializeFileLog() {
    if (!g_logFileInitialized) {
        g_logFile.open("C:\\gh0st_viewer_mem_log.txt", std::ios_base::app);
        if (!g_logFile.is_open()) {
            g_logFile.open("gh0st_viewer_mem_log.txt", std::ios_base::app);
        }
        if (g_logFile.is_open()) {
            g_logFileInitialized = true;
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            char time_buf[30];
            ctime_s(time_buf, sizeof(time_buf), &now_c);
            time_buf[strlen(time_buf) - 1] = '\0';
            g_logFile << "\n=================================================\n";
            g_logFile << "Log Started: " << time_buf << "\n";
            g_logFile << "=================================================\n" << std::endl;
        }
    }
}

// Função variádica para logar formatado
inline void FileLog(const char* format, ...) {
    if (!g_logFileInitialized || !g_logFile.is_open()) {
        InitializeFileLog(); // Tenta inicializar se ainda não foi
        if (!g_logFile.is_open()) return; // Ainda não conseguiu abrir, não faz nada
    }

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Adicionar timestamp
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm timeinfo;
    localtime_s(&timeinfo, &now_c); // localtime_s para segurança

    char time_buf[100];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    g_logFile << "[" << time_buf << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
        << buffer << std::endl;
}


inline uintptr_t HexStringToUint(const std::string& hexStr) {
    if (hexStr.empty()) return 0;
    try {
        return static_cast<uintptr_t>(std::stoul(hexStr, nullptr, 16));
    }
    catch (const std::exception& e) {
        // LogConsole::Instance().AddLog("Error HexStringToUint: %s for %s", e.what(), hexStr.c_str());
        return 0;
    }
}

inline void ParseAddressChain(const std::string& addrChain, uintptr_t& baseOffset, std::vector<uintptr_t>& offsets) {
    offsets.clear(); baseOffset = 0; if (addrChain.empty()) return;
    std::stringstream ss(addrChain); std::string token; bool first = true;
    while (std::getline(ss, token, ',')) {
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (token.rfind("0x", 0) == 0 || token.rfind("0X", 0) == 0) token = token.substr(2);
        if (token.empty()) continue;
        try {
            if (first) { baseOffset = HexStringToUint(token); first = false; }
            else { offsets.push_back(HexStringToUint(token)); }
        }
        catch (const std::exception& e) {
            
        }
    }
}
static inline bool _SafeReadRawPointer(uintptr_t address, uintptr_t& outValue) {
    __try {
        if (address == 0) { FileLog("_SafeReadRawPointer: Null address provided"); return false; }
        outValue = *(uintptr_t*)address;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        FileLog("_SafeReadRawPointer: SEH Exception at 0x%08X", address);
        return false;
    }
}

template <typename T>
static inline bool _SafeReadRawValue(uintptr_t address, T& outValue) {
    __try {
        if (address == 0) { FileLog("_SafeReadRawValue: Null address provided"); return false; }
        outValue = *(T*)address;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        FileLog("_SafeReadRawValue: SEH Exception at 0x%08X reading typeid_hash %zu", address, typeid(T).hash_code());
        return false;
    }
}

static inline bool _SafeCopyFirstChar(const char* src, char& outChar) {
    if (!src) { FileLog("_SafeCopyFirstChar: Null src pointer"); return false; }
    __try {
        char c = src[0];
        outChar = c;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        FileLog("_SafeCopyFirstChar: SEH Exception reading from 0x%08X", (uintptr_t)src);
        return false;
    }
}


template <typename T>
static inline bool ReadPointerChainAlt_WithFileLogs(uintptr_t moduleBase, uintptr_t baseOffset, const std::vector<uintptr_t>& offsets, T& outValue, uintptr_t* finalAddress = nullptr, bool absolute = false) {
    uintptr_t current_address = absolute ? baseOffset : moduleBase + baseOffset;
    FileLog("[RChain] Start. baseAddr:0x%08X, baseOffset:0x%08X, abs:%d -> startAddr:0x%08X", moduleBase, baseOffset, absolute, current_address);

    if (offsets.empty()) {
        FileLog("[RChain] No offsets. Direct read from 0x%08X", current_address);
        if (finalAddress) *finalAddress = current_address;
        bool success = _SafeReadRawValue<T>(current_address, outValue);
        if (!success) FileLog("[RChain] Direct read FAILED from 0x%08X", current_address);
        return success;
    }

    for (size_t i = 0; i < offsets.size() - 1; i++) {
        uintptr_t temp_val;
        FileLog("[RChain] Loop %d: Reading ptr from 0x%08X", (int)i, current_address);
        if (!_SafeReadRawPointer(current_address, temp_val)) {
            FileLog("[RChain] Loop %d: FAILED Read ptr from 0x%08X", (int)i, current_address);
            return false;
        }
        current_address = temp_val + offsets[i];
        FileLog("[RChain] Loop %d: OK. Deref'd 0x%08X, added 0x%08X -> new_addr 0x%08X", (int)i, temp_val, offsets[i], current_address);
    }

    uintptr_t last_ptr_val;
    FileLog("[RChain] Final deref: Reading ptr from 0x%08X (before last offset)", current_address);
    if (!_SafeReadRawPointer(current_address, last_ptr_val)) {
        FileLog("[RChain] Final deref: FAILED Read ptr from 0x%08X", current_address);
        return false;
    }

    current_address = last_ptr_val + offsets.back();
    FileLog("[RChain] Final deref: OK. Deref'd 0x%08X, added last offset 0x%08X -> final_data_addr 0x%08X", last_ptr_val, offsets.back(), current_address);

    if (finalAddress) *finalAddress = current_address;

    FileLog("[RChain] Reading final value (typeid_hash %zu) from 0x%08X", typeid(T).hash_code(), current_address);
    bool success_final_read = _SafeReadRawValue<T>(current_address, outValue);
    if (!success_final_read) FileLog("[RChain] FAILED Reading final value from 0x%08X", current_address);
    else FileLog("[RChain] Final value read successfully.");
    return success_final_read;
}

#define ReadPointerChainAlt ReadPointerChainAlt_WithFileLogs


class GameData {
public:
    float tee1 = 0.f, tee2 = 0.f, tee3 = 0.f;
    float pin1 = 0.f, pin2 = 0.f, pin3 = 0.f;
    float xAxis = 0.f, yAxis = 0.f;
    float ballCos = 1.f, ballSin = 0.f;
    float characterGrid = 0.f;
    int terrainMem = 0;
    float angleCos = 1.f, angleSin = 0.f;
    float spin = 0.f, spinMax = 0.f;
    float curve = 0.f, curveMax = 0.f;
    std::string wind = "0m";
    unsigned char mapStatus = 0;
    unsigned char driverMem = 0;

    double distance = 0.0;
    double height = 0.0;
    double angle = 0.0;
    double ballBreak = 0.0;
    int terrainPercent = 100;
    std::string spinStr = "0/0";
    std::string curveStr = "0/0";
    double pb = 0.0;

    GameData() {
        InitializeFileLog(); 
        FileLog("GameData instance created.");
    }

    void Update() {
        FileLog("--- GameData::Update Called ---");
        uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");
        if (!moduleBase) {
            FileLog("[GameData] Update: Failed to get module base for ProjectG.exe. Wind and Terrain will use defaults.");
            wind = "0m (NoMod)";
            terrainMem = 0;
            terrainPercent = GameCalc::Terrain(terrainMem);
            return;
        }
        FileLog("[GameData] moduleBase (ProjectG.exe): 0x%08X", moduleBase);

        std::vector<uintptr_t> no_offsets;
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("A47E30"), no_offsets, tee1, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("A47F00"), no_offsets, tee2, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("A47F04"), no_offsets, tee3, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("AFD154"), no_offsets, pin1, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("AFD158"), no_offsets, pin2, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("AFD15C"), no_offsets, pin3, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("B024A0"), no_offsets, ballCos, nullptr, false);
        ReadPointerChainAlt<float>(moduleBase, HexStringToUint("B024A8"), no_offsets, ballSin, nullptr, false);
        ReadPointerChainAlt<unsigned char>(moduleBase, HexStringToUint("A79011"), no_offsets, driverMem, nullptr, false);
        ReadPointerChainAlt<unsigned char>(moduleBase, HexStringToUint("A47E28"), no_offsets, mapStatus, nullptr, false);

        uintptr_t baseOffset_chain;
        std::vector<uintptr_t> offsets_chain;

        FileLog("--- Reading xAxis ---");
        ParseAddressChain("00A73E60,0x34,0x18,0x10,0x30,0x0,0x21C,0x1C", baseOffset_chain, offsets_chain);
        ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, xAxis, nullptr, false);

        FileLog("--- Reading Wind ---");
        ParseAddressChain("00B006E8,8,10,30,0,220,28,0", baseOffset_chain, offsets_chain);
        uintptr_t address_of_wind_string = 0; 

        int dummy_value_to_get_address;
        if (ReadPointerChainAlt<int>(moduleBase, baseOffset_chain, offsets_chain, dummy_value_to_get_address, &address_of_wind_string, false)) {
            FileLog("[GameData] Wind chain resolved. Address of string start: 0x%08X", address_of_wind_string);
            if (address_of_wind_string != 0) {
                char* pWindActualStr = (char*)address_of_wind_string;
                char firstChar = 'X';
                if (_SafeCopyFirstChar(pWindActualStr, firstChar)) {
                    FileLog("[GameData] Wind _SafeCopyFirstChar OK from 0x%08X. firstChar: '%c'", address_of_wind_string, firstChar);
                    if (firstChar != '\0' && isdigit(static_cast<unsigned char>(firstChar))) {
                        wind = std::string(1, firstChar) + "m";
                    }
                    else if (firstChar != '\0' && toupper(static_cast<unsigned char>(firstChar)) == 'C') {
                        wind = "CALM";
                    }
                    else {
                        wind = "0m (UnkChar)";
                        if (firstChar == '\0') wind = "0m (EmptyStr)";
                    }
                }
                else {
                    FileLog("[GameData] Wind _SafeCopyFirstChar FAILED from pWindActualStr: 0x%08X", address_of_wind_string);
                    wind = "0m (1stFail)";
                }
            }
            else {
                FileLog("[GameData] Wind resolved address_of_wind_string is NULL");
                wind = "0m (ResolvedNull)";
            }
        }
        else {
            FileLog("[GameData] Wind ReadPointerChainAlt FAILED to resolve address for wind string");
            wind = "0m (AddrChainFail)";
        }
        FileLog("[GameData] Final Wind Value: %s", wind.c_str());

        // Terreno
        FileLog("--- Reading terrainMem ---");
        ParseAddressChain("00B006E8,8,C,C,30,0,21C,AC", baseOffset_chain, offsets_chain); 
        int tempTerrainMem = terrainMem;
        if (!ReadPointerChainAlt<int>(moduleBase, baseOffset_chain, offsets_chain, tempTerrainMem, nullptr, false)) { 
            FileLog("[GameData] Failed to read terrainMem (base: ProjectG.exe+0x%X). Kept previous value: %d", baseOffset_chain, terrainMem);
        }
        else {
            terrainMem = tempTerrainMem;
            FileLog("[GameData] terrainMem read from memory: %d", terrainMem);
        }

        FileLog("--- Reading characterGrid ---");
        ParseAddressChain("00A73E60,0x34,0xB4,0x28,0x14,0x30,0x0,0x7C", baseOffset_chain, offsets_chain);
        ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, characterGrid, nullptr, false);

        FileLog("--- Reading remaining values ---");
        ParseAddressChain("00A73E60,0x34,0x18,0x10,0x30,0x0,0x234,0xAC", baseOffset_chain, offsets_chain); ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, angleCos, nullptr, false);
        ParseAddressChain("00A73E60,0x34,0x18,0x10,0x30,0x0,0x234,0xB4", baseOffset_chain, offsets_chain); ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, angleSin, nullptr, false);
        ParseAddressChain("00A73E60,0x34,0x18,0x10,0x3C,0x30,0x0,0x1C", baseOffset_chain, offsets_chain); ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, spin, nullptr, false);
        ParseAddressChain("00B006E8,0x1C,0xC,0x28,0x2C,0x30,0x0,0x24", baseOffset_chain, offsets_chain); ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, spinMax, nullptr,false);
        ParseAddressChain("00B006E8,0x8,0xC,0xC,0x40,0x0,0x0,0x18", baseOffset_chain, offsets_chain); ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, curve, nullptr, false);
        ParseAddressChain("00B006E8,0x1C,0xC,0x28,0x2C,0x30,0x0,0x20", baseOffset_chain, offsets_chain); ReadPointerChainAlt<float>(moduleBase, baseOffset_chain, offsets_chain, curveMax, nullptr,false);

        distance = GameCalc::Distance(pin1, tee1, pin3, tee3);
        height = GameCalc::Height(tee2, pin2);
        angle = GameCalc::DegreeAngle(angleSin, angleCos);
        ballBreak = GameCalc::BallBreak(ballSin, ballCos, xAxis, yAxis);
        terrainPercent = GameCalc::Terrain(terrainMem);
        spinStr = GameCalc::Spin(spin, spinMax);
        curveStr = GameCalc::Curve(curve, curveMax);
        pb = GameCalc::PB(pin1, tee1, pin3, tee3, characterGrid);
        FileLog("[GameData] Update complete. Wind: %s, TerrainMem: %d, TerrainPercent: %d", wind.c_str(), terrainMem, terrainPercent);
        FileLog("--- GameData::Update Finished ---");
    }

    static GameData& Instance() {
        static GameData instance; 
        return instance;
    }
};