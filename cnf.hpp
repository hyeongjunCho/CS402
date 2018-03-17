#ifndef CNF_H
#define CNF_H

#include <string>
#include <stack>

using namespace std;

class TreeNode
{
    public:
        TreeNode(const string val);
        string value;
        int valueType; // 0: literal, 1: unary op, 2: binary op
        TreeNode *parent, *left, *right;
};

class CnfTree
{
    public:
        ~CnfTree();
        void clean(TreeNode* root);
        void make_tree(const string expr);
        vector<string> get_literals();
        string get_prefix();
        string get_postfix();
        string get_infix();
        string get_minisat_form();
        int get_max_literal();
        void implFree();
        void NNF();
        void CNF();
    private:
        stack<TreeNode**> op_stack; // Not stack of TreeNode list
        string get_prefix(TreeNode* node);
        string get_postfix(TreeNode* node);
        string get_infix(TreeNode* node);
        string get_minisat_form(TreeNode* node);
        TreeNode* implFree(TreeNode* node);
        TreeNode* NNF(TreeNode* node);
        TreeNode* CNF(TreeNode* node);
        TreeNode* distr(TreeNode* node1, TreeNode* node2);
        TreeNode* root;
        vector<string> literals;
        int CNF_clauses;
};

#endif