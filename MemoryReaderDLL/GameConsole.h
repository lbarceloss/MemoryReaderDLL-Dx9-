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

        if (command == "/help") {
            AddLog("Comandos disponiveis:");
            AddLog("/assist - Ativar ou desativar assistencia.");
            AddLog("/wind - Exibir informacoes do vento.");
            AddLog("/effects - Mostrar efeitos ativos.");
        }
        else if (command == "/assist") {
            AddLog("@assist on - Liga assistencia.");
            AddLog("@assist off - Desliga assistencia.");
            AddLog("@typeshot [0-3] - Define o tipo de jogada.");
            AddLog("  0 = Dunk, 1 = Toma, 2 = Cobra, 3 = Spike");
        }
        else if (command == "@assist on") {
            assistEnabled = true;
            uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");
            if (WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,54", 66, false)) {
                AddLog("[SUCCESS] Assistencia ativada!");
            }
            else {
                AddLog("[ERROR] Falha ao ativar assistencia!");
            }
        }
        else if (command == "@assist off") {
            assistEnabled = false;
            uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");
            if (WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,54", 0, false)) {
                AddLog("[SUCCESS] Assistencia desativada!");
            }
            else {
                AddLog("[ERROR] Falha ao desativar assistencia!");
            }
        }
        else if (command.rfind("@typeshot ", 0) == 0) { // Verifica se começa com "@typeshot "
            int shotType = -1;
            std::istringstream ss(command.substr(10)); // Pega o valor depois de "@typeshot "
            ss >> shotType;

            if (shotType >= 0 && shotType <= 3) {
                typeShot = shotType;
                uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("ProjectG.exe");

                if (WriteMemory(moduleBase, "00B006E8,1C,C,C,14,18,0,4C", shotType, false)) {
                    AddLog("[SUCCESS] Tipo de jogada definido como %d!", shotType);
                }
                else {
                    AddLog("[ERROR] Falha ao definir o tipo de jogada!");
                }
            }
            else {
                AddLog("[ERROR] Comando invalido! Use um valor entre 0 e 3:");
                AddLog("  0 = Dunk, 1 = Toma, 2 = Cobra, 3 = Spike");
            }
        }
        else {
            AddLog("[ERROR] Comando desconhecido! Digite /help para ajuda.");
        }
    }

    void Draw(const char* title, bool* open = nullptr) {
        if (!ImGui::Begin(title, open, ImGuiWindowFlags_NoCollapse)) {
            ImGui::End();
            return;
        }

        // Botão para limpar logs
        if (ImGui::Button("Limpar")) {
            ClearLog();
        }
        ImGui::Separator();

        // Exibe o histórico de comandos
        ImGui::BeginChild("ConsoleScroll", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& log : logEntries) {
            ImGui::TextUnformatted(log.c_str());
        }
        ImGui::EndChild();

        // Campo de entrada para comandos
        if (ImGui::InputText("Digite um comando", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string command(inputBuffer);
            ExecuteCommand(command);
            inputBuffer[0] = '\0'; // Limpa o buffer após executar o comando
        }

        ImGui::End();
    }
};
