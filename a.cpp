#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main() {
    vector<string> a;
    a.push_back("1");
    a.push_back("false");
    a.push_back("true");
    a.push_back("|");
    a.push_back("&");
    sort(a.begin(), a.end());

    for (auto i:a) {
        cout << i << endl;
    }
    return 0;
}