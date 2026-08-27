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
#include <sstream> // 需要用到 std::ostringstream

// generateHTML 和 processUser 实现
void generateHTML(const UserInfo& user, const std::vector<RatingChange>& history, const int binsAll[6], const int bins180[6]) {
    // 1. 【生成数据文件的内容，准备注入到模板里】
    std::ostringstream dataStream;
    auto colorAndTitle = getColorAndTitle(user.rating);
    std::string color = colorAndTitle.first;
    std::string title = colorAndTitle.second;
    dataStream << "{\n";
    dataStream << "  \"handle\": \"" << user.handle << "\",\n";
    dataStream << "  \"rating\": " << user.rating << ",\n";
    dataStream << "  \"maxRating\": " << user.maxRating << ",\n";
    dataStream << "  \"rank\": \"" << user.rank << "\",\n";
    dataStream << "  \"color\": \"" << color << "\",\n";
    dataStream << "  \"avatar\": \"" << user.avatar << "\",\n";
    dataStream << "  \"totalContests\": " << (int)history.size() << ",\n";
    dataStream << "  \"recentCount\": " << countRecentContests(history, 180) << ",\n";
    dataStream << "  \"recentMax\": " << getMaxRatingRecent(history, 180) << ",\n";

    // Rating历史
    dataStream << "  \"ratingHistory\": [\n";
    for (size_t i = 0; i < history.size(); i++) {
        dataStream << "    [" << history[i].timestamp * 1000LL << ", " << history[i].newRating << "]";
        if (i != history.size() - 1) dataStream << ",";
        dataStream << "\n";
    }
    dataStream << "  ],\n";

    // 难度分布
    dataStream << "  \"binsAll\": [";
    for (int i = 0; i < 6; i++) { dataStream << binsAll[i]; if (i < 5) dataStream << ", "; }
    dataStream << "],\n";
    dataStream << "  \"bins180\": [";
    for (int i = 0; i < 6; i++) { dataStream << bins180[i]; if (i < 5) dataStream << ", "; }
    dataStream << "],\n";

    // 比赛详情
    dataStream << "  \"contests\": [\n";
    std::vector<RatingChange> sorted = history;
    std::sort(sorted.begin(), sorted.end(), [](const RatingChange& a, const RatingChange& b) { return a.timestamp > b.timestamp; });
    for (size_t i = 0; i < sorted.size(); i++) {
        auto& rc = sorted[i];
        auto oldPair = getColorAndTitle(rc.oldRating);
        std::string oldColor = oldPair.first;
        auto newPair = getColorAndTitle(rc.newRating);
        std::string newColor = newPair.first;
        std::string scores = "";
        for (int j = 0; j < rc.problemCount; j++) {
            scores += rc.problems[j].index + ":" + std::to_string(rc.problems[j].points).substr(0, 4) + " ";
        }
        std::string upsolved = "";
        for (int j = 0; j < rc.problemCount; j++) {
            if (rc.problems[j].solvedAfter) upsolved += rc.problems[j].index + "√ ";
        }

        dataStream << "    {\n";
        dataStream << "      \"name\": \"" << rc.contestName << "\",\n";
        dataStream << "      \"date\": \"" << formatTime(rc.timestamp) << "\",\n";
        dataStream << "      \"oldRating\": " << rc.oldRating << ",\n";
        dataStream << "      \"oldColor\": \"" << oldColor << "\",\n";
        dataStream << "      \"newRating\": " << rc.newRating << ",\n";
        dataStream << "      \"newColor\": \"" << newColor << "\",\n";
        dataStream << "      \"rank\": " << rc.rank << ",\n";
        dataStream << "      \"scores\": \"" << scores << "\",\n";
        dataStream << "      \"upsolved\": \"" << upsolved << "\"\n";
        dataStream << "    }";
        if (i != sorted.size() - 1) dataStream << ",";
        dataStream << "\n";
    }
    dataStream << "  ]\n";
    dataStream << "}\n";

    std::string jsonData = dataStream.str();

    // 2. 【将数据直接注入内置模板，生成自包含的最终报告】
    std::ofstream dst(user.handle + "_report.html", std::ios::binary);
    if (!dst) {
        std::cerr << "无法创建报告文件！" << std::endl;
        return;
    }

    // 我们的 HTML 模板直接写在这里，并用一个占位符 __JSON_DATA__ 等待被替换
    std::string htmlTemplate = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CF Analyzer</title>
    <script src="https://cdn.bootcdn.net/ajax/libs/echarts/5.5.0/echarts.min.js"></script>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .card { background: white; padding: 20px; margin: 20px auto; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); max-width: 1200px; }
        .user-header { display: flex; align-items: center; gap: 20px; }
        .avatar { width: 80px; height: 80px; border-radius: 50%; }
        h1 { margin: 0; }
        table { width: 100%; border-collapse: collapse; }
        th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <div class="card">
        <div class="user-header">
            <img class="avatar" id="avatar" src="" alt="avatar">
            <div>
                <h1 id="handle" style="color: #ff8c00;"></h1>
                <p>Title: <span id="rank"></span> | Rating: <span id="rating"></span></p>
                <p>Max Rating: <span id="maxRating"></span> | Total Contests: <span id="totalContests"></span></p>
                <p>Recent 180d Contests: <span id="recentCount"></span> | Recent 180d Max: <span id="recentMax"></span></p>
            </div>
        </div>
    </div>

    <div class="card">
        <h2>Rating History</h2>
        <div id="ratingChart" style="width:100%;height:400px;"></div>
    </div>

    <div class="card">
        <h2>Problem Difficulty Distribution</h2>
        <div id="difficultyChart" style="width:100%;height:400px;"></div>
    </div>

    <div class="card">
        <h2>Contest Details</h2>
        <table id="contestTable">
            <thead>
                <tr><th>Contest Name</th><th>Date</th><th>Old Rating</th><th>New Rating</th><th>Rank</th><th>Scores</th><th>Upsolved</th></tr>
            </thead>
            <tbody></tbody>
        </table>
    </div>

    <script>
        var data = __JSON_DATA__;

        document.getElementById('handle').textContent = data.handle;
        document.getElementById('handle').style.color = data.color;
        document.getElementById('rank').textContent = data.rank;
        document.getElementById('rating').textContent = data.rating;
        document.getElementById('maxRating').textContent = data.maxRating;
        document.getElementById('totalContests').textContent = data.totalContests;
        document.getElementById('recentCount').textContent = data.recentCount;
        document.getElementById('recentMax').textContent = data.recentMax;
        if(data.avatar) document.getElementById('avatar').src = data.avatar;

        var chart = echarts.init(document.getElementById('ratingChart'));
        chart.setOption({
            xAxis: { type: 'time', name: 'Time' },
            yAxis: { type: 'value', name: 'Rating' },
            series: [{
                data: data.ratingHistory,
                type: 'line', smooth: true,
                lineStyle: { color: '#ff8c00', width: 3 },
                areaStyle: { color: 'rgba(255,140,0,0.1)' }
            }],
            tooltip: { trigger: 'axis' }
        });

        var diffChart = echarts.init(document.getElementById('difficultyChart'));
        diffChart.setOption({
            title: { text: 'AC Problem Difficulty' },
            xAxis: { data: ['<=1000', '1001-1500', '1501-2000', '2001-2500', '2501-3000', '>3000'] },
            yAxis: {},
            series: [
                { name: 'All', type: 'bar', data: data.binsAll },
                { name: 'Recent 180d', type: 'bar', data: data.bins180 }
            ],
            legend: { data: ['All', 'Recent 180d'] },
            tooltip: { trigger: 'axis' },
            grid: { left: '10%', right: '5%', bottom: '10%', top: '15%' }
        });

        var tbody = document.querySelector('#contestTable tbody');
        data.contests.forEach(c => {
            var row = tbody.insertRow();
            row.innerHTML = `
                <td>${c.name}</td><td>${c.date}</td>
                <td style="color:${c.oldColor}">${c.oldRating}</td>
                <td style="color:${c.newColor}">${c.newRating}</td>
                <td>${c.rank}</td><td>${c.scores}</td><td>${c.upsolved}</td>
            `;
        });
    </script>
</body>
</html>
    )";

    // 找到模板中的占位符，并将其替换为我们生成的 JSON 数据字符串
    size_t pos = htmlTemplate.find("__JSON_DATA__");
    if (pos != std::string::npos) {
        htmlTemplate.replace(pos, 13, jsonData); // 13 是 "__JSON_DATA__" 的长度
    }

    dst << htmlTemplate;
    dst.close(); // 这就是正确的写法

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