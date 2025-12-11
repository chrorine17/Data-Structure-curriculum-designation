#pragma once

#include "Types.h"

#include <string>

using namespace std;

// 核心算法模块：封装所有串匹配算法的声明，供主流程统一调用
SearchResult bruteForceSearch(const string& text, const string& pattern);

SearchResult rabinKarpSearch(const string& text, const string& pattern, int base = 256, int modulus = 101);

SearchResult kmpSearch(const string& text, const string& pattern);

SearchResult sundaySearch(const string& text, const string& pattern);

SearchResult boyerMooreSearch(const string& text, const string& pattern);

// 根据用户选择运行对应的匹配算法，返回统计结果
SearchResult runAlgorithm(int choice, const string& text, const string& pattern);
