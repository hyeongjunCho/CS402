#ifndef CNF_H
#define CNF_H

#include <string>
#include <stack>
#include <memory>

using namespace std;

class TreeNode {
    public:
        TreeNode(const string val);
        ~TreeNode();
        // void Free();
        string value;
        int valueType; // 0: literal, 1: unary op, 2: binary op
        shared_ptr<TreeNode> left, right;
        weak_ptr<TreeNode> parent;
        bool is_compact;
};

class CnfTree {
    public:
        CnfTree();
        ~CnfTree();
        void clean(shared_ptr<TreeNode> root);
        void make_tree(const string expr);
        vector<string> get_literals();
        void check_parent();
        string get_prefix();
        string get_postfix();
        string get_infix();
        string get_minisat_form();
        int get_max_literal();
        void implFree();
        void NNF();
        void CNF();
    private:
        // stack<shared_ptr<TreeNode>*> op_stack; // Not stack of TreeNode list
        stack<shared_ptr<TreeNode>> exp_stack;
        shared_ptr<TreeNode> make_sub_tree(const string expr);
        void check_parent(shared_ptr<TreeNode> node, int* count0, int* count1);
        void set_parent(shared_ptr<TreeNode> node);
        string get_prefix(shared_ptr<TreeNode> node);
        string get_postfix(shared_ptr<TreeNode> node);
        string get_infix(shared_ptr<TreeNode> node);
        string get_minisat_form(shared_ptr<TreeNode> node);
        shared_ptr<TreeNode> implFree(shared_ptr<TreeNode> node);
        shared_ptr<TreeNode> NNF(shared_ptr<TreeNode> node);
        shared_ptr<TreeNode> CNF(shared_ptr<TreeNode> node);
        shared_ptr<TreeNode> distr(shared_ptr<TreeNode> node1, shared_ptr<TreeNode> node2);
        void compact_tree(shared_ptr<TreeNode>& node);
        shared_ptr<TreeNode> root;
        vector<string> literals;
        int CNF_clauses;
};

int node_num = 0;
void reset(shared_ptr<TreeNode>& node);

#endif