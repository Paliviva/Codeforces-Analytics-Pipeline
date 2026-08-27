#pragma once
#include <vector>
#include <string>
#include "data_structures.h"

int countRecentContests(const std::vector<RatingChange>& history, int days);
int getMaxRatingRecent(const std::vector<RatingChange>& history, int days);
void computeDifficultyHistogram(const std::vector<RatingChange>& history, const std::string& handle, int binsAll[6], int bins180[6]);
void markAfterContestSolves(std::vector<RatingChange>& history, const std::string& handle);