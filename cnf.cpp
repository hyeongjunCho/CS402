#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
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

TreeNode::TreeNode(const string val) {
    value = val;
    left = NULL;
    right = NULL;
}

CnfTree::~CnfTree() {
    clean(root);
}

void CnfTree::clean(TreeNode* root) {
    if (root->value.size() != 0) {
        clean(root->left);
        clean(root->right);
    } else {
        delete root;
    }
}

void CnfTree::make_tree(const string expr) {
    vector<string> vals = split(expr);
    
    for (auto s:vals) {
        if (s[0] == '&' || s[0] == '|' || s[0] == '>' || s[0] == '<' || s[0] == '=') {
            // binary operator
            TreeNode* op = new TreeNode(s);
            op->valueType = 2;
            if (op_stack.empty()) {
                root = op;
            } else {
                *op_stack.top() = op;
                op_stack.pop();
            }
            op_stack.push(&op->right);
            op_stack.push(&op->left);
        } else if (s[0] == '-') {
            // unary operator
            TreeNode* op = new TreeNode(s);
            op->valueType = 1;
            if (op_stack.empty()) {
                root = op;
            } else {
                *op_stack.top() = op;
                op_stack.pop();
            }
            op_stack.push(&op->left);
        } else {
            // literal
            TreeNode* l = new TreeNode(s);
            l->valueType = 0;
            if (op_stack.empty()) {
                root = l;
            } else {
                *op_stack.top() = l;
                op_stack.pop();
            }
        }
    }
}

string CnfTree::get_prefix() {
    return get_prefix(root);
}

string CnfTree::get_prefix(TreeNode* root) {
    if (root == nullptr) return "";
    string a, b;
    
    if (root->left) {
        if (root->valueType == 2) {
            a = get_prefix(root->left) + " ";
        } else {
            a = get_prefix(root->left);
        }
    } else {
        a = "";
    }

    if (root->right) {
        b = get_prefix(root->right);
    } else {
        b = "";
    }
    if (root->valueType != 1) {
        if (root->valueType == 0) {
            return root->value;
        } else {
            return a + root->value + " " + b;
        }
    } else {
        return root->value + " " + a;
    }
}

string CnfTree::get_postfix() {
    return get_postfix(root);
}

string CnfTree::get_postfix(TreeNode* root) {
    if (root == nullptr) return "";
    string a, b;
    
    if (root->left) {
        a = get_postfix(root->left) + " ";
    } else {
        a = "";
    }

    if (root->right) {
        b = get_postfix(root->right) + " ";
    } else {
        b = "";
    }
    return a + b + root->value;
}

string CnfTree::get_infix() {
    return get_infix(root);
}

string CnfTree::get_infix(TreeNode* root) {
    if (root == nullptr) return "";
    string a, b;
    
    if (root->left) {
        if (root->valueType == 2) {
            a = get_infix(root->left) + " ";
        } else {
            a = get_infix(root->left);
        }
    } else {
        a = "";
    }

    if (root->right) {
        b = get_infix(root->right);
    } else {
        b = "";
    }
    if (root->valueType != 1) {
        if (root->valueType == 0) {
            return root->value;
        } else {
            return "(" + a + root->value + " " + b + ")";
        }
    } else {
        return root->value + " " + a;
    }
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

    cout << str << endl;

    CnfTree* tree = new CnfTree();
    tree->make_tree(str);
    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;
    cout << tree->get_postfix() << endl;

    return 0;
}
