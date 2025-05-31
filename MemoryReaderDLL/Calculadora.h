#include "imgui.h"
#include <vector>
#include <string>
#include <iostream>
#include <curl/curl.h>

// Callback para armazenar a resposta do servidor em uma std::string
size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

class Calculadora {
public:

    static Calculadora& Instance() {
        static Calculadora instance;
        return instance;
    }

    void Draw(const char* title, bool* open = nullptr) {
        ImGui::Begin(title, open);
        ImGui::Text("Calculadora pronta!");
        ImGui::End();
    }


private:
    char inputBuffer[256] = "";
    std::vector<std::string> consoleOutput;

    void SendHTTPRequest() {
        CURL* curl;
        CURLcode res;
        std::string response_data;

        std::string json_data = R"({
            "power": 50,
            "auxpart_pwr": 0,
            "card_pwr": 0,
            "mascot_pwr": 0,
            "ps_card_pwr": 0,
            "club_index": 0,
            "shot_index": 0,
            "power_shot_index": 0,
            "distance": 200,
            "height": 4,
            "wind": 5,
            "degree": 47,
            "ground": 100,
            "spin": 5,
            "curve": 5,
            "slope_break": 0
        })";

        AddToConsole("Enviando JSON...");

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/calcular");
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_data.size());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                AddToConsole("Erro na requisição: " + std::string(curl_easy_strerror(res)));
            }
            else {
                std::string formatted = response_data;
                for (auto& ch : formatted) {
                    if (ch == ',') ch = '\n';
                }
                AddToConsole("Resposta do servidor:\n" + formatted);
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }

        curl_global_cleanup();
    }

    void AddToConsole(const std::string& text) {
        consoleOutput.push_back(text);
    }
};
