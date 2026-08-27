#pragma once
#include <string>
#include <vector>
#include "data_structures.h"

void generateHTML(const UserInfo& user, const std::vector<RatingChange>& history, const int binsAll[6], const int bins180[6]);
UserSummary processUser(const std::string& handle);
