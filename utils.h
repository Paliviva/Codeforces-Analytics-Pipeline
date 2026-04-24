#pragma once
#include <string>
#include <utility>

// 文件缓存
bool file_exists(const std::string& filename);
std::string read_file(const std::string& filename);
void write_file(const std::string& filename, const std::string& content);

// 颜色映射
std::pair<std::string, std::string> getColorAndTitle(int rating);

// 时间格式化
std::string formatTime(long long timestamp);