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

vector<string> split(const string str, int delimiter(int) = ::isspace) {
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

std::string ReplaceAll(string str, const string from, const string to){
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
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
    parent;
    left = NULL;
    right = NULL;
    is_compact = false;
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

void reset(shared_ptr<TreeNode>& node) {
    // cout << node.use_count() << " " << node->value << endl;
    if (!node) return;
    if (node->left) {
        reset(node->left);
    }
    if (node->right) {
        reset(node->right);
    }
    if (node.use_count() > 1) node.reset();
}

// void TreeNode::Free() {
//     if (left) {
//         left->Free();
//         delete left;
//         left = NULL;
//     }
//     if (right) {
//         right->Free();
//         delete right;
//         right = NULL;    
//     }
// }

CnfTree::CnfTree() {
    root = NULL;
}

CnfTree::~CnfTree() {
    root = NULL;
}

void CnfTree::make_tree(const string expr) {
    vector<string> vals = split(expr);
    for (auto it = vals.rbegin(); it != vals.rend(); it++) {
        string s = *it;
        shared_ptr<TreeNode> node(new TreeNode(s));
        if (s[0] == '&' || s[0] == '|' || s[0] == '>' || s[0] == '<' || s[0] == '=') {
            node->left = exp_stack.top();
            exp_stack.pop();
            node->right = exp_stack.top();
            exp_stack.pop();
            exp_stack.push(node);
        } else if (s[0] == '-') {
            node->left = exp_stack.top();
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
    while(!exp_stack.empty()) exp_stack.pop();
}

shared_ptr<TreeNode> CnfTree::make_sub_tree(const string expr) {
    vector<string> vals = split(expr);
    for (auto it = vals.rbegin(); it != vals.rend(); it++) {
        string s = *it;
        shared_ptr<TreeNode> node = make_shared<TreeNode>(s);
        if (s[0] == '&' || s[0] == '|' || s[0] == '>' || s[0] == '<' || s[0] == '=') {
            node->left = exp_stack.top();
            exp_stack.pop();
            node->right = exp_stack.top();
            exp_stack.pop();
            exp_stack.push(node);
        } else if (s[0] == '-') {
            node->left = exp_stack.top();
            exp_stack.pop();
            exp_stack.push(node);
        } else {
            exp_stack.push(node);
            if (find(begin(literals), end(literals), node->value) == end(literals)) {
                literals.push_back(node->value);
            }
        }
    }
    return exp_stack.top();
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

void CnfTree::check_parent() {
    int c0 = 0;
    int c1 = 0;
    check_parent(root, &c0, &c1);
    cout << "c0" << c0 << endl;
    cout << "c1" << c1 << endl;
}

void CnfTree::check_parent(shared_ptr<TreeNode> node, int* count0, int* count1) {
    if (node->left) {
        shared_ptr<TreeNode> temp = node->left->parent.lock();
        if (temp == node) {
            *count1 += 1;
        } else {
            *count0 += 1;
        }
        // cout << node->left->parent->num << "  " << node->num << endl;
    }
    if (node->right) {
        shared_ptr<TreeNode> temp = node->right->parent.lock();
        if (temp == node) {
            *count1 += 1;
        } else {
            *count0 += 1;
        }
        // cout << node->right->parent->num << "  " << node->num << endl;
    }
    if (node->left) {
        check_parent(node->left, count0, count1);
    }
    if (node->right) {
        check_parent(node->right, count0, count1);
    }
}

void CnfTree::set_parent(shared_ptr<TreeNode> node) {
    if (node->left) {
        node->left->parent = node;
    }
    if (node->right) {
        node->right->parent = node;
    }
    if (node->left) {
        set_parent(node->left);        
    }
    if (node->right) {
        set_parent(node->right);        
    }
}

string CnfTree::get_prefix() {
    return get_prefix(root);
}

string CnfTree::get_prefix(shared_ptr<TreeNode> node) {
    if (node.use_count() == 0) return "";
    // cout << node.use_count() << " " << node->value << endl;
    string a, b;
    
    if (node->valueType == 2) {
        a = get_prefix(node->left) + " ";
        b = get_prefix(node->right);
    } else if (node->valueType == 1) {
        a = get_prefix(node->left);
    }

    if (node->valueType == 0) {
        return node->value;
    } else {
        return node->value + " " + a + b;
    }
}

string CnfTree::get_postfix() {
    return get_postfix(root);
}

string CnfTree::get_postfix(shared_ptr<TreeNode> node) {
    if (node.use_count() == 0) return "";
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

string CnfTree::get_infix(shared_ptr<TreeNode> node) {
    if (node.use_count() == 0) return "";
    string a, b;

    
    if (node->valueType == 2) {
        a = get_infix(node->left) + " ";
        b = get_infix(node->right);
    } else if (node->valueType == 1) {
        a = get_infix(node->left);
    }
    
    if (node->valueType == 0) {
        return node->value;
    } else if (node->valueType == 1) {
        return node->value + " " + a;
    } else {
        shared_ptr<TreeNode> temp = node->parent.lock();        
        if ((temp && temp->value == node->value && node->value == "&") ||
        (temp && temp->value == node->value && node->value == "|") ||
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

string CnfTree::get_minisat_form(shared_ptr<TreeNode> node) {
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
            return node->value;
        }
        return node->value;

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
    set_parent(root);
    // check_parent();
}

shared_ptr<TreeNode> CnfTree::implFree(shared_ptr<TreeNode> node) {
    if (node->valueType == 0) {
        return node;
    } else if (node->value == ">") {
        node->value = "|";
        node->valueType = 2;
        shared_ptr<TreeNode> leftNode = make_shared<TreeNode>("-");
        leftNode->left = implFree(node->left);
        node->left = leftNode;
        node->right = implFree(node->right);
    } else if (node->value == "-") {
        node->left = implFree(node->left);
    } else {
        node->left = implFree(node->left);
        node->right = implFree(node->right);
    }
    return node;
}

void CnfTree::NNF() {
    root = NNF(root);
    set_parent(root);
    // check_parent();
}

shared_ptr<TreeNode> CnfTree::NNF(shared_ptr<TreeNode> node) {
    if (node->valueType == 0) {
        return node;
    } else if (node->value == "-" && node->left->valueType == 0) {
        return node;
    } else if (node->value == "-" && node->left->value == "-") {
        node = NNF(node->left->left);
    } else if (node->value == "&" || node->value == "|") {
        node->left = NNF(node->left);
        node->right = NNF(node->right);
    } else if (node->value == "-" && node->left->value == "&") {
        node->value = "|";
        node->valueType = 2;
        shared_ptr<TreeNode> leftNode = make_shared<TreeNode>("-");
        leftNode->left = node->left->left;
        shared_ptr<TreeNode> rightNode = make_shared<TreeNode>("-");
        rightNode->left = node->left->right;
        node->left = leftNode;
        node->right = rightNode;
        node = NNF(node);
    } else if (node->value == "-" && node->left->value == "|") {
        node->value = "&";
        node->valueType = 2;
        shared_ptr<TreeNode> leftNode = make_shared<TreeNode>("-");
        leftNode->left = node->left->left;
        shared_ptr<TreeNode> rightNode = make_shared<TreeNode>("-");
        rightNode->left = node->left->right;
        node->left = leftNode;
        node->right = rightNode;
        node = NNF(node);
    }
    return node;
}

void CnfTree::CNF() {
    root = CNF(root);
    cout << 1111 << endl;
    compact_tree(root);
    set_parent(root);
    cout << 1111 << endl;
}

shared_ptr<TreeNode> CnfTree::CNF(shared_ptr<TreeNode> node) {
    if (node) {
        if (node->valueType == 0) {
            return node;
        } else if (node->value == "-" && node->left->valueType == 0) {
            return node;
        } else if (node->value == "&") {
            node->left = CNF(node->left);
            node->right = CNF(node->right);
        } else if (node->value == "|") {
            shared_ptr<TreeNode> leftVar = CNF(node->left);
            // compact_tree(leftVar);
            shared_ptr<TreeNode> rightVar = CNF(node->right);
            // compact_tree(rightVar);
            shared_ptr<TreeNode> d = distr(leftVar, rightVar);
            node = d;
            compact_tree(node);
        }
    }
    return node;
}

shared_ptr<TreeNode> CnfTree::distr(shared_ptr<TreeNode> node1, shared_ptr<TreeNode> node2) {
    shared_ptr<TreeNode> returnValue;
    if (node1->value == "&") {
        returnValue = make_shared<TreeNode>("&");
        shared_ptr<TreeNode> leftNode(distr(node1->left, node2));
        shared_ptr<TreeNode> rightNode(distr(node1->right, node2));
        returnValue->left = leftNode;
        returnValue->right = rightNode;
    } else if (node2->value == "&") {
        returnValue = make_shared<TreeNode>("&");
        shared_ptr<TreeNode> leftNode(distr(node1, node2->left));
        shared_ptr<TreeNode> rightNode(distr(node1, node2->right));
        returnValue->left = leftNode;
        returnValue->right = rightNode;
    } else {
        returnValue = make_shared<TreeNode>("|");
        returnValue->left = node1;
        returnValue->right = node2;
    }

    return returnValue;
}

void CnfTree::compact_tree(shared_ptr<TreeNode>& node) {
    if (node == NULL) {
        node = NULL;
        return;
    }
    if (node->valueType) {
        compact_tree(node->left);
        // cout << node->left->value << endl;
        // cout << node->value << endl;
    }
    if (node->valueType == 2) {
        compact_tree(node->right);
        // cout << node->right->value << endl;
        // cout << node->value << endl;
    }
    if (node->valueType == 2) {
        if (node->left == NULL && node->right == NULL) {
            node = NULL;
            return;
        } else if (node->left == NULL) {
            node = node->right;
            return;
        } else if (node->right == NULL) {
            node = node->left;
            return;
        }
    } else if (node->valueType == 1) {
        if (node->left == NULL) {
            node = NULL;
            return;
        }
    }
    if (node->valueType && node->left->value == "true") {
        if (node->value == "|") {
            reset(node->left);
            reset(node->right);
            node->value = "true";
            node->valueType = 0;
        } else if (node->value == "&") {
            if (node->right->value == "true") {
                reset(node->left);
                reset(node->right);
                node->value = "true";
                node->valueType = 0;
            } else {
                reset(node->left);
                node = node->right;
            }
        } else if (node->value == "-") {
            reset(node->left);
            node->value = "false";
            node->valueType = 0;
        }
    } else if (node->valueType == 2 && node->right->value == "true") {
        if (node->value == "|") {
            reset(node->left);
            reset(node->right);
            node->value = "true";
            node->valueType = 0;
        } else if (node->value == "&") {
            reset(node->right);
            node = node->left;
        }
    } else if (node->valueType && node->left->value == "false") {
        if (node->value == "&") {
            reset(node->left);
            reset(node->right);
            node->value = "false";
            node->valueType = 0;
        } else if (node->value == "|") {
            if (node->right->value == "false") {
                reset(node->left);
                reset(node->right);
                node->value = "false";
                node->valueType = 0;
            } else {
                reset(node->left);
                node = node->right;
            }
        } else if (node->value == "-") {
            reset(node->left);
            node->value = "true";
            node->valueType = 0;
        }
    } else if (node->valueType == 2 && node->right->value == "false") {
        if (node->value == "&") {
            reset(node->left);
            reset(node->right);
            node->value = "false";
            node->valueType = 0;
        } else if (node->value == "|") {
            reset(node->right);
            node = node->left;
        }
    }

    if (node->value == "|") {
        string left = get_prefix(node->left);
        string right = get_prefix(node->right);
        vector<string> leftString = split(left);
        vector<string> rightString = split(right);
        if (find(leftString.begin(), leftString.end(), "&") == leftString.end() &&
        find(rightString.begin(), rightString.end(), "&") == rightString.end()) {
            bool to_break = false;
            for (int i = 0; i < leftString.size(); i++) {
                for (int j = 0; j < rightString.size(); j++) {
                    string l = leftString[i];
                    string r = rightString[j];
                    if (l == "|") break;
                    if (r == "|") continue;
                    if (l == r && l != "-") {
                        if (i > 0 && leftString[i - 1] == "-" && (j == 0 || rightString[j - 1] != "-")) {
                            reset(node->left);
                            reset(node->right);
                            node->value = "true";
                            node->valueType = 0;
                            to_break = true;
                            break;
                        } else if (j > 0 && rightString[j - 1] == "-" && (i == 0 || leftString[i - 1] != "-")) {
                            reset(node->left);
                            reset(node->right);
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
            if (!to_break) {
                string negationLeft = ReplaceAll(left, "- ", "-");
                string negationRight = ReplaceAll(right, "- ", "-");
                vector<string> negationLeftString = split(negationLeft);
                vector<string> negationRightString = split(negationRight);
                negationLeftString.erase(unique(negationLeftString.begin(), negationLeftString.end()), negationLeftString.end());
                negationRightString.erase(unique(negationRightString.begin(), negationRightString.end()), negationRightString.end());
                sort(negationLeftString.begin(), negationLeftString.end());
                sort(negationRightString.begin(), negationRightString.end());
                negationLeft = "";
                while (true) {
                    auto it = negationLeftString.rbegin();
                    if (*it != "|") break;
                    negationLeftString.pop_back();
                }
                for (int i = 0; i < (int)negationLeftString.size() - 1; i++) {
                    negationLeft += "| ";
                }
                for (auto str:negationLeftString) {
                    negationLeft += str + " ";
                }
                negationLeft = negationLeft.substr(0, negationLeft.length() - 1);
                // cout << "negationLeft  " << negationLeft << endl;
                
                negationRight = "";
                while (true) {
                    auto it = negationRightString.rbegin();
                    if (*it != "|") break;
                    negationRightString.pop_back();
                }
                for (int i = 0; i < (int)negationRightString.size() - 1; i++) {
                    negationRight += "| ";
                }
                for (auto str:negationRightString) {
                    negationRight += str + " ";
                }
                negationRight = negationRight.substr(0, negationRight.length() - 1);
                // cout << "negationRight " << negationRight << endl;
                // if (negationLeftString == negationRightString) {
                //     reset(node);
                //     node = make_sub_tree(ReplaceAll(negationLeft, "-", "- "));
                //     while(!exp_stack.empty()) exp_stack.pop();
                int count = 0;
                for (auto a:negationLeftString) {
                    for (auto b:negationRightString) {
                        if (a == b) {
                            count++;
                        }
                    }
                }
                if (count == negationLeftString.size()) {
                    reset(node);
                    node = make_sub_tree(ReplaceAll(negationRight, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                } else if (count == negationRightString.size()) {
                    reset(node);
                    node = make_sub_tree(ReplaceAll(negationLeft, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                } else {
                    // cout << "node->left->value" << node->left->value << endl;
                    reset(node->left);
                    // cout << "node->right->value" << node->right->value << endl;
                    reset(node->right);
                    node->left = make_sub_tree(ReplaceAll(negationLeft, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                    node->right = make_sub_tree(ReplaceAll(negationRight, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                }
            }
        }
    } else if (node->value == "&") {
        string left = get_prefix(node->left);
        string right = get_prefix(node->right);
        vector<string> leftString = split(left);
        vector<string> rightString = split(right);
        if (find(leftString.begin(), leftString.end(), "|") == leftString.end() &&
        find(rightString.begin(), rightString.end(), "|") == rightString.end()) {
            bool to_break = false;
            for (int i = 0; i < leftString.size(); i++) {
                for (int j = 0; j < rightString.size(); j++) {
                    string l = leftString[i];
                    string r = rightString[j];
                    if (l == "&") break;
                    if (r == "&") continue;
                    if (l == r && l != "-") {
                        if (i > 0 && leftString[i - 1] == "-" && (j == 0 || rightString[j - 1] != "-")) {
                            reset(node->left);
                            reset(node->right);
                            node->value = "false";
                            node->valueType = 0;
                            to_break = true;
                            break;
                        } else if (j > 0 && rightString[j - 1] == "-" && (i == 0 || leftString[i - 1] != "-")) {
                            reset(node->left);
                            reset(node->right);
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
            if (!to_break) {
                string negationLeft = ReplaceAll(left, "- ", "-");
                string negationRight = ReplaceAll(right, "- ", "-");
                vector<string> negationLeftString = split(negationLeft);
                vector<string> negationRightString = split(negationRight);
                negationLeftString.erase(unique(negationLeftString.begin(), negationLeftString.end()), negationLeftString.end());
                negationRightString.erase(unique(negationRightString.begin(), negationRightString.end()), negationRightString.end());
                sort(negationLeftString.begin(), negationLeftString.end());
                sort(negationRightString.begin(), negationRightString.end());
                negationLeft = "";
                while (true) {
                    auto it = negationLeftString.begin();
                    if (*it != "&") break;
                    negationLeftString.erase(negationLeftString.begin());
                }
                for (int i = 0; i < (int)negationLeftString.size() - 1; i++) {
                    negationLeft += "& ";
                }
                for (auto str:negationLeftString) {
                    negationLeft += str + " ";
                }
                negationLeft = negationLeft.substr(0, negationLeft.length() - 1);
                // cout << "negationLeft  " << negationLeft << endl;
                
                negationRight = "";
                while (true) {
                    auto it = negationRightString.begin();
                    if (*it != "&") break;
                    negationRightString.erase(negationRightString.begin());
                }
                for (int i = 0; i < (int)negationRightString.size() - 1; i++) {
                    negationRight += "& ";
                }
                for (auto str:negationRightString) {
                    negationRight += str + " ";
                }
                negationRight = negationRight.substr(0, negationRight.length() - 1);
                // cout << "negationRight " << negationRight << endl;
                // if (negationLeftString == negationRightString) {
                //     reset(node);
                //     node = make_sub_tree(ReplaceAll(negationLeft, "-", "- "));
                //     while(!exp_stack.empty()) exp_stack.pop();
                int count = 0;
                for (auto a:negationLeftString) {
                    for (auto b:negationRightString) {
                        if (a == b) {
                            count++;
                        }
                    }
                }
                if (count == negationLeftString.size()) {
                    reset(node);
                    node = make_sub_tree(ReplaceAll(negationRight, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                } else if (count == negationRightString.size()) {
                    reset(node);
                    node = make_sub_tree(ReplaceAll(negationLeft, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                } else {
                    // cout << "node->left->value" << node->left->value << endl;
                    reset(node->left);
                    // cout << "node->right->value" << node->right->value << endl;
                    reset(node->right);
                    node->left = make_sub_tree(ReplaceAll(negationLeft, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                    node->right = make_sub_tree(ReplaceAll(negationRight, "-", "- "));
                    while(!exp_stack.empty()) exp_stack.pop();
                }
            }
        }
    }
    node->is_compact = true;
    // cout << node->value << endl;
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

    tree->implFree();

    cout << tree->get_prefix() << endl;

    tree->NNF();

    cout << tree->get_prefix() << endl;

    tree->CNF();

    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;

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
        ofstream outTreeFile("minisatForm.txt");
    outTreeFile << minisat_form_fixed;
    outTreeFile.close();

    system("./minisat minisatForm.txt minisatOut.txt > temptemptemp.temp");
    system("rm temptemptemp.temp");

    ifstream inTreeFile("minisatOut.txt");
    getline(inTreeFile, str);
    cout << str << endl;
    inTreeFile.close();    
    
    return 0;
}
