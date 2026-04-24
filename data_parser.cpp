#define _CRT_SECURE_NO_WARNINGS
#include "data_parser.h"
#include "third_party/cJSON.h"
#include <cstring>

UserInfo parseUserInfo(const std::string& jsonStr) {
    UserInfo info;
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) return info;
    cJSON* status = cJSON_GetObjectItem(root, "status");
    if (status && strcmp(status->valuestring, "OK") == 0) {
        cJSON* result = cJSON_GetObjectItem(root, "result");
        if (cJSON_IsArray(result) && cJSON_GetArraySize(result) > 0) {
            cJSON* user = cJSON_GetArrayItem(result, 0);
            cJSON* handle = cJSON_GetObjectItem(user, "handle");
            if (handle) info.handle = handle->valuestring;
            cJSON* rating = cJSON_GetObjectItem(user, "rating");
            if (rating) info.rating = rating->valueint;
            cJSON* maxRating = cJSON_GetObjectItem(user, "maxRating");
            if (maxRating) info.maxRating = maxRating->valueint;
            cJSON* rank = cJSON_GetObjectItem(user, "rank");
            if (rank) info.rank = rank->valuestring;
            cJSON* avatar = cJSON_GetObjectItem(user, "avatar");
            if (avatar) info.avatar = avatar->valuestring;
        }
    }
    cJSON_Delete(root);
    return info;
}

std::vector<RatingChange> parseUserRating(const std::string& jsonStr) {
    std::vector<RatingChange> history;
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) return history;
    cJSON* status = cJSON_GetObjectItem(root, "status");
    if (status && strcmp(status->valuestring, "OK") == 0) {
        cJSON* result = cJSON_GetObjectItem(root, "result");
        if (cJSON_IsArray(result)) {
            int size = cJSON_GetArraySize(result);
            for (int i = 0; i < size; i++) {
                cJSON* item = cJSON_GetArrayItem(result, i);
                RatingChange rc;
                cJSON* cid = cJSON_GetObjectItem(item, "contestId");
                if (cid) rc.contestId = cid->valueint;
                cJSON* cname = cJSON_GetObjectItem(item, "contestName");
                if (cname) rc.contestName = cname->valuestring;
                cJSON* ts = cJSON_GetObjectItem(item, "ratingUpdateTimeSeconds");
                if (ts) rc.timestamp = (long long)ts->valuedouble;
                cJSON* old = cJSON_GetObjectItem(item, "oldRating");
                if (old) rc.oldRating = old->valueint;
                cJSON* nw = cJSON_GetObjectItem(item, "newRating");
                if (nw) rc.newRating = nw->valueint;
                cJSON* rk = cJSON_GetObjectItem(item, "rank");
                if (rk) rc.rank = rk->valueint;
                history.push_back(rc);
            }
        }
    }
    cJSON_Delete(root);
    return history;
}

void fillStandingsInfo(const std::string& jsonStr, RatingChange& rc, const std::string& handle) {
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) return;
    cJSON* status = cJSON_GetObjectItem(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0) {
        cJSON_Delete(root);
        return;
    }
    cJSON* result = cJSON_GetObjectItem(root, "result");
    cJSON* rows = cJSON_GetObjectItem(result, "rows");
    cJSON* problems = cJSON_GetObjectItem(result, "problems");
    if (!rows || !problems) {
        cJSON_Delete(root);
        return;
    }
    int rowCount = cJSON_GetArraySize(rows);
    for (int i = 0; i < rowCount; i++) {
        cJSON* row = cJSON_GetArrayItem(rows, i);
        cJSON* party = cJSON_GetObjectItem(row, "party");
        cJSON* members = cJSON_GetObjectItem(party, "members");
        if (cJSON_GetArraySize(members) > 0) {
            cJSON* member = cJSON_GetArrayItem(members, 0);
            cJSON* h = cJSON_GetObjectItem(member, "handle");
            if (h && strcmp(h->valuestring, handle.c_str()) == 0) {
                cJSON* rank = cJSON_GetObjectItem(row, "rank");
                rc.rank = rank ? rank->valueint : 0;
                cJSON* points = cJSON_GetObjectItem(row, "points");
                rc.totalPoints = points ? (float)points->valuedouble : 0.0f;
                cJSON* problemResults = cJSON_GetObjectItem(row, "problemResults");
                int probCount = cJSON_GetArraySize(problemResults);
                rc.problemCount = probCount;
                for (int j = 0; j < probCount && j < 15; j++) {
                    cJSON* pr = cJSON_GetArrayItem(problemResults, j);
                    cJSON* prob = cJSON_GetArrayItem(problems, j);
                    cJSON* index = cJSON_GetObjectItem(prob, "index");
                    rc.problems[j].index = index ? index->valuestring : "";
                    cJSON* pts = cJSON_GetObjectItem(pr, "points");
                    rc.problems[j].points = pts ? (float)pts->valuedouble : 0.0f;
                    cJSON* rej = cJSON_GetObjectItem(pr, "rejectedAttemptCount");
                    rc.problems[j].rejectedAttempts = rej ? rej->valueint : 0;
                }
                break;
            }
        }
    }
    cJSON_Delete(root);
}
