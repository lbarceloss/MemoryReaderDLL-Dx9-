#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdarg> // Para `va_list`
#include "imgui.h"
#include "LogConsole.h"

class GameConsole {
public:
    std::vector<std::string> logEntries;
    char inputBuffer[128]; // Buffer para entrada do usuário
    bool assistEnabled = false; // Estado do assist (On/Off)
    int typeShot = 0; // 0 = Dunk, 1 = Toma, 2 = Cobra, 3 = Spike

    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total_size = size * nmemb;
        output->append(static_cast<char*>(contents), total_size);
        return total_size;
    }

    static GameConsole& Instance() {
        static GameConsole instance;
        return instance;
    }

    void AddLog(const char* format, ...) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        logEntries.push_back(std::string(buffer));
    }

    void ClearLog() {
        logEntries.clear();
    }

    void ExecuteCommand(const std::string& command) {
        AddLog("> %s", command.c_str()); // Exibe o comando digitado
        uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");

        // Para ver os comandos
        if (command == "/help") {
            AddLog("/assist para ver os comandos do assist.");
            AddLog("/wind para ver os comandos do wind.");
            AddLog("/spincurve para ver os comandos do spin e curva.");
            AddLog("/gamesets para ver os comandos do gamesets.");
            return;
        }

        // Assistência e tipos de jogada
        if (command == "/assist") {
            AddLog("@assist on/off - Liga/desliga assistencia.");
            AddLog("@typeshot [0-3] - Define o tipo de jogada.");
            AddLog("    [0] Dunk.");
            AddLog("    [1] Toma.");
            AddLog("    [2] Cobra.");
            AddLog("    [3] Spike.");
            AddLog("@precise on/off - Ativa/desativa precisao.");
        }
        /*
        else if (command == "@assist on") {
            WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,54", 66, false) ?
                AddLog("[SUCCESS] Assist ativado!") : AddLog("[ERROR] Falha ao ativar assist!");
        }
        else if (command == "@assist off") {
            WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,54", 0, false) ?
                AddLog("[SUCCESS] Assist desativada!") : AddLog("[ERROR] Falha ao desativar assist!");
        }
        else if (command == "@typeshot 0") {
            WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,4C", 0, false) ?
                AddLog("[SUCCESS] Dunk ativado!") : AddLog("[ERROR] Falha ao ativar assist Dunk");
        }
        else if (command == "@typeshot 1") {
            WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,4C", 1, false) ?
                AddLog("[SUCCESS] Toma ativado!") : AddLog("[ERROR] Falha ao ativar assist Toma");
        }
        else if (command == "@typeshot 2") {
            WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,4C", 2, false) ?
                AddLog("[SUCCESS] Cobra ativado!") : AddLog("[ERROR] Falha ao ativar assist Cobra");
        }
        else if (command == "@typeshot 3") {
            WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,4C", 3, false) ?
                AddLog("[SUCCESS] Spike ativado!") : AddLog("[ERROR] Falha ao ativar assist Spike");
        }

        // Wind e Degree
        else if (command == "/wind") {
            AddLog("@windf [0-99] - Define forca do vento.");
            AddLog("@degree [0-360] - Define angulo do vento.");
        }
        else if (command.rfind("@windf ", 0) == 0) {
            int windForce;
            std::istringstream ss(command.substr(7));
            ss >> windForce;

            if (windForce >= 0 && windForce <= 99) {
                WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,68", windForce, false) ?
                    AddLog("[SUCCESS] Forca do vento definida como %d!", windForce) :
                    AddLog("[ERROR] Falha ao definir a forca do vento!");
            }
            else {
                AddLog("[ERROR] Valor inválido! Use de 0 a 99.");
            }
        }
        else if (command.rfind("@degree ", 0) == 0) {
            float degree;
            std::istringstream ss(command.substr(8));
            ss >> degree;

            if (degree >= 0.0f && degree <= 360.0f) {
                WriteMemoryFloat(moduleBase, "00B006E8,1C,C,C,14,18,0,6C", degree, false) ?
                    AddLog("[SUCCESS] Angulo do vento definido como %.2f!", degree) :
                    AddLog("[ERROR] Falha ao definir o angulo do vento!");
            }
            else {
                AddLog("[ERROR] Valor invalido! Use de 0 a 360.");
            }
        }

        // Spin e Curve
        else if (command == "/spincurve") {
            AddLog("@spin [0-30] - Define Spin.");
            AddLog("@curve [0-30] - Define Curve.");
        }
        else if (command.rfind("@spin ", 0) == 0) {
            float spin;
            std::istringstream ss(command.substr(6));
            ss >> spin;

            if (spin >= -30.0f && spin <= 30.0f) {
                WriteMemoryFloat(moduleBase, "00A73E60,34,18,10,3C,30,0,1C", spin, false) ?
                    AddLog("[SUCCESS] Spin definido como %.2f!", spin) :
                    AddLog("[ERROR] Falha ao definir o Spin!");
            }
            else {
                AddLog("[ERROR] Valor invalido! Use de -30 a 30.");
            }
        }
        else if (command.rfind("@curve ", 0) == 0) {
            float curve;
            std::istringstream ss(command.substr(7));
            ss >> curve;

            if (curve >= -30.0f && curve <= 30.0f) {
                WriteMemoryFloat(moduleBase, "00B006E8,8,C,C,40,0,0,18", curve, false) ?
                    AddLog("[SUCCESS] Curve definido como %.2f!", curve) :
                    AddLog("[ERROR] Falha ao definir o Curve!");
            }
            else {
                AddLog("[ERROR] Valor invalido! Use de -30 a 30.");
            }
        }
        // Utils
        else if (command == "/gamesets") {
            AddLog("@gravity - Define gravidade.");
            AddLog("@balldiameter - Define diâmetro da bola.");
            AddLog("@ballmass  - Define massa da bola.");
            AddLog("@curveconst - Define constante de curva.");
            AddLog("@spinconst - Define constante de spin.");
            AddLog("@utilsreset - Reseta valores para os padroes.");
        }
        /*
        else if (command.rfind("@gravity ", 0) == 0) {
            double gravity;
            std::istringstream ss(command.substr(9));
            ss >> gravity;

            if (WriteMemoryRelative<double>("ProjectG.exe", 0x90AA28, gravity)) {
                AddLog("[SUCCESS] Gravidade definida como %.2f!", gravity);
            }
            else {
                AddLog("[ERROR] Falha ao definir a gravidade!");
            }
        }
        else if (command.rfind("@balldiameter ", 0) == 0) {
            float diameter;
            std::istringstream ss(command.substr(14));
            ss >> diameter;

            if (WriteMemoryRelative<float>("ProjectG.exe", 0xA47EF8, diameter)) {
                AddLog("[SUCCESS] Diâmetro da bola definido como %.2f!", diameter);
            }
            else {
                AddLog("[ERROR] Falha ao definir o diâmetro da bola!");
            }
        }
        else if (command.rfind("@ballmass ", 0) == 0) {
            float diameter;
            std::istringstream ss(command.substr(14));
            ss >> diameter;

            if (WriteMemoryRelative<float>("ProjectG.exe", 0xA47EF4, diameter)) {
                AddLog("[SUCCESS] Diâmetro da bola definido como %.2f!", diameter);
            }
            else {
                AddLog("[ERROR] Falha ao definir o diâmetro da bola!");
            }
            }
        else if (command == "@utilsreset") {
            WriteMemoryDouble(moduleBase, "00B006E8,1C,C,C,14,18,0,78", 9.81, false);
            WriteMemoryFloat(moduleBase, "00B006E8,1C,C,C,14,18,0,7C", 1.68f, false);
            AddLog("[SUCCESS] Valores resetados para padrao!");
        }
        else {
            AddLog("[ERROR] Comando desconhecido! Digite /help para ajuda.");
        }
        */
    }
    

    void Draw(const char* title, bool* open = nullptr) {
        if (!ImGui::Begin(title, open, ImGuiWindowFlags_NoCollapse)) {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Limpar")) {
            ClearLog();
        }
        ImGui::Separator();

        ImGui::BeginChild("ConsoleScroll", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& log : logEntries) {
            ImGui::TextUnformatted(log.c_str());
        }
        ImGui::EndChild();

        if (ImGui::InputText("Digite um comando", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string command(inputBuffer);
            ExecuteCommand(command);
            inputBuffer[0] = '\0';
        }

        ImGui::End();
    }
};
