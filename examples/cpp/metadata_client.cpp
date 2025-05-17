#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>

// 回调函数，用于接收HTTP响应数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        // 处理内存分配失败
        return 0;
    }
}

class ImageForensicsClient {
private:
    std::string baseUrl;
    CURL* curl;

public:
    ImageForensicsClient(const std::string& url = "http://localhost:8080") : baseUrl(url) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
    }

    ~ImageForensicsClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    // 健康检查
    std::string checkHealth() {
        if (!curl) {
            return "{\"error\": \"CURL initialization failed\"}";
        }

        std::string readBuffer;
        std::string url = baseUrl + "/health";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            return "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        return readBuffer;
    }

    // 提取元数据
    std::string extractMetadata(const std::string& imagePath) {
        if (!curl) {
            return "{\"error\": \"CURL initialization failed\"}";
        }

        std::string readBuffer;
        std::string url = baseUrl + "/metadata";

        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;

        // 添加文件到表单
        curl_formadd(&formpost, &lastptr,
                    CURLFORM_COPYNAME, "image",
                    CURLFORM_FILE, imagePath.c_str(),
                    CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        
        // 清理表单
        curl_formfree(formpost);
        
        if (res != CURLE_OK) {
            return "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        return readBuffer;
    }

    // 取证分析
    std::string analyzeForensics(const std::string& imagePath) {
        if (!curl) {
            return "{\"error\": \"CURL initialization failed\"}";
        }

        std::string readBuffer;
        std::string url = baseUrl + "/forensics";

        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;

        // 添加文件到表单
        curl_formadd(&formpost, &lastptr,
                    CURLFORM_COPYNAME, "image",
                    CURLFORM_FILE, imagePath.c_str(),
                    CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        
        // 清理表单
        curl_formfree(formpost);
        
        if (res != CURLE_OK) {
            return "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        return readBuffer;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    ImageForensicsClient client;

    // 检查API健康状态
    std::cout << "Checking API health..." << std::endl;
    std::string healthResponse = client.checkHealth();
    std::cout << "Health response: " << healthResponse << std::endl << std::endl;

    // 提取元数据
    std::cout << "Extracting metadata from " << imagePath << "..." << std::endl;
    std::string metadataResponse = client.extractMetadata(imagePath);
    std::cout << "Metadata response: " << metadataResponse << std::endl << std::endl;

    // 取证分析
    std::cout << "Analyzing forensics for " << imagePath << "..." << std::endl;
    std::string forensicsResponse = client.analyzeForensics(imagePath);
    std::cout << "Forensics response: " << forensicsResponse << std::endl;

    return 0;
} 