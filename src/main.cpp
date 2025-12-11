#include "StringMatcher.h"
#include "Types.h"
#include "Utils.h"

#include <iostream>

using namespace std;

// 程序入口：展示复杂度提示并提供交互菜单
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<DataSet> dataSets = buildDataSets();
    showComplexityNote();

    // 主循环处理用户菜单，按输入选择不同功能
    while (true) {
        cout << "1. 选择单一算法运行\n2. 并行比较所有算法\n3. 退出\n请输入选项: ";
        int choice;
        if (!(cin >> choice)) {
            break;
        }
        if (choice == 1) {
            // 传入算法运行器，确保工具层与算法层解耦
            runSingleAlgorithm(dataSets, runAlgorithm);
        }
        else if (choice == 2) {
            // 并行模式同样通过回调调用算法层，保持单向依赖
            runAllAlgorithmsParallel(dataSets, runAlgorithm);
        }
        else {
            break;
        }
    }

    return 0;
}
