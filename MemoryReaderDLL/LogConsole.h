#pragma once
#include <vector>
#include <string>
#include <mutex>
#include "imgui.h"

class LogConsole {
public:
    static LogConsole& Instance() {
        static LogConsole instance;
        return instance;
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        logs.clear();
    }

    void AddLog(const char* format, ...) {
        std::lock_guard<std::mutex> lock(mutex_);
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        logs.push_back(std::string(buffer));
    }

    void Draw(const char* title, bool* open = nullptr) {
        if (!ImGui::Begin(title, open, ImGuiWindowFlags_NoCollapse)) {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear")) {
            Clear();
        }
        ImGui::Separator();

        ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& log : logs) {
            ImGui::TextUnformatted(log.c_str());
        }

        if (autoScroll && !logs.empty()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

private:
    std::vector<std::string> logs;
    std::mutex mutex_;
    bool autoScroll = true;

    LogConsole() = default;
    ~LogConsole() = default;
    LogConsole(const LogConsole&) = delete;
    LogConsole& operator=(const LogConsole&) = delete;
};
