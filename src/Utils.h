#pragma once

#include "Types.h"

#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

// 辅助功能模块：负责数据集生成与结果打印，避免主流程混杂细节
inline string generateRandomString(int length, const string& alphabet) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, static_cast<int>(alphabet.size() - 1));

    string result;
    result.reserve(length);
    // 循环生成随机字符，并顺序追加到结果字符串中
    for (int i = 0; i < length; ++i) {
        result.push_back(alphabet[dist(gen)]);
    }
    return result;
}

// 构建若干不同特性的内置数据集，覆盖长度、分布与模式频率
inline vector<DataSet> buildDataSets() {
    vector<DataSet> dataSets;
    dataSets.push_back({ "短文本重复", "abracadabra abracadabra", "abra" });

    string balanced = generateRandomString(5000, "abcdefghijklmnopqrstuvwxyz");
    string commonPattern = balanced.substr(250, 10);
    dataSets.push_back({ "中等随机", balanced, commonPattern });

    string skewed = generateRandomString(4000, "aaaaab");
    dataSets.push_back({ "偏斜字符分布", skewed, "aaaaa" });

    string longText = generateRandomString(20000, "abcde");
    dataSets.push_back({ "长文本低频模式", longText, "edc" });

    return dataSets;
}

// 打印数据集概要，方便用户选择或改为自定义输入
inline void printDataSets(const vector<DataSet>& dataSets) {
    for (size_t i = 0; i < dataSets.size(); ++i) {
        cout << i + 1 << ". " << dataSets[i].name << " | 文本长度: "
            << dataSets[i].text.size() << " | 模式: " << dataSets[i].pattern
            << "\n";
    }
    cout << dataSets.size() + 1 << ". 自定义输入\n";
}

// 打印一次搜索的结果，输出匹配位置、比较次数耗时
inline void printResult(const SearchResult& result) {
    cout << "算法: " << result.algorithmName << '\n';
    cout << "匹配位置: ";
    if (result.positions.empty()) {
        cout << "无匹配";
    }
    else {
        for (size_t i = 0; i < result.positions.size(); ++i) {
            cout << result.positions[i];
            if (i + 1 < result.positions.size()) {
                cout << ", ";
            }
        }
    }
    cout << "\n字符比较次数: " << result.characterComparisons;
    cout << "\n耗时: " << fixed << setprecision(3) << result.duration.count() << " ms\n\n";
}

// 单算法模式：让用户选择数据集与算法，展示单次运行的详细指标
inline void runSingleAlgorithm(const vector<DataSet>& dataSets,
    const function<SearchResult(int, const string&, const string&)>& runner) {
    printDataSets(dataSets);
    cout << "请选择数据集: ";
    int dataChoice;
    cin >> dataChoice;

    string text;
    string pattern;

    if (dataChoice >= 1 && dataChoice <= static_cast<int>(dataSets.size())) {
        text = dataSets[dataChoice - 1].text;
        pattern = dataSets[dataChoice - 1].pattern;
    }
    else {
        // 用户选择自定义时，读取完整文本与模式串
        cout << "请输入文本: ";
        cin.ignore();
        getline(cin, text);
        cout << "请输入模式串: ";
        getline(cin, pattern);
    }

    cout << "选择算法:\n1. Brute Force\n2. Rabin-Karp\n3. Sunday\n4. KMP\n5. Boyer-Moore\n";
    int algoChoice;
    cin >> algoChoice;

    // 调用外部传入的算法运行器，保持工具层不直接依赖算法模块
    SearchResult result = runner(algoChoice, text, pattern);
    printResult(result);
}

// 并行对比模式：在同一数据集上并发运行所有算法，便于横向比较
inline void runAllAlgorithmsParallel(
    const vector<DataSet>& dataSets,
    const function<SearchResult(int, const string&, const string&)>& runner) {
    printDataSets(dataSets);
    cout << "请选择数据集: ";
    int dataChoice;
    cin >> dataChoice;

    string text;
    string pattern;

    if (dataChoice >= 1 && dataChoice <= static_cast<int>(dataSets.size())) {
        text = dataSets[dataChoice - 1].text;
        pattern = dataSets[dataChoice - 1].pattern;
    }
    else {
        cout << "请输入文本: ";
        cin.ignore();
        getline(cin, text);
        cout << "请输入模式串: ";
        getline(cin, pattern);
    }

    vector<future<SearchResult>> futures;
    // 循环启动 5 种算法的异步任务，方便并行比较
    for (int i = 1; i <= 5; ++i) {
        futures.emplace_back(async(launch::async, [i, &text, &pattern, &runner]() {
            return runner(i, text, pattern);
            }));
    }

    // 依次获取异步结果并打印，避免乱序输出
    for (auto& f : futures) {
        printResult(f.get());
    }
}

// 展示各算法的时间复杂度、空间复杂度概览，帮助用户理解选择
inline void showComplexityNote() {
    cout << "复杂度概览（平均/最坏）：\n";
    cout << "Brute Force: O(nm) / O(nm), 空间 O(1)\n";
    cout << "Rabin-Karp: O(n+m) / O(nm)（出现哈希冲突时），空间 O(1)\n";
    cout << "KMP: O(n+m) / O(n+m)，空间 O(m)\n";
    cout << "Sunday: O(n) / O(nm)，空间 O(|Σ|)\n";
    cout << "Boyer-Moore: O(n/m) / O(nm)，空间 O(|Σ|+m)\n\n";
}
