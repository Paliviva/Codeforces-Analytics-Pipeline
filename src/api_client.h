#pragma once
#include <string>
void throttle();
std::string httpGet(const std::string& url);
std::string getUserInfo(const std::string& handle);
std::string getUserRating(const std::string& handle);
std::string getContestStandings(int contestId, const std::string& handle);
std::string getUserStatus(const std::string& handle);