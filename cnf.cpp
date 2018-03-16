#include <iostream>
#include <fstream>
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
                op->parent = *op_stack.top();
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
                op->parent = *op_stack.top();
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
                l->parent = *op_stack.top();
                *op_stack.top() = l;
                op_stack.pop();
                if (!(find(begin(literals), end(literals), l->value) != end(literals))) {
                    literals.push_back(l->value);
                }
            }
        }
    }
}

vector<string> CnfTree::get_literals() {
    return literals;
}

string CnfTree::get_prefix() {
    return get_prefix(root);
}

string CnfTree::get_prefix(TreeNode* node) {
    if (node == nullptr) return "";
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
    if (node == nullptr) return "";
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
    if (node == nullptr) return "";
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
    if (node == nullptr) return "";
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
        [](char c) { return !std::isdigit(c); }) == node->value.end()) {
            return *find(begin(literals), end(literals), node->value);
        }
        return to_string(distance(begin(literals), find(begin(literals), end(literals), node->value)) + 1);
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
    if (node->valueType == 0 || (node->value == "-" && node->left->valueType == 0)) {
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
        node = NNF(node);
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
        node = NNF(node);
    }
    return node;
}

void CnfTree::CNF() {
    root = CNF(root);
}

TreeNode* CnfTree::CNF(TreeNode* node) {
    if (node->valueType == 0 || (node->value == "-" && node->left->valueType == 0)) {
        return node;
    } else if (node->value == "&") {
        node->left = CNF(node->left);
        node->right = CNF(node->right);
        node->left->parent = node;
        node->right->parent = node;
    } else if (node->value == "|") {
        TreeNode* d = distr(CNF(node->left), CNF(node->right));
        node->value = d->value;
        node->left = d->left;
        node->right = d->right;
        node->left->parent = node;
        node->right->parent = node;
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
    negationTree->make_tree("- " + str);

    // cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    tree->implFree();

    // cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    tree->NNF();

    // cout << tree->get_prefix() << endl;
    // cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    tree->CNF();

    cout << tree->get_prefix() << endl;
    cout << tree->get_infix() << endl;
    // cout << tree->get_postfix() << endl;
    // cout << endl;

    negationTree->implFree();
    negationTree->NNF();
    negationTree->CNF();

    // cout << negationTree->get_prefix() << endl;
    // cout << negationTree->get_infix() << endl;
    // cout << negationTree->get_postfix() << endl;
    // cout << endl;

    ofstream outf("negationMinisatForm.txt");
    outf << negationTree->get_minisat_form();
    outf.close();
    std::system("./minisat negationMinisatForm.txt negationMinisatOut.txt > temptemptemp.temp");
    std::system("rm temptemptemp.temp");


    ifstream negation("negationMinisatOut.txt");
    getline(negation, str);
    negation.close();
    if (str == "SAT") {
        cout << "Not Valid" << endl;
    } else {
        cout << "Valid" << endl;
    }

    return 0;
}
