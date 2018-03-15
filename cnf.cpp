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
    if (val == "-") {
        valueType = 1;
    } else if (val == "&" || val == "|" || val == ">" || val == "<" || val == "=") {
        valueType = 2;
    } else {
        valueType = 0;
    }
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
    if (root->valueType == 0) {
        return root->value;
    } else if (root->valueType == 1) {
        return root->value + " " + a;
    } else {
        return root->value + " " + a + b;
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
    
    if (root->valueType == 0) {
        return root->value;
    } else if (root->valueType == 1) {
        return root->value + " " + a;
    } else {
        return "(" + a + root->value + " " + b + ")";
    }
}

void CnfTree::implFree() {
    root = implFree(root);
}

TreeNode* CnfTree::implFree(TreeNode* root) {
    if (root->valueType == 0) {
        return root;
    } else if (root->value == ">") {
        root->value = "|";
        root->valueType = 2;
        TreeNode* leftNode = new TreeNode("-");
        leftNode->left = implFree(root->left);
        root->left = leftNode;
        root->right = implFree(root->right);
    } else if (root->value == "-") {
        root->left = implFree(root->left);
    } else {
        root->left = implFree(root->left);
        root->right = implFree(root->right);
    }
    return root;
}

void CnfTree::NNF() {
    root = NNF(root);
}

TreeNode* CnfTree::NNF(TreeNode* root) {
    if (root->valueType == 0 || (root->value == "-" && root->left->valueType == 0)) {
        return root;
    } else if (root->value == "-" && root->left->value == "-") {
        root = NNF(root->left->left);
    } else if (root->value == "&" || root->value == "|") {
        root->left = NNF(root->left);
        root->right = NNF(root->right);
    } else if (root->value == "-" && root->left->value == "&") {
        root->value = "|";
        root->valueType = 2;
        TreeNode* leftNode = new TreeNode("-");
        leftNode->left = root->left->left;
        TreeNode* rightNode = new TreeNode("-");
        rightNode->left = root->left->right;
        root->left = leftNode;
        root->right = rightNode;
        root = NNF(root);
    } else if (root->value == "-" && root->left->value == "|") {
        root->value = "&";
        root->valueType = 2;
        TreeNode* leftNode = new TreeNode("-");
        leftNode->left = root->left->left;
        TreeNode* rightNode = new TreeNode("-");
        rightNode->left = root->left->right;
        root->left = leftNode;
        root->right = rightNode;
        root = NNF(root);
    }
    return root;
}

void CnfTree::CNF() {
    root = CNF(root);
}

TreeNode* CnfTree::CNF(TreeNode* root) {
    if (root->valueType == 0 || (root->value == "-" && root->left->valueType == 0)) {
        return root;
    } else if (root->value == "&") {
        root->left = CNF(root->left);
        root->right = CNF(root->right);
    } else if (root->value == "|") {
        TreeNode* d = distr(CNF(root->left), CNF(root->right));
        root->value = d->value;
        root->left = d->left;
        root->right = d->right;
    }
    return root;
}

TreeNode* CnfTree::distr(TreeNode* node1, TreeNode* node2) {
    TreeNode* temp;
    if (node1->value == "&") {
        temp = new TreeNode("&");
        TreeNode* leftNode = distr(node1->left, node2);
        TreeNode* rightNode = distr(node1->right, node2);
        temp->left = leftNode;
        temp->right = rightNode;
    } else if (node2->value == "&") {
        temp = new TreeNode("&");
        TreeNode* leftNode = distr(node1, node2->left);
        TreeNode* rightNode = distr(node1, node2->right);
        temp->left = leftNode;
        temp->right = rightNode;
    } else {
        temp = new TreeNode("|");
        temp->left = node1;
        temp->right = node2;
    }
    return temp;
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

    CnfTree* tree = new CnfTree();

    tree->make_tree(str);
    
    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;
    cout << tree->get_postfix() << endl;
    cout << endl;

    tree->implFree();

    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;
    cout << tree->get_postfix() << endl;
    cout << endl;

    tree->NNF();

    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;
    cout << tree->get_postfix() << endl;
    cout << endl;

    tree->CNF();

    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;
    cout << tree->get_postfix() << endl;

    return 0;
}
