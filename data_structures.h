#pragma once    //设计图纸
#include <string>
struct UserInfo {
    std::string handle;
    int rating = 0;//如果从API抓来的数据没有rating字段，这个变量会自动设为0，防止后续计算崩溃。
    int maxRating = 0;
    std::string rank;
    std::string avatar;
};

struct RatingChange {
    int contestId = 0;
    std::string contestName;
    long long timestamp = 0;
    int oldRating = 0;
    int newRating = 0;
    int rank = 0;
    float totalPoints = 0.0f;
    struct {
        std::string index;
        float points = 0.0f;
        int rejectedAttempts = 0;
        int solvedAfter = 0;
    } problems[15];
    int problemCount = 0;
};//嵌套结构体 因为每道题的“得分”、“错了几次”、“是否补题”这三个信息是天然绑定的

struct UserSummary {
    std::string handle;
    int rating = 0;
    int maxRating = 0;
    std::string rank;
    std::string color;
    int totalContests = 0;
    int recent180Count = 0;
    int recent180Max = 0;
};

struct Submission {
    long long creationTimeSeconds = 0;
    std::string verdict;
    int contestId = 0;
    std::string problemIndex;
    int problemRating = 0;
};
 