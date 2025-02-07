#pragma once
#include <vector>
#include <cstdint>
#include <Windows.h>
#include <string>
#include <cstdio>

template <typename T>
inline bool ReadPointerChainAlt(uintptr_t moduleBase, uintptr_t baseOffset, const std::vector<uintptr_t>& offsets, T& outValue, uintptr_t* finalAddress = nullptr) {
    __try {
        uintptr_t addr = moduleBase + baseOffset;
        for (size_t i = 0; i < offsets.size() - 1; i++) {
            addr = *(uintptr_t*)addr;
            addr += offsets[i];       
        }
        addr = *(uintptr_t*)addr;   
        addr += offsets.back();       
        if (finalAddress)
            *finalAddress = addr;
        outValue = *(T*)addr;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

class MemoryReader {
public:
    double distancia;
    float altura;
    float vento;
    float angulo;         
    std::string terreno;
    float slope;
    int powerBar;
    int spin;
    float curva;

    MemoryReader()
        : distancia(0.0), altura(0.0f), vento(0.0f), angulo(0.0f),
        terreno("0"), slope(0.0f), powerBar(0), spin(0), curva(0.0f)
    {
    }

    // Método que atualiza os valores lidos da memória
    void Update() {
        // Leitura direta para outros valores (endereços fictícios - substitua pelos reais)
        distancia = *(double*)(0x00E47E30);  // Exemplo
        altura = *(float*)(0x00E47E30);    // Exemplo
        vento = *(float*)(0x00E47E30);    // Exemplo

        // Leitura do valor "Ângulo" via pointer chain:
        // Cadeia conforme o C#:
        // "ProjectG.exe+00A73E60, 0x34, 0xB4, 0x28, 0x14, 0x30, 0x0, 0x7C"
        // Obtenha a base do módulo "ProjectG.exe" explicitamente.
        uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");
        std::vector<uintptr_t> offsets = { 0x34, 0xB4, 0x28, 0x14, 0x30, 0x0, 0x7C };
        uintptr_t finalAddr = 0;
        if (!ReadPointerChainAlt<float>(moduleBase, 0x00A73E60, offsets, angulo, &finalAddr)) {
            angulo = 0.0f;
            finalAddr = 0;
        }
        // Converter o endereço final para uma string hexadecimal de 8 dígitos
        char buffer[16];
        sprintf_s(buffer, sizeof(buffer), "%08X", finalAddr);
        terreno = buffer;

        // Leitura direta para os demais valores (endereços fictícios - substitua conforme necessário)
        slope = *(float*)(0x00E47E30);
        powerBar = *(int*)(0x00E47E30);
        spin = *(int*)(0x00E47E30);
        curva = *(float*)(0x00E47E30);
    }

    // Padrão Singleton para facilitar o acesso
    static MemoryReader& Instance() {
        static MemoryReader instance;
        return instance;
    }
};
