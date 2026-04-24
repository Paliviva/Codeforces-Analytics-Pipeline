#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <curl/curl.h>
#include "third_party/cJSON.h"
#include <direct.h>
#include "data_structures.h"
#include "utils.h"
#include "api_client.h"
#include "data_parser.h"
#include "statistics.h"
#include "html_render.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")

int main() {
    (void)_mkdir("cache");   // 明确忽略返回值

    std::cout << "选择模式：\n";
    std::cout << "  1 - 单用户（输入用户名）\n";
    std::cout << "  2 - 多用户（从 users.txt 读取）\n";
    std::cout << "请输入选择 (1/2): ";
    int mode;
    std::cin >> mode;
    std::cin.ignore();

    if (mode == 1) {
        std::string handle;
        std::cout << "请输入 Codeforces 用户名: ";
        std::cin >> handle;
        processUser(handle);
    }
    else if (mode == 2) {
        std::ifstream userFile("users.txt");
        if (!userFile) {
            std::cerr << "无法打开 users.txt，请确保文件存在于程序目录下！" << std::endl;
            system("pause");
            return 1;
        }

        std::vector<UserSummary> allSummaries;
        std::string handle;
        while (std::getline(userFile, handle)) {
            size_t start = handle.find_first_not_of(" \t\r\n");
            size_t end = handle.find_last_not_of(" \t\r\n");
            if (start == std::string::npos) continue;
            handle = handle.substr(start, end - start + 1);
            if (handle.empty()) continue;

            UserSummary sum = processUser(handle);
            if (sum.rating > 0 || !sum.rank.empty()) {
                allSummaries.push_back(sum);
            }
        }
        userFile.close();

        if (!allSummaries.empty()) {
            std::ofstream indexOut("index.html", std::ios::binary);
            if (indexOut) {
                const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
                indexOut.write(reinterpret_cast<const char*>(bom), sizeof(bom));

                indexOut << "<!DOCTYPE html>\n<html>\n<head>\n";
                indexOut << "<meta charset=\"UTF-8\">\n";
                indexOut << "<title>Codeforces 多用户总览</title>\n";
                indexOut << "<style>\n";
                indexOut << "body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n";
                indexOut << "table { border-collapse: collapse; width: 100%; background: white; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n";
                indexOut << "th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }\n";
                indexOut << "th { background: #5470c6; color: white; }\n";
                indexOut << "tr:hover { background: #f1f1f1; }\n";
                indexOut << "a { text-decoration: none; font-weight: bold; }\n";
                indexOut << "</style>\n</head>\n<body>\n";
                indexOut << "<h1>Codeforces 用户总览</h1>\n";
                indexOut << "<table>\n";
                indexOut << "<tr><th>用户名</th><th>当前Rating</th><th>头衔</th><th>最高Rating</th><th>总比赛</th><th>近180天比赛</th><th>近180天最高</th></tr>\n";

                for (const auto& s : allSummaries) {
                    indexOut << "<tr>";
                    indexOut << "<td><a href=\"" << s.handle << "_report.html\" style=\"color:" << s.color << ";\">" << s.handle << "</a></td>";
                    indexOut << "<td style=\"color:" << s.color << ";\">" << s.rating << "</td>";
                    indexOut << "<td>" << s.rank << "</td>";
                    indexOut << "<td>" << s.maxRating << "</td>";
                    indexOut << "<td>" << s.totalContests << "</td>";
                    indexOut << "<td>" << s.recent180Count << "</td>";
                    indexOut << "<td>" << s.recent180Max << "</td>";
                    indexOut << "</tr>\n";
                }

                indexOut << "</table>\n</body>\n</html>";
                indexOut.close();
                std::cout << "\n多用户总览页面已生成: index.html" << std::endl;
            }
        }
    }
    else {
        std::cout << "无效选择！" << std::endl;
    }

    system("pause");
    return 0;
}