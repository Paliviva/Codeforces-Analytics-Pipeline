#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "html_render.h"
#include "api_client.h"
#include "data_parser.h"
#include "statistics.h"
#include "utils.h"
#include <fstream>
#include <algorithm>
#include <iomanip>

// generateHTML 和 processUser 实现
void generateHTML(const UserInfo& user, const std::vector<RatingChange>& history, const int binsAll[6], const int bins180[6]) {
    std::ofstream out(user.handle + "_report.html", std::ios::binary);
    if (!out) {
        std::cerr << "无法创建HTML文件！" << std::endl;
        return;
    }
    // 写入UTF-8 BOM
    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    out.write(reinterpret_cast<const char*>(bom), sizeof(bom));
    std::vector<RatingChange> sorted = history;
    std::sort(sorted.begin(), sorted.end(), [](const RatingChange& a, const RatingChange& b) {
        return a.timestamp > b.timestamp;
        });

    std::pair<std::string, std::string> userPair = getColorAndTitle(user.rating);
    std::string color = userPair.first;
    std::string title = userPair.second;

    int totalContests = (int)history.size();
    int recent180Count = countRecentContests(history, 180);
    int recent180Max = getMaxRatingRecent(history, 180);

    // HTML头部
    out << "<!DOCTYPE html>\n<html>\n<head>\n";
    out << "<meta charset=\"UTF-8\">\n";
    out << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
    out << "<title>Codeforces Clawer - " << user.handle << "</title>\n";
    out << "<script src=\"https://cdn.bootcdn.net/ajax/libs/echarts/5.5.0/echarts.min.js\"></script>\n";
    out << "<style>\n";
    out << "body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n";
    out << ".card { background: white; padding: 20px; margin: 20px 0; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n";
    out << ".user-header { display: flex; align-items: center; gap: 20px; }\n";
    out << ".avatar { width: 80px; height: 80px; border-radius: 50%; }\n";
    out << "h1 { margin: 0; }\n";
    out << "table { width: 100%; border-collapse: collapse; }\n";
    out << "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }\n";
    out << "</style>\n</head>\n<body>\n";

    // 用户信息卡片
    out << "<div class=\"card\">\n<div class=\"user-header\">\n";
    out << "<img class=\"avatar\" src=\"" << user.avatar << "\" alt=\"avatar\">\n";
    out << "<div>\n";
    out << "<h1 style=\"color: " << color << ";\">" << user.handle << "</h1>\n";
    out << "<p>当前头衔: " << user.rank << " | 当前等级分: " << user.rating << "</p>\n";
    out << "<p>最高等级分: " << user.maxRating << " | 总比赛次数: " << totalContests << "</p>\n";
    out << "<p>近180天比赛次数: " << recent180Count << " | 近180天最高分: " << recent180Max << "</p>\n";
    out << "</div></div></div>\n";

    // ECharts折线图
    out << "<div class=\"card\">\n<h2>Rating变化曲线</h2>\n";
    out << "<div id=\"ratingChart\" style=\"width:100%;height:400px;\"></div>\n</div>\n";
    out << "<script>\n";
    out << "var chart = echarts.init(document.getElementById('ratingChart'));\n";
    out << "var option = {\n";
    out << "  xAxis: { type: 'time', name: '比赛时间' },\n";
    out << "  yAxis: { type: 'value', name: 'Rating' },\n";
    out << "  series: [{\n";
    out << "    data: [\n";
    for (size_t i = 0; i < sorted.size(); i++) {
        out << "      [" << sorted[i].timestamp * 1000LL << ", " << sorted[i].newRating << "]";
        if (i != sorted.size() - 1) out << ",";
        out << "\n";
    }
    out << "    ],\n";
    out << "    type: 'line',\n";
    out << "    smooth: true,\n";
    out << "    lineStyle: { color: '#ff8c00', width: 3 },\n";
    out << "    areaStyle: { color: 'rgba(255,140,0,0.1)' }\n";
    out << "  }],\n";
    out << "  tooltip: { trigger: 'axis' }\n";
    out << "};\n";
    out << "chart.setOption(option);\n</script>\n";

    // 难度直方图（全部 + 近180天）
    out << "<div class=\"card\">\n<h2>题目难度分布</h2>\n";
    out << "<div id=\"difficultyChart\" style=\"width:100%;height:400px;\"></div>\n</div>\n";
    out << "<script>\n";
    out << "var diffChart = echarts.init(document.getElementById('difficultyChart'));\n";
    out << "var diffOption = {\n";
    out << "  title: { text: 'AC题目难度分布' },\n";
    out << "  xAxis: { data: ['≤1000', '1001-1500', '1501-2000', '2001-2500', '2501-3000', '>3000'] },\n";
    out << "  yAxis: {},\n";
    out << "  series: [\n";
    out << "    { name: '全部', type: 'bar', data: [";
    for (int i = 0; i < 6; i++) { out << binsAll[i]; if (i < 5) out << ", "; }
    out << "] },\n";
    out << "    { name: '近180天', type: 'bar', data: [";
    for (int i = 0; i < 6; i++) { out << bins180[i]; if (i < 5) out << ", "; }
    out << "] }\n";
    out << "  ],\n";
    out << "  legend: { data: ['全部', '近180天'] },\n";
    out << "  tooltip: { trigger: 'axis' },\n";
    out << "  grid: { left: '10%', right: '5%', bottom: '10%', top: '15%' }\n";
    out << "};\n";
    out << "diffChart.setOption(diffOption);\n</script>\n";

    // 比赛详情表格
    out << "<div class=\"card\">\n<h2>比赛详情</h2>\n<table>\n";
    out << "<tr><th>赛事名称</th><th>比赛时间</th><th>赛前Rating</th><th>赛后Rating</th><th>排名</th><th>各题得分</th><th>补题情况</th></tr>\n";
    for (const auto& rc : sorted) {
        std::pair<std::string, std::string> oldPair = getColorAndTitle(rc.oldRating);
        std::string oldColor = oldPair.first;
        std::pair<std::string, std::string> newPair = getColorAndTitle(rc.newRating);
        std::string newColor = newPair.first;

        out << "<tr>";
        out << "<td>" << rc.contestName << "</td>";
        out << "<td>" << formatTime(rc.timestamp) << "</td>";
        out << "<td style=\"color:" << oldColor << ";\">" << rc.oldRating << "</td>";
        out << "<td style=\"color:" << newColor << ";\">" << rc.newRating << "</td>";
        out << "<td>" << rc.rank << "</td>";
        out << "<td>";
        for (int j = 0; j < rc.problemCount; j++) {
            out << rc.problems[j].index << ":" << rc.problems[j].points << " ";
        }
        out << "</td>";
        out << "<td>";
        for (int j = 0; j < rc.problemCount; j++) {
            if (rc.problems[j].solvedAfter) out << rc.problems[j].index << "√ ";
        }
        out << "</td>";
        out << "</tr>\n";
    }
    out << "</table>\n</div>\n</body>\n</html>";
    out.close();
    std::cout << "报告已生成: " << user.handle << "_report.html" << std::endl;
}

// 处理单个用户，生成报告，并返回摘要信息
UserSummary processUser(const std::string& handle) {
    UserSummary summary;
    summary.handle = handle;

    std::cout << "\n===== 正在处理用户: " << handle << " =====\n";

    // 1. 获取用户信息
    std::string userJson = getUserInfo(handle);
    if (userJson.empty()) {
        std::cerr << "获取用户信息失败！跳过该用户。" << std::endl;
        summary.rating = 0;
        return summary;
    }
    UserInfo user = parseUserInfo(userJson);
    std::cout << "用户: " << user.handle << ", Rating: " << user.rating << std::endl;

    // 填充摘要
    summary.rating = user.rating;
    summary.maxRating = user.maxRating;
    summary.rank = user.rank;
    std::pair<std::string, std::string> colorPair = getColorAndTitle(user.rating);
    summary.color = colorPair.first;

    // 2. 获取比赛历史
    std::string ratingJson = getUserRating(handle);
    if (ratingJson.empty()) {
        std::cerr << "获取比赛历史失败！跳过该用户。" << std::endl;
        return summary;
    }
    std::vector<RatingChange> history = parseUserRating(ratingJson);
    std::cout << "共找到 " << history.size() << " 场比赛记录" << std::endl;

    summary.totalContests = (int)history.size();
    summary.recent180Count = countRecentContests(history, 180);
    summary.recent180Max = getMaxRatingRecent(history, 180);

    // 3. 补全每场比赛的各题分数
    std::cout << "正在获取各场比赛详细数据..." << std::endl;
    for (auto& rc : history) {
        std::string standingsJson = getContestStandings(rc.contestId, handle);
        if (!standingsJson.empty()) {
            fillStandingsInfo(standingsJson, rc, handle);
        }
        throttle();
    }

    // 4. 标记赛后补题
    markAfterContestSolves(history, handle);

    // 5. 计算难度分布
    int binsAll[6], bins180[6];
    computeDifficultyHistogram(history, handle, binsAll, bins180);

    // 6. 生成HTML报告
    generateHTML(user, history, binsAll, bins180);

    return summary;
}