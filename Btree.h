#include"BTreeNode.h"
#include"Product.h"
#include <iostream>
using namespace std;
class BTree
{
private:
    BTreeNode* root;
    int t;
public:
    BTree(int _t);
    void destroyNode(BTreeNode* node);
    BTreeNode* GetRoot();
    void insert(Product* p);
    Product* search(int k);
    void traverse();
    void remove(int k);
    ~BTree();




};
