#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <cmath>
#include "nonogram.hpp"

using namespace std;

vector<int> split(const string& str, int delimiter(int) = ::isspace) {
    vector<int> result;
    auto e = str.end();
    auto i = str.begin();
    while (i != e) {
        i = find_if_not(i, e, delimiter);
        if (i == e) break;
        auto j = find_if(i, e, delimiter);
        result.push_back(stoi(string(i,j)));
        i = j;
    }
    return result;
}

void combinationRepeatedList(int* set, int set_size, int n, int m, int index, string* returnValue) {
    if (set_size == m) {
        for (int i = 0; i < m; i++) {
            *returnValue += to_string(set[i]);
        }
        *returnValue += "\n";
        return;
    } 
    if (index == n) {
        return;
    }

    set[set_size] = index;
    combinationRepeatedList(set, set_size + 1, n, m, index, returnValue);
    combinationRepeatedList(set, set_size, n, m, index + 1, returnValue);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "error... ./nonogram <filename>" << endl;
        return 0;
    }

    string line;
    string filename(argv[1]);
    vector< vector<int> > rows;
    vector< vector<int> > columns;
    vector< vector<int> > spaced_rows;
    vector< vector<int> > spaced_columns;
    list<string> Xs_list;
    vector<string> Xs_vector_string;
    string splited;
    string formula;
    string formula_row;
    string formula_column;
    vector<string> clauses;
    vector<int> Xs_vector_int;
    vector<int> Xs_vector_size;
    ifstream inf(filename.c_str());
    getline(inf, line);
    int num_row = stoi(line);
    getline(inf, line);
    int num_column = stoi(line);

    for (int i = 0; i < num_row; i++) {
        getline(inf, line);
        rows.push_back(split(line));
    }
    for (int i = 0; i < num_column; i++) {
        getline(inf, line);
        columns.push_back(split(line));
    }
    inf.close();

    for (auto row:rows) {
        vector<int> vec;
        for (auto i:row) {
            vec.push_back(i + 1);
        }
        vec.back() = vec.back() - 1;
        spaced_rows.push_back(vec);
    }

    for (auto column:columns) {
        vector<int> vec;
        for (auto i:column) {
            vec.push_back(i + 1);
        }
        vec.back() = vec.back() - 1;
        spaced_columns.push_back(vec);
    }

    for (auto row:spaced_rows) {
        int num_space_available = row.size() + 1;
        int num_Xs = num_column;
        for (auto i:row) {
            num_Xs -= i;
        }
        int* set = (int*)malloc(sizeof(int) * num_Xs);
        string Xs = "";
        combinationRepeatedList(set, 0, num_space_available, num_Xs, 0, &Xs);
        Xs_list.push_back(Xs);
        Xs_vector_int.push_back(num_Xs);
    }
    for (auto str:Xs_list) {
        auto e = str.end();
        auto i = str.begin();
        int past_size = Xs_vector_string.size();
        while (i != e) {
            i = find_if_not(i, e, [](char c) {return c == '\n';});
            if (i == e) break;
            auto j = find_if(i, e, [](char c) {return c == '\n';});
            splited = string(i,j);
            i = j;
            Xs_vector_string.push_back(splited);
        }
        Xs_vector_size.push_back(Xs_vector_string.size() - past_size);
    }
    vector<string> lines;
    vector<long long int> combination_repeated_nums;
    for (int i = 0, j = 0; i < spaced_rows.size(); i++) {
        vector<int> row = spaced_rows[i];
        vector<vector<int>> checked_literals;
        int base = i * num_column;
        combination_repeated_nums.push_back(Xs_vector_size[i]);
        for (long long int num = 0; num < Xs_vector_size[i]; num++) {
            int current_cursor = 1 + base;
            string Xs = Xs_vector_string[j];
            for (int k = 0; k < row.size(); k++) {
                for (int l = 0; l < Xs_vector_int[i]; l++) {
                    if ((int)(Xs.c_str()[l]) - 48 > k) {
                        break;
                    }
                    if ((int)(Xs.c_str()[l]) - 48 < k) {
                        continue;
                    }
                    current_cursor++;
                }
                if (k == row.size() - 1) {
                    if (checked_literals.size() == num) {
                        checked_literals.push_back(vector<int>());
                        for (int l = 0; l < row[k]; l++) {
                            checked_literals[num].push_back(current_cursor);
                            current_cursor++;
                        }
                    } else {
                        for (int l = 0; l < row[k]; l++) {
                            checked_literals[num].push_back(current_cursor);
                            current_cursor++;
                        }
                    }
                } else {
                    if (checked_literals.size() == num) {
                        checked_literals.push_back(vector<int>());
                        for (int l = 0; l < row[k]; l++) {
                            if (!(rows[i][k] == row[k] && l == row[k]) && !(rows[i][k] != row[k] && l == row[k] - 1)) {
                                checked_literals[num].push_back(current_cursor);
                            }
                            current_cursor++;
                        }
                    } else {
                        for (int l = 0; l < row[k]; l++) {
                            if (!(rows[i][k] == row[k] && l == row[k]) && !(rows[i][k] != row[k] && l == row[k] - 1)) {
                                checked_literals[num].push_back(current_cursor);
                            }
                            current_cursor++;
                        }
                    }
                }
            }
            j++;
        }
        string line;
        for (int k = 0; k < checked_literals.size(); k++) {
            for (int l = 0; l < num_column - 1; l++) {
                line += "& ";
            }
            for (int m = 1; m <= num_column; m++) {
                int cursor = m + i * num_column;
                if (find(checked_literals[k].begin(), checked_literals[k].end(), cursor) != checked_literals[k].end()) {
                    line += to_string(cursor) + " ";
                } else {
                    line += "- " + to_string(cursor) + " ";
                }
            }
        }
        lines.push_back(line);
    }
    for (int k = 0; k < rows.size(); k++) {
        clauses.push_back("");
        for (int l = 0; l < combination_repeated_nums[k] - 1; l++) {
            clauses[k] += "| ";
        }
        clauses[k] += lines[k].substr(0, lines[k].length() - 1);
    }
    for (int i = 0; i < clauses.size(); i++) {
        if (i == 0) {
            formula_row = clauses[i];
        } else {
            formula_row = "& " + formula_row + " " + clauses[i];
        }
    }

    Xs_list.clear();
    clauses.clear();
    lines.clear();
    combination_repeated_nums.clear();
    Xs_vector_int.clear();
    Xs_vector_string.clear();
    Xs_vector_size.clear();

    for (auto column:spaced_columns) {
        int num_space_available = column.size() + 1;
        int num_Xs = rows.size();
        for (auto i:column) {
            num_Xs -= i;
        }
        int* set = (int*)malloc(sizeof(int) * num_Xs);
        string Xs = "";
        combinationRepeatedList(set, 0, num_space_available, num_Xs, 0, &Xs);
        Xs_list.push_back(Xs);
        Xs_vector_int.push_back(num_Xs);
    }
    
    for (auto str:Xs_list) {
        auto e = str.end();
        auto i = str.begin();
        int past_size = Xs_vector_string.size();
        while (i != e) {
            i = find_if_not(i, e, [](char c) {return c == '\n';});
            if (i == e) break;
            auto j = find_if(i, e, [](char c) {return c == '\n';});
            splited = string(i,j);
            i = j;
            Xs_vector_string.push_back(splited);
        }
        Xs_vector_size.push_back(Xs_vector_string.size() - past_size);
    }
    for (int i = 0, j = 0; i < spaced_columns.size(); i++) {
        vector<int> column = spaced_columns[i];
        vector<vector<int>> checked_literals;
        int base = i;
        combination_repeated_nums.push_back(Xs_vector_size[i]);
        for (long long int num = 0; num < Xs_vector_size[i]; num++) {
            int current_cursor = 1 + i;
            string Xs = Xs_vector_string[j];
            for (int k = 0; k < column.size(); k++) {
                for (int l = 0; l < Xs_vector_int[i]; l++) {
                    if ((int)(Xs.c_str()[l]) - 48 > k) {
                        break;
                    }
                    if ((int)(Xs.c_str()[l]) - 48 < k) {
                        continue;
                    }
                    current_cursor += num_column;
                }
                if (k == column.size() - 1) {
                    if (checked_literals.size() == num) {
                        checked_literals.push_back(vector<int>());
                        for (int l = 0; l < column[k]; l++) {
                            checked_literals[num].push_back(current_cursor);
                            current_cursor += num_column;                                
                        }
                    } else {
                        for (int l = 0; l < column[k]; l++) {
                            checked_literals[num].push_back(current_cursor);
                            current_cursor += num_column;
                        }
                    }
                } else {
                    if (checked_literals.size() == num) {
                        checked_literals.push_back(vector<int>());
                        for (int l = 0; l < column[k]; l++) {
                            if (!(columns[i][k] == column[k] && l == column[k]) && !(columns[i][k] != column[k] && l == column[k] - 1)) {
                                checked_literals[num].push_back(current_cursor);
                            }
                            current_cursor += num_column;
                        }
                    } else {
                        for (int l = 0; l < column[k]; l++) {
                            if (!(columns[i][k] == column[k] && l == column[k]) && !(columns[i][k] != column[k] && l == column[k] - 1)) {
                                checked_literals[num].push_back(current_cursor);
                            }
                            current_cursor += num_column;
                        }
                    }
                }
            }
            j++;
        }
        string line;
        for (int k = 0; k < checked_literals.size(); k++) {
            for (int l = 0; l < rows.size() - 1; l++) {
                line += "& ";
            }
            for (int m = 0; m < rows.size(); m++) {
                int cursor = i + 1 + m * rows.size();
                if (find(checked_literals[k].begin(), checked_literals[k].end(), cursor) != checked_literals[k].end()) {
                    line += to_string(cursor) + " ";
                } else {
                    line += "- " + to_string(cursor) + " ";
                }
            }
        }
        lines.push_back(line);        
    }
    for (int k = 0; k < num_column; k++) {
        clauses.push_back("");
        for (int l = 0; l < combination_repeated_nums[k] - 1; l++) {
            clauses[k] += "| ";
        }
        clauses[k] += lines[k].substr(0, lines[k].length() - 1);
    }
    for (int i = 0; i < clauses.size(); i++) {
        if (i == 0) {
            formula_column = clauses[i];
        } else {
            formula_column = "& " + formula_column + " " + clauses[i];
        }
    }

    formula = "& " + formula_row + " " + formula_column;
    ofstream outf("nonogram_parse.txt");    
    outf << formula << endl;
    outf.close();


    return 0;
}