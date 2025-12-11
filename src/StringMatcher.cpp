#include "StringMatcher.h"

#include <algorithm>
#include <unordered_map>

using namespace std;

// 朴素匹配算法：逐位移动窗口，时间复杂度 O(nm)，空间复杂度 O(1)
SearchResult bruteForceSearch(const string& text, const string& pattern) {
    SearchResult result{ "Brute Force" };
    auto start = chrono::high_resolution_clock::now();

    // 边界处理：若模式为空或文本短于模式，则无需匹配
    if (pattern.empty() || text.size() < pattern.size()) {
        result.duration = chrono::high_resolution_clock::now() - start;
        return result;
    }

    // 外层循环滑动窗口，逐位尝试匹配模式串
    for (size_t i = 0; i <= text.size() - pattern.size(); ++i) {
        size_t j = 0;
        // 内层循环逐字符比较，遇到不等立即跳出
        for (; j < pattern.size(); ++j) {
            ++result.characterComparisons;
            if (text[i + j] != pattern[j]) {
                break;
            }
        }
        // 当 j 到达模式长度表示匹配成功，记录起始位置
        if (j == pattern.size()) {
            result.positions.push_back(static_cast<int>(i));
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.duration = end - start;
    return result;
}

// Rabin-Karp 算法：使用滚动哈希加速匹配，平均时间 O(n+m)，最坏 O(nm)
SearchResult rabinKarpSearch(const string& text, const string& pattern, int base, int modulus) {
    SearchResult result{ "Rabin-Karp" };
    auto start = chrono::high_resolution_clock::now();

    // 边界处理：空模式或文本太短直接返回空结果
    if (pattern.empty() || text.size() < pattern.size()) {
        result.duration = chrono::high_resolution_clock::now() - start;
        return result;
    }

    int m = static_cast<int>(pattern.size());
    int n = static_cast<int>(text.size());

    int patternHash = 0;
    int textHash = 0;
    int highestPower = 1; // base^(m-1) % modulus

    // 计算最高位权值，后续用于滚动哈希移除首字符
    for (int i = 0; i < m - 1; ++i) {
        highestPower = (highestPower * base) % modulus;
    }

    // 同步计算模式与首个窗口的哈希值
    for (int i = 0; i < m; ++i) {
        patternHash = (base * patternHash + pattern[i]) % modulus;
        textHash = (base * textHash + text[i]) % modulus;
    }

    // 主循环：滑动窗口并比较哈希，必要时逐字符确认
    for (int i = 0; i <= n - m; ++i) {
        if (patternHash == textHash) {
            bool match = true;
            // 哈希相等时再逐字符比对确认，避免误报
            for (int j = 0; j < m; ++j) {
                ++result.characterComparisons;
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                result.positions.push_back(i);
            }
        }

        if (i < n - m) {
            // 滚动更新哈希：移除旧首字符、加入新字符
            textHash = (base * (textHash - text[i] * highestPower) + text[i + m]) % modulus;
            if (textHash < 0) {
                textHash += modulus;
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.duration = end - start;
    return result;
}

// 构建 KMP 的前缀函数（部分匹配表），记录比较次数方便统计
vector<int> buildKmpPrefix(const string& pattern, size_t& comparisons) {
    vector<int> prefix(pattern.size());
    int length = 0;
    prefix[0] = 0;

    // 遍历模式串，构建最长相等前后缀长度数组
    for (size_t i = 1; i < pattern.size(); ++i) {
        while (length > 0 && pattern[length] != pattern[i]) {
            ++comparisons;
            length = prefix[length - 1];
        }
        ++comparisons;
        if (pattern[length] == pattern[i]) {
            ++length;
        }
        prefix[i] = length;
    }
    return prefix;
}

// KMP 算法：利用前缀函数避免重复比较，时间复杂度 O(n+m)，空间 O(m)
SearchResult kmpSearch(const string& text, const string& pattern) {
    SearchResult result{ "Knuth-Morris-Pratt" };
    auto start = chrono::high_resolution_clock::now();

    // 边界处理：模式为空或过长直接返回
    if (pattern.empty() || text.size() < pattern.size()) {
        result.duration = chrono::high_resolution_clock::now() - start;
        return result;
    }

    size_t prefixComparisons = 0;
    vector<int> prefix = buildKmpPrefix(pattern, prefixComparisons);

    int j = 0;
    // 遍历主串字符，利用前缀表回退避免重复比较
    for (int i = 0; i < static_cast<int>(text.size()); ++i) {
        while (j > 0 && pattern[j] != text[i]) {
            ++result.characterComparisons;
            j = prefix[j - 1];
        }
        ++result.characterComparisons;
        if (pattern[j] == text[i]) {
            ++j;
        }
        // 成功匹配整段模式后记录位置，并按照前缀表回退
        if (j == static_cast<int>(pattern.size())) {
            result.positions.push_back(i - j + 1);
            j = prefix[j - 1];
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.characterComparisons += prefixComparisons;
    result.duration = end - start;
    return result;
}

// 构建 Sunday 算法的位移表：记录字符距离模式串末尾的最右距离
unordered_map<char, int> buildSundayTable(const string& pattern) {
    unordered_map<char, int> table;
    for (int i = 0; i < static_cast<int>(pattern.size()); ++i) {
        table[pattern[i]] = static_cast<int>(pattern.size() - i);
    }
    return table;
}

// Sunday 算法：观察主串窗口后一位，按位移表跳转，均时间 O(n)
SearchResult sundaySearch(const string& text, const string& pattern) {
    SearchResult result{ "Sunday" };
    auto start = chrono::high_resolution_clock::now();

    if (pattern.empty() || text.size() < pattern.size()) {
        result.duration = chrono::high_resolution_clock::now() - start;
        return result;
    }

    unordered_map<char, int> shiftTable = buildSundayTable(pattern);
    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    int i = 0;

    // 循环滑动窗口，若匹配失败依据窗口后一位字符决定跳转步长
    while (i <= n - m) {
        int j = 0;
        while (j < m && text[i + j] == pattern[j]) {
            ++result.characterComparisons;
            ++j;
        }
        if (j == m) {
            result.positions.push_back(i);
        }
        if (i + m >= n) {
            break;
        }
        char next = text[i + m];
        int shift = shiftTable.count(next) ? shiftTable[next] : m + 1;
        ++result.characterComparisons; // mismatch comparison
        i += shift;
    }

    auto end = chrono::high_resolution_clock::now();
    result.duration = end - start;
    return result;
}

const int ASCII_SIZE = 256;

// 构建 Boyer-Moore 的坏字符表：记录字符在模式中的最右位置
void buildBadCharacterTable(const string& pattern, vector<int>& badChar) {
    badChar.assign(ASCII_SIZE, -1);
    for (int i = 0; i < static_cast<int>(pattern.size()); ++i) {
        badChar[static_cast<unsigned char>(pattern[i])] = i;
    }
}

// 构建 Boyer-Moore 的好后缀表：计算不同后缀对应的跳转距离
vector<int> buildGoodSuffixTable(const string& pattern) {
    int m = static_cast<int>(pattern.size());
    vector<int> suffix(m, 0);
    vector<int> prefix(m, 0);

    // 通过反向比较计算每个后缀的起点位置与是否为前缀
    for (int i = 0; i < m - 1; ++i) {
        int j = i;
        int k = 0;
        while (j >= 0 && pattern[j] == pattern[m - 1 - k]) {
            --j;
            ++k;
            suffix[k] = j + 1;
        }
        if (j == -1) {
            prefix[k] = 1;
        }
    }

    vector<int> shift(m, m);
    // 处理完整前缀匹配的情况，更新位移表
    for (int i = 0; i < m; ++i) {
        if (prefix[i]) {
            shift[m - 1 - i] = m - 1 - i;
        }
    }

    // 按照出现的后缀长度调整对应的位置跳转值
    for (int i = 1; i < m; ++i) {
        int slen = suffix[i];
        if (slen != 0) {
            shift[m - 1 - slen] = m - 1 - i;
        }
    }
    return shift;
}

// Boyer-Moore 算法：结合坏字符与好后缀启发式，平均时间近似 O(n/m)
SearchResult boyerMooreSearch(const string& text, const string& pattern) {
    SearchResult result{ "Boyer-Moore" };
    auto start = chrono::high_resolution_clock::now();

    if (pattern.empty() || text.size() < pattern.size()) {
        result.duration = chrono::high_resolution_clock::now() - start;
        return result;
    }

    vector<int> badChar;
    buildBadCharacterTable(pattern, badChar);
    vector<int> goodSuffix = buildGoodSuffixTable(pattern);

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    int i = 0;

    // 从右到左比较窗口与模式，利用两类启发式决定跳转
    while (i <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[i + j]) {
            ++result.characterComparisons;
            --j;
        }
        if (j < 0) {
            result.positions.push_back(i);
            i += goodSuffix[0];
        }
        else {
            ++result.characterComparisons; // mismatch comparison
            int badShift = j - badChar[static_cast<unsigned char>(text[i + j])];
            int goodShift = goodSuffix[j];
            i += max(1, max(badShift, goodShift));
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.duration = end - start;
    return result;
}


// 根据用户选择运行对应的匹配算法，返回统计结果
SearchResult runAlgorithm(int choice, const string& text, const string& pattern) {
    switch (choice) {
    case 1:
        return bruteForceSearch(text, pattern);
    case 2:
        return rabinKarpSearch(text, pattern);
    case 3:
        return sundaySearch(text, pattern);
    case 4:
        return kmpSearch(text, pattern);
    case 5:
        return boyerMooreSearch(text, pattern);
    default:
        return {};
    }
}
