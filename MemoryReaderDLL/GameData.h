#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <algorithm>   
#include <cctype>       
#include "GameCalc.h"   
#include "MemoryReader.h"
#include "LogConsole.h"

uintptr_t HexStringToUint(const std::string& hexStr) {
    return static_cast<uintptr_t>(std::stoul(hexStr, nullptr, 16));
}
#pragma region Read
void ParseAddressChain(const std::string& addrChain, uintptr_t& baseOffset, std::vector<uintptr_t>& offsets) {
    offsets.clear();
    std::stringstream ss(addrChain);
    std::string token;
    bool first = true;
    while (std::getline(ss, token, ',')) {
        // Remove eventuais espaços
        token.erase(std::remove_if(token.begin(), token.end(),
            [](unsigned char c) { return std::isspace(c); }), token.end());
        // Se o token começa com "0x" ou "0X", remove-o
        if (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X")
            token = token.substr(2);
        if (first) {
            baseOffset = HexStringToUint(token);
            first = false;
        }
        else {
            offsets.push_back(HexStringToUint(token));
        }
    }
}

template <typename T>
bool ReadPointerChainAlt(uintptr_t moduleBase, uintptr_t baseOffset, const std::vector<uintptr_t>& offsets, T& outValue, uintptr_t* finalAddress = nullptr, bool absolute = false) {
    __try {
        uintptr_t addr = absolute ? baseOffset : moduleBase + baseOffset;
        //WriteLog("ReadPointerChainAlt: Starting addr = 0x%08X", addr);
        for (size_t i = 0; i < offsets.size() - 1; i++) {
            addr = *(uintptr_t*)addr;
            //WriteLog("ReadPointerChainAlt: After dereference index %d, addr = 0x%08X", (int)i, addr);
            addr += offsets[i];
            //WriteLog("ReadPointerChainAlt: After adding offset 0x%X, addr = 0x%08X", offsets[i], addr);
        }
        addr = *(uintptr_t*)addr;
        //WriteLog("ReadPointerChainAlt: After final dereference, addr = 0x%08X", addr);
        addr += offsets.back();
        // WriteLog("ReadPointerChainAlt: After adding final offset 0x%X, final addr = 0x%08X", offsets.back(), addr);
        if (finalAddress)
            *finalAddress = addr;
        outValue = *(T*)addr;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        //WriteLog("ReadPointerChainAlt: Exception occurred during pointer chain reading.");
        return false;
    }
}

#pragma endregion

#pragma region Write
template <typename T>
bool WritePointerChainAlt(
    uintptr_t moduleBase,
    uintptr_t baseOffset,
    const std::vector<uintptr_t>& offsets,
    T newValue,
    uintptr_t* finalAddress = nullptr,
    bool absolute = false)
{
    __try {
        LogConsole::Instance().AddLog("[DEBUG] Iniciando escrita na cadeia de ponteiros...");

        // Calcula o endereço base: absoluto ou relativo
        uintptr_t addr = absolute ? baseOffset : moduleBase + baseOffset;
        LogConsole::Instance().AddLog("[INFO] Endereco base inicial: 0x%08X", addr);

        // Percorre offsets até o penúltimo
        for (size_t i = 0; i < offsets.size() - 1; i++) {
            if (addr == 0) {
                LogConsole::Instance().AddLog("[ERROR] Endereço nulo encontrado antes do offset %d!", (int)i);
                return false;
            }

            // Desreferencia o ponteiro
            uintptr_t deref = *(uintptr_t*)addr;
            LogConsole::Instance().AddLog("[INFO] Deref [%d]: 0x%08X -> 0x%08X", (int)i, addr, deref);

            if (deref == 0) {
                LogConsole::Instance().AddLog("[ERROR] Ponteiro nulo apos dereferenciar no indice %d!", (int)i);
                return false;
            }

            // Soma o offset
            uintptr_t newAddr = deref + offsets[i];
            //LogConsole::Instance().AddLog("[INFO] Adicionando offset 0x%X -> Novo endereço: 0x%08X", offsets[i], newAddr);

            addr = newAddr;
        }

        // Último desreferenciamento
        if (addr == 0) {
            LogConsole::Instance().AddLog("[ERROR] Endereço nulo antes do ultimo dereferenciamento!");
            return false;
        }

        uintptr_t finalDeref = *(uintptr_t*)addr;
        //LogConsole::Instance().AddLog("[INFO] Ultimo deref: 0x%08X -> 0x%08X", addr, finalDeref);

        if (finalDeref == 0) {
            LogConsole::Instance().AddLog("[ERROR] Ponteiro nulo no ultimo dereferenciamento!");
            return false;
        }

        // Soma o último offset
        finalDeref += offsets.back();
        LogConsole::Instance().AddLog("[INFO] Endereço final para escrita: 0x%08X", finalDeref);

        // Se quiser retornar o endereço final
        if (finalAddress) {
            *finalAddress = finalDeref;
        }

        // Escreve o valor
        *(T*)finalDeref = newValue;
        LogConsole::Instance().AddLog("[SUCCESS] Escrito valor %d no endereço 0x%08X", newValue, finalDeref);

        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        LogConsole::Instance().AddLog("[ERROR] Exceção ocorreu ao tentar escrever na memória!");
        return false;
    }
}


bool WriteMemory(uintptr_t moduleBase, const std::string& addrChain, int newValue, bool absolute = false) {
    uintptr_t baseOffset;
    std::vector<uintptr_t> offsets;
    ParseAddressChain(addrChain, baseOffset, offsets);

    LogConsole::Instance().AddLog("[DEBUG] Tentando escrever na memoria no endereço: %s", addrChain.c_str());
    bool result = WritePointerChainAlt<int>(moduleBase, baseOffset, offsets, newValue, nullptr, absolute);

    if (!result) {
        LogConsole::Instance().AddLog("[ERROR] Falha ao escrever no endereco: %s", addrChain.c_str());
    }
    return result;
}

#pragma endregion

bool autoPlayEnabled = false; // Auto Play começa desativado

class GameData {
public:
    float tee1, tee2, tee3;
    float pin1, pin2, pin3;
    float xAxis, yAxis; 
    float ballCos, ballSin;
    float characterGrid;
    int terrainMem; 
    float angleCos, angleSin;
    float spin, spinMax;
    float curve, curveMax;
    std::string wind;
    unsigned char mapStatus;
    unsigned char driverMem;

    double distance;
    double height;
    double angle;    
    double ballBreak;
    int terrainPercent;
    std::string spinStr;
    std::string curveStr;
    double pb;     
    /*
    int estadoPixel;       // Estado Pixel (INT)
    float hitPosition;     // Hit Position (FLOAT)
    float pangyaPixel;     // Pangya Pixel (FLOAT)

    void CheckAndPressSpace() {
        if (!autoPlayEnabled) return;  // Só executa se o Auto Play estiver ativado

        if (estadoPixel != 1 && estadoPixel != 0) {  // Estado Pixel diferente de 1 e 0
            if (pangyaPixel >= (hitPosition - 1) && pangyaPixel <= (hitPosition + 1)) {  // Pangya Pixel dentro da faixa
                PressSpace();
            }
        }
    }
    

    void PressSpace() {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_SPACE; // Tecla espaço

        // Pressionar a tecla
        SendInput(1, &input, sizeof(INPUT));

        // Soltar a tecla
        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_SPACE;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
    */
    void Update() {
        uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");
        //WriteLog("GameData::Update: Module base = 0x%08X", moduleBase);

        tee1 = *(float*)(moduleBase + HexStringToUint("A47E30"));
        tee2 = *(float*)(moduleBase + HexStringToUint("A47F00"));
        tee3 = *(float*)(moduleBase + HexStringToUint("A47F04"));
        pin1 = *(float*)(moduleBase + HexStringToUint("AFD154"));
        pin2 = *(float*)(moduleBase + HexStringToUint("AFD158"));
        pin3 = *(float*)(moduleBase + HexStringToUint("AFD15C"));
        ballCos = *(float*)(moduleBase + HexStringToUint("B024A0"));
        ballSin = *(float*)(moduleBase + HexStringToUint("B024A8"));
        driverMem = *(unsigned char*)(moduleBase + HexStringToUint("A79011"));
        mapStatus = *(unsigned char*)(moduleBase + HexStringToUint("A47E28"));
        /*
        // Ler Estado Pixel
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x20,0x10,0xB4,0x30,0x8,0x128,0xF8", baseOffset, offsets);
            if (!ReadPointerChainAlt<int>(moduleBase, baseOffset, offsets, estadoPixel, nullptr, false))
                estadoPixel = 0;
        }

        // Ler Hit Position
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x20,0x10,0xB4,0x30,0x8,0x128,0xFC", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, hitPosition, nullptr, false))
                hitPosition = 0.0f;
        }

        // Ler Pangya Pixel
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x20,0x10,0xB4,0x30,0x8,0x128,0xE8", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, pangyaPixel, nullptr, false))
                pangyaPixel = 0.0f;
        }
        */

        // xAxisAddress: "00A73E60,0x34,0x18,0x10,0x30,0x0,0x21C,0x1C"
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00A73E60,0x34,0x18,0x10,0x30,0x0,0x21C,0x1C", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, xAxis, nullptr, false))
                xAxis = 0.0f;
        }

        // Wind (endereço absoluto): "00B006E8,0x8,0x10,0x30,0x0,0x220,0x28,0x0"
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x8,0x10,0x30,0x0,0x220,0x28,0x0", baseOffset, offsets);
            uintptr_t finalAddr = 0;
            char* windStr = nullptr;
            if (ReadPointerChainAlt<char*>(moduleBase, baseOffset, offsets, windStr, &finalAddr, true)) {
                wind = std::string(windStr);
            }
            else {
                wind = "0m";
            }  
        }

        // characterGridAddress: "00A73E60,0x34,0xB4,0x28,0x14,0x30,0x0,0x7C"
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00A73E60,0x34,0xB4,0x28,0x14,0x30,0x0,0x7C", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, characterGrid, nullptr, false))
                characterGrid = 0.0f;
        }
        // terrainAddress: "00B006E8,0x8,0xC,0xC,0x30,0x0,0x21C,0xAC" (absoluto)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x8,0xC,0xC,0x30,0x0,0x21C,0xAC", baseOffset, offsets);
            int terrainVal = 0;
            if (!ReadPointerChainAlt<int>(moduleBase, baseOffset, offsets, terrainVal, nullptr, true))
                terrainVal = 0;
            terrainMem = terrainVal;
        }
        // angleCosAddress: "00A73E60,0x34,0x18,0x10,0x30,0x0,0x234,0xAC" (relativo)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00A73E60,0x34,0x18,0x10,0x30,0x0,0x234,0xAC", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, angleCos, nullptr, false))
                angleCos = 0.0f;
        }
        // angleSinAddress: "00A73E60,0x34,0x18,0x10,0x30,0x0,0x234,0xB4" (relativo)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00A73E60,0x34,0x18,0x10,0x30,0x0,0x234,0xB4", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, angleSin, nullptr, false))
                angleSin = 0.0f;
        }
        // spinAddress: "00A73E60,0x34,0x18,0x10,0x3C,0x30,0x0,0x1C" (relativo)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00A73E60,0x34,0x18,0x10,0x3C,0x30,0x0,0x1C", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, spin, nullptr, false))
                spin = 0.0f;
        }
        // spinMaxAddress: "00B006E8,0x1C,0xC,0x28,0x2C,0x30,0x0,0x24" (absoluto)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x1C,0xC,0x28,0x2C,0x30,0x0,0x24", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, spinMax, nullptr, false))
                spinMax = 0.0f;
        }
        // curveAddress: "00B006E8,0x8,0xC,0xC,0x40,0x0,0x0,0x18" (absoluto)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x8,0xC,0xC,0x40,0x0,0x0,0x18", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, curve, nullptr, false))
                curve = 0.0f;
        }
        // curveMaxAddress: "00B006E8,0x1C,0xC,0x28,0x2C,0x30,0x0,0x20" (absoluto)
        {
            uintptr_t baseOffset;
            std::vector<uintptr_t> offsets;
            ParseAddressChain("00B006E8,0x1C,0xC,0x28,0x2C,0x30,0x0,0x20", baseOffset, offsets);
            if (!ReadPointerChainAlt<float>(moduleBase, baseOffset, offsets, curveMax, nullptr, false))
                curveMax = 0.0f;
        }

        distance = GameCalc::Distance(pin1, tee1, pin3, tee3);
        height = GameCalc::Height(tee2, pin2);
        angle = GameCalc::DegreeAngle(angleSin, angleCos);
        ballBreak = GameCalc::BallBreak(ballSin, ballCos, xAxis, yAxis);
        terrainPercent = GameCalc::Terrain(terrainMem);
        spinStr = GameCalc::Spin(spin, spinMax);
        curveStr = GameCalc::Curve(curve, curveMax);
        pb = GameCalc::PB(pin1, tee1, pin3, tee3, characterGrid);
    }

    // Singleton para facilitar o acesso, se desejado
    static GameData& Instance() {
        static GameData instance;
        return instance;
    }
};
