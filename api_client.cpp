#define _CRT_SECURE_NO_WARNINGS   //数据入口
#include "api_client.h"
#include "utils.h"          // 因为用到了 file_exists, read_file, write_file
#include <curl/curl.h>
#include <windows.h>        // Sleep
#include <ctime>
#include <iostream>         // std::cout

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}  //会被网络库调用多次  每次接受数据分片  计算实际写入量size&nmumb  

void throttle() {    //限流函数  static time_t lastCall是一个静态局部变量，它的值在函数调用结束后不会丢失，下次调用时依然是上次的值
    static time_t lastCall = 0;
    time_t now = time(nullptr);
    if (now - lastCall < 2) {
        Sleep((DWORD)(2000 - (now - lastCall) * 1000));
    }
    lastCall = time(nullptr);
}

std::string httpGet(const std::string& url) {
    throttle();
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK) ? response : "";
}

std::string getUserInfo(const std::string& handle) {
    return httpGet("https://codeforces.com/api/user.info?handles=" + handle);
}

std::string getUserRating(const std::string& handle) {
    return httpGet("https://codeforces.com/api/user.rating?handle=" + handle);
}

std::string getContestStandings(int contestId, const std::string& handle) {
    std::string cacheFile = "cache/contest_" + std::to_string(contestId) + ".json";
    if (file_exists(cacheFile)) {
        std::cout << "从缓存加载比赛 " << contestId << std::endl;
        return read_file(cacheFile);
    }
    std::string url = "https://codeforces.com/api/contest.standings?contestId="
        + std::to_string(contestId) + "&handles=" + handle + "&showUnofficial=false";
    std::string response = httpGet(url);
    if (!response.empty()) {
        write_file(cacheFile, response);
        std::cout << "已缓存比赛 " << contestId << std::endl;
    }
    return response;
}

std::string getUserStatus(const std::string& handle) {
    return httpGet("https://codeforces.com/api/user.status?handle=" + handle);
}