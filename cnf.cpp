#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <typeinfo>
#include <string>
#include "cnf.hpp"
using namespace std;

vector<string> split(const string& str, int delimiter(int) = ::isspace) {
    vector<string> result;
    auto e = str.end();
    auto i = str.begin();
    while (i != e) {
        i = find_if_not(i, e, delimiter);
        if (i == e) break;
        auto j = find_if(i, e, delimiter);
        result.push_back(string(i,j));
        i = j;
    }
    return result;
}

std::string ReplaceAll(std::string &str, const std::string& from, const std::string& to){
    size_t start_pos = 0; //string처음부터 검사
    while((start_pos = str.find(from, start_pos)) != std::string::npos)  //from을 찾을 수 없을 때까지
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
    }
    return str;
}

string minisatFormFixedToString(string str) {
    int start_pos = 0;
    int past_pos = 0;
    int n;
    string replacedString;
    ReplaceAll(str, " 0", "");
    start_pos = str.find("\n", start_pos);
    while(start_pos != string::npos)
    {
        string line;
        line = str.substr(past_pos, start_pos - past_pos);
        start_pos += 1;
        past_pos = start_pos;
        n = count(line.begin(), line.end(), ' ');
        for (int i = 0; i < n; i++) {
            line = "| " + line;
        }
        replacedString = replacedString + line + "\n";
        start_pos = str.find("\n", start_pos);
    }
    n = count(replacedString.begin(), replacedString.end(), '\n');
    for (int i = 0; i < n - 1; i++) {
        replacedString = "& " + replacedString;
    }
    replace(replacedString.begin(), replacedString.end(), '\n', ' ');
    ReplaceAll(replacedString, "-", "- ");

    return replacedString;
}


TreeNode::TreeNode(const string val) {
    value = val;
    parent = NULL;
    left = NULL;
    right = NULL;
    if (val == "-") {
        valueType = 1;
    } else if (val == "&" || val == "|" || val == ">" || val == "<" || val == "=") {
        valueType = 2;
    } else {
        valueType = 0;
    }
}

TreeNode::~TreeNode() {
}

void TreeNode::Free() {
    if (left) {
        left->Free();
        delete left;
        left = NULL;
    }
    if (right) {
        right->Free();
        delete right;
        right = NULL;    
    }
    delete this;
}

CnfTree::CnfTree() {
    root = NULL;
}

CnfTree::~CnfTree() {
    clean(root);
}

void CnfTree::clean(TreeNode* root) {
    if (root->value.size() != 0) {
        clean(root->left);
        clean(root->right);
    } else {
        root->Free();
    }
}

void CnfTree::make_tree(const string expr) {
    vector<string> vals = split(expr);
    for (auto it = vals.rbegin(); it != vals.rend(); it++) {
        string s = *it;
        TreeNode* node = new TreeNode(s);
        if (s[0] == '&' || s[0] == '|' || s[0] == '>' || s[0] == '<' || s[0] == '=') {
            node->left = exp_stack.top();
            node->left->parent = node;
            exp_stack.pop();
            node->right = exp_stack.top();
            node->right->parent = node;
            exp_stack.pop();
            exp_stack.push(node);
        } else if (s[0] == '-') {
            node->left = exp_stack.top();
            node->left->parent = node;
            exp_stack.pop();
            exp_stack.push(node);
        } else {
            exp_stack.push(node);
            if (find(begin(literals), end(literals), node->value) == end(literals)) {
                literals.push_back(node->value);
            }
        }
    }
    root = exp_stack.top();
}

vector<string> CnfTree::get_literals() {
    return literals;
}

int CnfTree::get_max_literal() {
    if (literals[0].find_first_not_of( "0123456789" ) == string::npos) {
        int max = 0;
        for (auto str:literals) {
            if (max < stoi(str)) {
                max = stoi(str);
            }
        }
        return max;
    }
    return get_literals().size();
}

string CnfTree::get_prefix() {
    return get_prefix(root);
}

string CnfTree::get_prefix(TreeNode* node) {
    if (!node) return "";
    string a, b;
    
    if (node->left) {
        if (node->valueType == 2) {
            a = get_prefix(node->left) + " ";
        } else {
            a = get_prefix(node->left);
        }
    } else {
        a = "";
    }

    if (node->right) {
        b = get_prefix(node->right);
    } else {
        b = "";
    }
    if (node->valueType == 0) {
        return node->value;
    } else if (node->valueType == 1) {
        return node->value + " " + a;
    } else {
        return node->value + " " + a + b;
    }
}

string CnfTree::get_postfix() {
    return get_postfix(root);
}

string CnfTree::get_postfix(TreeNode* node) {
    if (!node) return "";
    string a, b;
    
    if (node->left) {
        a = get_postfix(node->left) + " ";
    } else {
        a = "";
    }

    if (node->right) {
        b = get_postfix(node->right) + " ";
    } else {
        b = "";
    }
    return a + b + node->value;
}

string CnfTree::get_infix() {
    return get_infix(root);
}

string CnfTree::get_infix(TreeNode* node) {
    if (!node) return "";
    string a, b;
    
    if (node->left) {
        if (node->valueType == 2) {
            a = get_infix(node->left) + " ";
        } else {
            a = get_infix(node->left);
        }
    } else {
        a = "";
    }

    if (node->right) {
        b = get_infix(node->right);
    } else {
        b = "";
    }
    
    if (node->valueType == 0) {
        return node->value;
    } else if (node->valueType == 1) {
        return node->value + " " + a;
    } else {
        if ((node->parent && node->parent->value == node->value && node->value == "&") ||
        (node->parent && node->parent->value == node->value && node->value == "|") ||
        node == root) {
            return a + node->value + " " + b;
        }
        return "(" + a + node->value + " " + b + ")";
    }
}

string CnfTree::get_minisat_form() {
    CNF_clauses = 0;
    string expression = get_minisat_form(root);
    if (expression != "") {
        CNF_clauses += 1;
    }
    expression += " 0";
    return "p cnf " + to_string(literals.size()) + " " + to_string(CNF_clauses) + "\n" + expression;
}

string CnfTree::get_minisat_form(TreeNode* node) {
    if (!node) return "";
    string a, b;
    if (node->left) {
        a = get_minisat_form(node->left);
    } else {
        a = "";
    }

    if (node->right) {
        b = get_minisat_form(node->right);
    } else {
        b = "";
    }
    
    if (node->valueType == 0) {
        if (!node->value.empty() && find_if(node->value.begin(),
        node->value.end(),
        [](char c) { return !isdigit(c); }) == node->value.end()) {
            // return *find(begin(literals), end(literals), node->value);
            return node->value;
        }
        return node->value;
        // return to_string(distance(begin(literals), find(begin(literals), end(literals), node->value)) + 1);
    } else if (node->valueType == 1) {
        return node->value + a;
    } else if (node->value == "&") {
        CNF_clauses += 1;
        return a + " 0" + "\n" + b;
    } else {
        return a + " " + b;
    }
}

void CnfTree::implFree() {
    root = implFree(root);
}

TreeNode* CnfTree::implFree(TreeNode* node) {
    if (node->valueType == 0) {
        return node;
    } else if (node->value == ">") {
        node->value = "|";
        node->valueType = 2;
        TreeNode* leftNode = new TreeNode("-");
        leftNode->left = implFree(node->left);
        node->left = leftNode;
        node->right = implFree(node->right);
        node->left->parent = node;
        node->right->parent = node;
    } else if (node->value == "-") {
        node->left = implFree(node->left);
        node->left->parent = node;
    } else {
        node->left = implFree(node->left);
        node->right = implFree(node->right);
        node->left->parent = node;
        node->right->parent = node;
    }
    return node;
}

void CnfTree::NNF() {
    root = NNF(root);
}

TreeNode* CnfTree::NNF(TreeNode* node) {
    if (node->valueType == 0) {
        return node;
    } else if (node->value == "-" && node->left->valueType == 0) {
        node->left->parent = node;
        return node;
    } else if (node->value == "-" && node->left->value == "-") {
        TreeNode* temp = node->parent;
        node = NNF(node->left->left);
        node->parent = temp;
    } else if (node->value == "&" || node->value == "|") {
        node->left = NNF(node->left);
        node->right = NNF(node->right);
        node->left->parent = node;
        node->right->parent = node;
    } else if (node->value == "-" && node->left->value == "&") {
        node->value = "|";
        node->valueType = 2;
        TreeNode* leftNode = new TreeNode("-");
        leftNode->left = node->left->left;
        TreeNode* rightNode = new TreeNode("-");
        rightNode->left = node->left->right;
        node->left = leftNode;
        node->right = rightNode;
        node->left->parent = node;
        node->right->parent = node;
        TreeNode* temp = node->parent;
        node = NNF(node);
        node->parent = temp;
    } else if (node->value == "-" && node->left->value == "|") {
        node->value = "&";
        node->valueType = 2;
        TreeNode* leftNode = new TreeNode("-");
        leftNode->left = node->left->left;
        TreeNode* rightNode = new TreeNode("-");
        rightNode->left = node->left->right;
        node->left = leftNode;
        node->right = rightNode;
        node->left->parent = node;
        node->right->parent = node;
        TreeNode* temp = node->parent;
        node = NNF(node);
        node->parent = temp;
    }
    return node;
}

void CnfTree::CNF() {
    root = CNF(root);
    cout << 1 << endl;
    root = compact_tree(root);
    cout << 1 << endl;
}

TreeNode* CnfTree::CNF(TreeNode* node) {
    if (node) {
        if (node->valueType == 0) {
            return node;
        } else if (node->value == "-" && node->left->valueType == 0) {
            node->left->parent = node;
            return node;
        } else if (node->value == "&") {
            node->left = CNF(node->left);
            node->right = CNF(node->right);
            node->left->parent = node;
            node->right->parent = node;
        } else if (node->value == "|") {
            TreeNode* d = distr(CNF(node->left), CNF(node->right));
            node->value = d->value;
            node->valueType = d->valueType;
            node->left = d->left;
            node->right = d->right;
            node->left->parent = node;
            node->right->parent = node;
        }
    }
    return node;
}

TreeNode* CnfTree::distr(TreeNode* node1, TreeNode* node2) {
    TreeNode* temp;
    if (node1->value == "&") {
        temp = new TreeNode("&");
        TreeNode* leftNode = distr(node1->left, node2);
        TreeNode* rightNode = distr(node1->right, node2);
        temp->left = leftNode;
        temp->right = rightNode;
        temp->left->parent = temp;
        temp->right->parent = temp;
    } else if (node2->value == "&") {
        temp = new TreeNode("&");
        TreeNode* leftNode = distr(node1, node2->left);
        TreeNode* rightNode = distr(node1, node2->right);
        temp->left = leftNode;
        temp->right = rightNode;
        temp->left->parent = temp;
        temp->right->parent = temp;
    } else {
        temp = new TreeNode("|");
        temp->left = node1;
        temp->right = node2;
        temp->left->parent = temp;
        temp->right->parent = temp;
    }
    return temp;
}

TreeNode* CnfTree::compact_tree(TreeNode* node) {
    if (node == NULL) return NULL;
    string a, b;
    TreeNode* leftReturn;
    TreeNode* rightReturn;

    if (node->left) {
        leftReturn = compact_tree(node->left);
    }
    if (node->right) {
        rightReturn = compact_tree(node->right);
    }
    if (node == root) {
        return node;
    }
    // if (leftReturn != NULL || rightReturn != NULL) {
    //     return NULL;
    // }
    if (node->left && node->left->value == "true") {
        cout << 2 << "    " << node->value << endl;
        if (node->value == "|") {
            node->value = "true";
            node->valueType = 0;
            node->left->Free();
            node->right->Free();
            cout << 4 << "    " << node->value << endl;
        } else if (node->value == "&") {
            if (node->right->value == "true") {
                node->value = "true";
                node->valueType = 0;
                node->left->Free();
                node->right->Free();
                cout << 5 << "    " << node->value << endl;
            } else {
                node = node->right;
                cout << 3 << "    " << node->value << endl;
            }
        }
        return node;
    } else if (node->right && node->right->value == "true") {
        cout << 2 << "    " << node->value << endl;
        if (node->value == "|") {
            node->value = "true";
            node->valueType = 0;
            node->left->Free();
            node->right->Free();
            cout << 4 << "    " << node->value << endl;
        } else if (node->value == "&") {
            if (node->left->value == "true") {
                node->value = "true";
                node->valueType = 0;
                node->left->Free();
                node->right->Free();
                cout << 5 << "    " << node->value << endl;
            } else {
                node = node->left;
                cout << 3 << "    " << node->value << endl;
            }
        }
        return node;
    } else if (node->left && node->left->value == "false") {
        cout << 2 << "    " << node->value << endl;
        if (node->value == "&") {
            node->value = "false";
            node->valueType = 0;
            node->left->Free();
            node->right->Free();
            cout << 4 << "    " << node->value << endl;
        } else if (node->value == "|") {
            if (node->right->value == "false") {
                node->value = "false";
                node->valueType = 0;
                node->left->Free();
                node->right->Free();
                cout << 5 << "    " << node->value << endl;
            } else {
                node = node->right;
                cout << 3 << "    " << node->value << endl;
            }
        }
        return node;
    } else if (node->right && node->right->value == "false") {
        cout << 2 << "    " << node->value << endl;
        if (node->value == "&") {
            node->value = "false";
            node->valueType = 0;
            node->left->Free();
            node->right->Free();
            cout << 4 << "    " << node->value << endl;
        } else if (node->value == "|") {
            if (node->left->value == "false") {
                node->value = "false";
                node->valueType = 0;
                node->left->Free();
                node->right->Free();
                cout << 5 << "    " << node->value << endl;
            } else {
                node = node->left;
                cout << 3 << "    " << node->value << endl;
            }
        }
        return node;
    } else if (node->value == "|") {
        string left = get_postfix(node->left);
        string right = get_postfix(node->right);
        vector<string> leftString = split(left);
        cout << "left" << left << endl;
        cout << "leftString.size()" << leftString.size() << endl;
        vector<string> rightString = split(right);
        cout << "right" << right << endl;
        cout << "rightString.size()" << rightString.size() << endl;
        if (leftString.size() == 0 && rightString.size() == 0) {
            node->left->Free();
            node->right->Free();
            node = NULL;
            return NULL;
        } else if (leftString.size() == 0) {
            node->value = node->right->value;
            node->valueType = node->right->valueType;
            node->left = node->right->left;
            node->right = node->right->right;            
            return NULL;
        } else if (rightString.size() == 0) {
            node->value = node->left->value;
            node->valueType = node->left->valueType;
            node->left = node->left->left;
            node->right = node->left->right;
            return NULL;
        }
        if (node->parent && find(leftString.begin(), leftString.end(), "&") == leftString.end() &&
        find(rightString.begin(), rightString.end(), "&") == rightString.end()) {
            bool to_break = false;
            for (int i = 0; i < leftString.size(); i++) {
                for (int j = 0; j < rightString.size(); j++) {
                    string l = leftString[i];
                    string r = rightString[j];
                    if (l != "|" && l != "&" && l == r) {
                        if (i > 0 && leftString[i - 1] == "-" && (j == 0 || rightString[j - 1] != "-")) {
                            node->left->Free();
                            node->right->Free();
                            node->value = "true";
                            node->valueType = 0;
                            to_break = true;
                            break;
                        } else if (j > 0 && rightString[j - 1] == "-" && (i == 0 || leftString[i - 1] != "-")) {
                            node->left->Free();
                            node->right->Free();
                            node->value = "true";
                            node->valueType = 0;
                            to_break = true;
                            break;                            
                        }
                    }
                }
                if (to_break) {
                    break;
                }
            }
        }
        // while (node && node->parent && node->value == "true" && node->parent->value == "|") {
        //     node = node->parent;
        //     node->Free();
        //     node->value = "true";
        //     node->valueType = 0;
        // }

        // if (node && node->parent && node->parent->parent && node->value == "true" && node->parent->value == "&") {
        //     if (node->parent->parent->left == node->parent) {
        //         if (node->parent->left == node) {
        //             node->parent->right->parent = node->parent->parent;
        //             node->parent->parent->left = node->parent->right;
        //             node = node->parent;
        //             node->left->Free();
        //             delete node->left;
        //             node->left = NULL;
        //         } else if (node->parent->right == node) {
        //             node->parent->left->parent = node->parent->parent;
        //             node->parent->parent->left = node->parent->left;
        //             node = node->parent;
        //             node->right->Free();
        //             delete node->right;
        //             node->right = NULL;
        //         }
        //     } else if (node->parent->parent->right == node->parent) {
        //         if (node->parent->left == node) {
        //             node->parent->right->parent = node->parent->parent;
        //             node->parent->parent->right = node->parent->right;
        //             node = node->parent;
        //             node->left->Free();
        //             delete node->left;
        //             node->left = NULL;
        //         } else if (node->parent->right == node) {
        //             node->parent->left->parent = node->parent->parent;
        //             node->parent->parent->right = node->parent->left;
        //             node = node->parent;
        //             node->right->Free();
        //             delete node->right;
        //             node->right = NULL;
        //         }
        //     }
        // } else if (node && !node->parent && node->value == "true") {
        //     return NULL;
        // } else if (node && !node->parent->parent && node->value == "true" && node->parent->value == "&") {
        //     if (node->parent->left == node) {
        //         node = node->parent;
        //         node->value = node->right->value;
        //         node->valueType = node->right->valueType;
        //         node->left->Free();
        //         delete node->left;
        //         node->left = node->right->left;
        //         node->right = node->right->right;
        //     } else if (node->parent->right == node) {
        //         node = node->parent;
        //         node->value = node->left->value;
        //         node->valueType = node->left->valueType;
        //         node->right->Free();
        //         delete node->right;
        //         node->left = node->left->left;
        //         node->right = node->left->right;
        //     }
        // }
    } else if (node->value == "&") {
        string left = get_postfix(node->left);
        string right = get_postfix(node->right);
        vector<string> leftString = split(left);
        cout << "left" << left << endl;
        cout << "leftString.size()" << leftString.size() << endl;
        vector<string> rightString = split(right);
        cout << "right" << right << endl;
        cout << "rightString.size()" << rightString.size() << endl;
        if (leftString.size() == 0 && rightString.size() == 0) {
            node->left->Free();
            node->right->Free();
            node = NULL;
            return NULL;
        } else if (leftString.size() == 0) {
            node->value = node->right->value;
            node->valueType = node->right->valueType;
            node->left = node->right->left;
            node->right = node->right->right;            
            return NULL;
        } else if (rightString.size() == 0) {
            node->value = node->left->value;
            node->valueType = node->left->valueType;
            node->left = node->left->left;
            node->right = node->left->right;
            return NULL;
        }
        if (node->parent && find(leftString.begin(), leftString.end(), "|") == leftString.end() &&
        find(rightString.begin(), rightString.end(), "|") == rightString.end()) {
            bool to_break = false;
            for (int i = 0; i < leftString.size(); i++) {
                for (int j = 0; j < rightString.size(); j++) {
                    string l = leftString[i];
                    string r = rightString[j];
                    if (l != "|" && l != "&" && l == r) {
                        cout << 6 << endl;
                        if (i > 0 && leftString[i - 1] == "-" && (j == 0 || rightString[j - 1] != "-")) {
                            node->left->Free();
                            node->right->Free();
                            node->value = "false";
                            node->valueType = 0;
                            to_break = true;
                            break;
                        } else if (j > 0 && rightString[j - 1] == "-" && (i == 0 || leftString[i - 1] != "-")) {
                            node->left->Free();
                            node->right->Free();
                            node->value = "false";
                            node->valueType = 0;
                            to_break = true;
                            break;
                        }
                    }
                }
                if (to_break) {
                    break;
                }
            }
        }
        // while (node && node->parent && node->value == "false" && node->parent->value == "&") {
        //     node = node->parent;
        //     node->Free();
        //     node->value = "false";
        //     node->valueType = 0;
        // }

        // if (node && node->parent && node->parent->parent && node->value == "false" && node->parent->value == "|") {
        //     if (node->parent->parent->left == node->parent) {
        //         if (node->parent->left == node) {
        //             node->parent->right->parent = node->parent->parent;
        //             node->parent->parent->left = node->parent->right;
        //             node = node->parent;
        //             node->left->Free();
        //             delete node->left;
        //             node->left = NULL;
        //         } else if (node->parent->right == node) {
        //             node->parent->left->parent = node->parent->parent;
        //             node->parent->parent->left = node->parent->left;
        //             node = node->parent;
        //             node->right->Free();
        //             delete node->right;
        //             node->right = NULL;
        //         }
        //     } else if (node->parent->parent->right == node->parent) {
        //         if (node->parent->left == node) {
        //             node->parent->right->parent = node->parent->parent;
        //             node->parent->parent->right = node->parent->right;
        //             node = node->parent;
        //             node->left->Free();
        //             delete node->left;
        //             node->left = NULL;
        //         } else if (node->parent->right == node) {
        //             node->parent->left->parent = node->parent->parent;
        //             node->parent->parent->right = node->parent->left;
        //             node = node->parent;
        //             node->right->Free();
        //             delete node->right;
        //             node->right = NULL;
        //         }
        //     }
        // } else if (!node->parent && node->value == "false") {
        //     return NULL;
        // } else if (!node->parent->parent && node->value == "false" && node->parent->value == "&") {
        //     if (node->parent->left == node) {
        //         node = node->parent;
        //         node->value = node->right->value;
        //         node->valueType = node->right->valueType;
        //         node->left->Free();
        //         delete node->left;
        //         node->left = node->right->left;
        //         node->right = node->right->right;
        //     } else if (node->parent->right == node) {
        //         node = node->parent;
        //         node->value = node->left->value;
        //         node->valueType = node->left->valueType;
        //         node->right->Free();
        //         delete node->right;
        //         node->left = node->left->left;
        //         node->right = node->left->right;
        //     }
        // }
    } else {
        return node;
    }
    // cout << "node->value" << node->value << endl;
    // cout << "node->parent->value" << node->parent->value << endl;
    return node;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "error... ./cnf <filename>" << endl;
        return 0;
    }

    string str;
    string filename(argv[1]);
    ifstream inf(filename.c_str());
    getline(inf, str);
    inf.close();

    CnfTree* tree = new CnfTree();
    CnfTree* negationTree = new CnfTree();

    tree->make_tree(str);

    cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    tree->implFree();

    cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    tree->NNF();

    cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    tree->CNF();

    cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    string minisat_form = tree->get_minisat_form();
    vector<string> minisat_form_fixed_vector;
    string minisat_form_fixed;
    string splited;

    auto e = minisat_form.end();
    auto i = minisat_form.begin();
    while (i != e) {
        i = find_if_not(i, e, [](char c) {return c == '\n';});
        if (i == e) break;
        auto j = find_if(i, e, [](char c) {return c == '\n';});
        splited = string(i,j);
        i = j;
        vector<string> splited_vector = split(splited);
        if (splited.c_str()[0] != 'p') {
            sort(splited_vector.begin(), --splited_vector.end());
        }
        splited_vector.erase(unique(splited_vector.begin(), splited_vector.end()), splited_vector.end());
        splited = "";
        for (auto str:splited_vector) {
            splited = splited + str + " ";
        }
        splited = splited.substr(0, splited.length() - 1);
        minisat_form_fixed_vector.push_back(splited);
    }
    sort(++minisat_form_fixed_vector.begin(), minisat_form_fixed_vector.end());
    minisat_form_fixed_vector.erase(unique(minisat_form_fixed_vector.begin(), minisat_form_fixed_vector.end()), minisat_form_fixed_vector.end());
    minisat_form_fixed_vector.erase(minisat_form_fixed_vector.begin());
    vector<vector<string>::iterator> its;
    for (auto it = minisat_form_fixed_vector.begin(); it != minisat_form_fixed_vector.end(); it++) {
        string str = *it;
        stringstream ss;
        str.erase(remove(str.begin(), str.end(), '-'), str.end());
        vector<string> temp = split(str);
        sort(temp.begin(), --temp.end());
        for(int i = 0; i < temp.size(); i++) {
            ss << temp[i];
            if (i != temp.size() - 1) {
                ss << " ";
            }
        }
        str = ss.str();
        ss.str("");
        temp.erase(unique(temp.begin(), temp.end()), temp.end());
        for(int i = 0; i < temp.size(); i++) {
            ss << temp[i];
            if (i != temp.size() - 1) {
                ss << " ";
            }
        }
        if (str != ss.str()) {
            its.push_back(it);
        }
    }
    for (auto it = its.rbegin(); it != its.rend(); it++) {
        minisat_form_fixed_vector.erase(*it);
    }

    vector<int> delete_lines;
    for (int i = 0; i < minisat_form_fixed_vector.size(); i++) {
        for (int j = 0; j < i; j++) {
            vector<string> first = split(minisat_form_fixed_vector[i]);
            vector<string> second = split(minisat_form_fixed_vector[j]);
            int count = 0;
            for (auto k:first) {
                for (auto l:second) {
                    if (k == l) {
                        count++;
                    }
                }
            }
            if (count == first.size()) {
                delete_lines.push_back(j);
            } else if (count == second.size()) {
                delete_lines.push_back(i);
            }
        }
    }
    sort(delete_lines.begin(), delete_lines.end());
    delete_lines.erase(unique(delete_lines.begin(), delete_lines.end()), delete_lines.end());
    
    for (auto i = delete_lines.rbegin(); i != delete_lines.rend(); i++) {
        minisat_form_fixed_vector.erase(minisat_form_fixed_vector.begin() + *i);
    }
    minisat_form_fixed_vector.insert(minisat_form_fixed_vector.begin(), "p cnf " + to_string(tree->get_max_literal()) + " " + to_string(minisat_form_fixed_vector.size()));
    for (auto str:minisat_form_fixed_vector) {
        minisat_form_fixed = minisat_form_fixed + str + "\n";
    }
    minisat_form_fixed = minisat_form_fixed.substr(0, minisat_form_fixed.length() - 1);
    
    // if (str == "SAT") {
    //     cout << "Not Valid" << endl;
    // } else {
    //     cout << "Valid" << endl;
    // }

    // auto first_new_line = minisat_form_fixed.find_first_of("\n");
    // string minisat_form_fixed_tree_form = minisat_form_fixed.substr(first_new_line + 1);
    // minisat_form_fixed_tree_form = minisat_form_fixed_tree_form + "\n";
    // minisat_form_fixed_tree_form = minisatFormFixedToString(minisat_form_fixed_tree_form);
    // tree->make_tree(minisat_form_fixed_tree_form);

    // tree->implFree();
    // tree->NNF();
    // tree->CNF();
    // cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    ofstream outTreeFile("minisatForm.txt");
    outTreeFile << minisat_form_fixed;
    outTreeFile.close();
    // std::system("./minisat minisatForm.txt minisatOut.txt");
    std::system("./minisat minisatForm.txt minisatOut.txt > temptemptemp.temp");
    std::system("rm temptemptemp.temp");

    ifstream inTreeFile("minisatOut.txt");
    getline(inTreeFile, str);
    cout << str << endl;
    inTreeFile.close();

    // minisat_form = tree->get_minisat_form();
    
    // // cout << minisat_form << endl;
    // minisat_form_fixed_vector.clear();
    // minisat_form_fixed.clear();

    // e = minisat_form.end();
    // i = minisat_form.begin();
    // while (i != e) {
    //     i = find_if_not(i, e, [](char c) {return c == '\n';});
    //     if (i == e) break;
    //     auto j = find_if(i, e, [](char c) {return c == '\n';});
    //     splited = string(i,j);
    //     i = j;
    //     vector<string> splited_vector = split(splited);
    //     if (splited.c_str()[0] != 'p') {
    //         sort(splited_vector.begin(), --splited_vector.end());
    //     }
    //     splited_vector.erase(unique(splited_vector.begin(), splited_vector.end()), splited_vector.end());
    //     splited = "";
    //     for (auto str:splited_vector) {
    //         splited = splited + str + " ";
    //     }
    //     splited = splited.substr(0, splited.length() - 1);
    //     minisat_form_fixed_vector.push_back(splited);
    // }
    // sort(++minisat_form_fixed_vector.begin(), minisat_form_fixed_vector.end());
    // minisat_form_fixed_vector.erase(unique(minisat_form_fixed_vector.begin(), minisat_form_fixed_vector.end()), minisat_form_fixed_vector.end());
    // minisat_form_fixed_vector.erase(minisat_form_fixed_vector.begin());
    // for (auto it = minisat_form_fixed_vector.begin(); it != minisat_form_fixed_vector.end(); it++) {
    //     string str = *it;
    //     stringstream ss;
    //     str.erase(remove(str.begin(), str.end(), '-'), str.end());
    //     vector<string> temp = split(str);
    //     sort(temp.begin(), --temp.end());
    //     for(int i = 0; i < temp.size(); i++) {
    //         ss << temp[i];
    //         if (i != temp.size() - 1) {
    //             ss << " ";
    //         }
    //     }
    //     str = ss.str();
    //     ss.str("");
    //     temp.erase(unique(temp.begin(), temp.end()), temp.end());
    //     for(int i = 0; i < temp.size(); i++) {
    //         ss << temp[i];
    //         if (i != temp.size() - 1) {
    //             ss << " ";
    //         }
    //     }
    //     if (str != ss.str()) {
    //         its.push_back(it);
    //     }
    // }
    // for (auto it = its.rbegin(); it != its.rend(); it++) {
    //     minisat_form_fixed_vector.erase(*it);
    // }

    // delete_lines.clear();
    // for (int i = 0; i < minisat_form_fixed_vector.size(); i++) {
    //     for (int j = 0; j < i; j++) {
    //         vector<string> first = split(minisat_form_fixed_vector[i]);
    //         vector<string> second = split(minisat_form_fixed_vector[j]);
    //         int count = 0;
    //         for (auto k:first) {
    //             for (auto l:second) {
    //                 if (k == l) {
    //                     count++;
    //                 }
    //             }
    //         }
    //         if (count == first.size()) {
    //             delete_lines.push_back(j);
    //         } else if (count == second.size()) {
    //             delete_lines.push_back(i);
    //         }
    //     }
    // }
    // sort(delete_lines.begin(), delete_lines.end());
    // delete_lines.erase(unique(delete_lines.begin(), delete_lines.end()), delete_lines.end());
    
    // for (auto i = delete_lines.rbegin(); i != delete_lines.rend(); i++) {
    //     minisat_form_fixed_vector.erase(minisat_form_fixed_vector.begin() + *i);
    // }

    // minisat_form_fixed_vector.insert(minisat_form_fixed_vector.begin(), "p cnf " + to_string(tree->get_max_literal()) + " " + to_string(minisat_form_fixed_vector.size()));
    // for (auto str:minisat_form_fixed_vector) {
    //     minisat_form_fixed = minisat_form_fixed + str + "\n";
    // }
    // minisat_form_fixed = minisat_form_fixed.substr(0, minisat_form_fixed.length() - 1);

    // cout << minisat_form_fixed << endl;

    return 0;
}
