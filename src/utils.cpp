#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>  // for _mkdir on Windows

bool file_exists(const std::string& filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}
 
std::string read_file(const std::string& filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) return "";
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

void write_file(const std::string& filename, const std::string& content) {
    std::ofstream out(filename, std::ios::out | std::ios::binary);
    out << content;
}

std::pair<std::string, std::string> getColorAndTitle(int rating) {
    if (rating < 1200) return { "#808080", "Newbie" };
    if (rating < 1400) return { "#008000", "Pupil" };
    if (rating < 1600) return { "#03a89e", "Specialist" };
    if (rating < 1900) return { "#0000ff", "Expert" };
    if (rating < 2100) return { "#aa00aa", "Candidate Master" };
    if (rating < 2300) return { "#ff8c00", "Master" };
    if (rating < 2400) return { "#ff8c00", "International Master" };
    if (rating < 2600) return { "#ff0000", "Grandmaster" };
    if (rating < 3000) return { "#ff0000", "International Grandmaster" };
    return { "#ff0000", "Legendary Grandmaster" };
}

std::string formatTime(long long timestamp) {
    time_t t = timestamp;
    struct tm tm_info;
    localtime_s(&tm_info, &t);  // 使用安全版本
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm_info);
    return std::string(buffer);
}