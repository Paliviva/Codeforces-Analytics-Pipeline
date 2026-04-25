#pragma once
#include <string>
struct UserInfo {
    std::string handle;
    int rating = 0;
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
};

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
 