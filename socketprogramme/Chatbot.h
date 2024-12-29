#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

static size_t WriteCallbackGemini(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

class GeminiAPIClient {
private:
    std::string apiKey;
    std::string baseUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent";

    // Xử lý lỗi cURL
    void handleCurlError(CURLcode res) {
        if (res != CURLE_OK) {
            throw std::runtime_error("cURL error: " + std::string(curl_easy_strerror(res)));
        }
    }

public:
    GeminiAPIClient(const std::string& key) : apiKey(key) {}

    std::string sendRequest(const std::string& prompt) {
        CURL* curl = nullptr;
        CURLcode res;
        std::string responseBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (!curl) {
            curl_global_cleanup();
            throw std::runtime_error("Failed to initialize cURL");
        }

        try {
            // Tạo URL với API key
            std::string fullUrl = baseUrl + "?key=" + apiKey;

            // Tạo payload JSON
            json payload = {
                {"contents", {
                    {{"parts", {
                        {{"text", prompt}}
                    }}}
                }}
            };

            std::string payloadString = payload.dump();

            // Thiết lập headers
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");

            // Cấu hình cURL
            curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadString.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackGemini);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

            // Thực hiện request
            res = curl_easy_perform(curl);
            handleCurlError(res);

            // Dọn dẹp
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();

            return responseBuffer;
        }
        catch (const std::exception& e) {
            if (curl) {
                curl_easy_cleanup(curl);
                curl_global_cleanup();
            }
            throw;
        }
    }

    // Phương thức phân tích JSON phản hồi
    std::string parseResponse(const std::string& jsonResponse) {
        try {
            auto parsedJson = json::parse(jsonResponse);

            // Trích xuất văn bản từ phản hồi
            if (parsedJson.contains("candidates") &&
                !parsedJson["candidates"].empty() &&
                parsedJson["candidates"][0].contains("content") &&
                parsedJson["candidates"][0]["content"].contains("parts") &&
                !parsedJson["candidates"][0]["content"]["parts"].empty()) {

                return parsedJson["candidates"][0]["content"]["parts"][0]["text"];
            }

            return "Không thể phân tích phản hồi";
        }
        catch (const std::exception& e) {
            return "Lỗi phân tích JSON: " + std::string(e.what());
        }
    }
};

