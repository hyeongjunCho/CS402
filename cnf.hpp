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
        TreeNode *left, *right;
};

class CnfTree
{
    public:
        ~CnfTree();
        void clean(TreeNode* root);
        void make_tree(const string expr);
        string get_prefix();
        string get_postfix();
        string get_infix();
    private:
        stack<TreeNode**> op_stack;
        string get_prefix(TreeNode* root);
        string get_postfix(TreeNode* root);
        string get_infix(TreeNode* root);
        TreeNode* root;
};

#endif