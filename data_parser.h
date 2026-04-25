#pragma once
#include <string>
#include <vector>
#include "data_structures.h"

UserInfo parseUserInfo(const std::string& jsonStr);
std::vector<RatingChange> parseUserRating(const std::string& jsonStr);
std::vector<Submission> parseUserStatus(const std::string& jsonStr);
void fillStandingsInfo(const std::string& jsonStr, RatingChange& rc, const std::string& handle);
