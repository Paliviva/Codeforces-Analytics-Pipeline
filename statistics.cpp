#define _CRT_SECURE_NO_WARNINGS
#include "statistics.h"
#include "api_client.h"
#include "data_parser.h"
#include "third_party/cJSON.h"
#include <ctime>
#include <cstring>

// 四个函数实现，直接剪切过来
void markAfterContestSolves(std::vector<RatingChange>& history, const std::string& handle) {
    // 1. 获取用户所有提交
    std::string statusJson = getUserStatus(handle);
    if (statusJson.empty()) return;

    cJSON* root = cJSON_Parse(statusJson.c_str());
    if (!root) return;
    cJSON* status = cJSON_GetObjectItem(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0) {
        cJSON_Delete(root);
        return;
    }
    cJSON* result = cJSON_GetObjectItem(root, "result");
    if (!cJSON_IsArray(result)) {
        cJSON_Delete(root);
        return;
    }

    // 2. 遍历提交，找出每个比赛每道题的首次AC时间
    int subCount = cJSON_GetArraySize(result);
    for (int i = 0; i < subCount; i++) {
        cJSON* sub = cJSON_GetArrayItem(result, i);
        cJSON* verdict = cJSON_GetObjectItem(sub, "verdict");
        if (!verdict || strcmp(verdict->valuestring, "OK") != 0) continue; // 只看AC

        cJSON* problem = cJSON_GetObjectItem(sub, "problem");
        if (!problem) continue;
        cJSON* contestIdItem = cJSON_GetObjectItem(problem, "contestId");
        cJSON* indexItem = cJSON_GetObjectItem(problem, "index");
        cJSON* timeItem = cJSON_GetObjectItem(sub, "creationTimeSeconds");
        if (!contestIdItem || !indexItem || !timeItem) continue;

        int contestId = contestIdItem->valueint;
        std::string index = indexItem->valuestring;
        long long submitTime = (long long)timeItem->valuedouble;

        // 找到对应的比赛
        for (auto& rc : history) {
            if (rc.contestId != contestId) continue;
            // 找到对应的题
            for (int j = 0; j < rc.problemCount; j++) {
                if (rc.problems[j].index == index) {
                    // 如果这道题之前没被标记过AC，或者这次提交更早，记录时间
                    // 但判定补题只需要知道“是否在比赛结束后AC”，所以先记下最早AC时间也行
                    // 简化：只要有一次AC，就检查时间
                    // 我们需要比赛结束时间，但当前没有存，可以用一个估算：ratingUpdateTimeSeconds是比赛rating更新时间，通常在比赛结束后不久
                    // 更准确的做法：从contest.list获取duration，但这里简化：用ratingUpdateTimeSeconds作为比赛结束参考
                    if (submitTime > rc.timestamp) { // rc.timestamp是ratingUpdateTimeSeconds
                        rc.problems[j].solvedAfter = 1;
                    }
                    else {
                        rc.problems[j].solvedAfter = 0; // 场内AC
                    }
                    break;
                }
            }
        }
    }
    cJSON_Delete(root);
}
// ==================== 统计计算 ====================
int countRecentContests(const std::vector<RatingChange>& history, int days) {
    time_t now = time(nullptr);
    time_t cutoff = now - days * 24 * 3600;
    int cnt = 0;
    for (const auto& rc : history) {
        if (rc.timestamp >= cutoff) cnt++;
    }
    return cnt;
}

int getMaxRatingRecent(const std::vector<RatingChange>& history, int days) {
    time_t now = time(nullptr);
    time_t cutoff = now - days * 24 * 3600;
    int maxR = 0;
    for (const auto& rc : history) {
        if (rc.timestamp >= cutoff && rc.newRating > maxR) {
            maxR = rc.newRating;
        }
    }
    return maxR;
}

void computeDifficultyHistogram(const std::vector<RatingChange>& history,
    const std::string& handle,
    int binsAll[6], int bins180[6]) {
    // 初始化
    memset(binsAll, 0, 6 * sizeof(int));
    memset(bins180, 0, 6 * sizeof(int));

    // 获取用户所有AC提交
    std::string statusJson = getUserStatus(handle);
    if (statusJson.empty()) return;
    cJSON* root = cJSON_Parse(statusJson.c_str());
    if (!root) return;
    cJSON* result = cJSON_GetObjectItem(root, "result");
    if (!cJSON_IsArray(result)) { cJSON_Delete(root); return; }

    time_t now = time(nullptr);
    time_t cutoff180 = now - 180 * 24 * 3600;

    int subCount = cJSON_GetArraySize(result);
    for (int i = 0; i < subCount; i++) {
        cJSON* sub = cJSON_GetArrayItem(result, i);
        cJSON* verdict = cJSON_GetObjectItem(sub, "verdict");
        if (!verdict || strcmp(verdict->valuestring, "OK") != 0) continue;
        cJSON* problem = cJSON_GetObjectItem(sub, "problem");
        if (!problem) continue;
        cJSON* ratingItem = cJSON_GetObjectItem(problem, "rating");
        if (!ratingItem) continue;
        int rating = ratingItem->valueint;
        cJSON* timeItem = cJSON_GetObjectItem(sub, "creationTimeSeconds");
        long long submitTime = (long long)timeItem->valuedouble;

        // 确定档位
        int idx = 0;
        if (rating <= 1000) idx = 0;
        else if (rating <= 1500) idx = 1;
        else if (rating <= 2000) idx = 2;
        else if (rating <= 2500) idx = 3;
        else if (rating <= 3000) idx = 4;
        else idx = 5;

        binsAll[idx]++;
        if (submitTime >= cutoff180) bins180[idx]++;
    }
    cJSON_Delete(root);
}