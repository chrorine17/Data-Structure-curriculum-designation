#pragma once

#include <chrono>
#include <string>
#include <vector>

using namespace std;

// 基础数据结构层：串匹配的结果与测试数据集定义，供各模块复用
struct SearchResult {
    string algorithmName;                                // 算法名称，便于打印标识
    vector<int> positions;                               // 匹配到的起始位置列表
    size_t characterComparisons{};                       // 字符比较次数统计
    chrono::duration<double, milli> duration{};          // 耗时（毫秒）
};

// 测试数据集结构：包含数据集名称、文本和模式串
struct DataSet {
    string name;
    string text;
    string pattern;
};
