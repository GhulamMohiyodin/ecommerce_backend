#pragma once       
#include<vector>
#include"Product.h"
using namespace std;

class BTreeNode {
public:
    int t;
    bool leaf;
    vector<int> keys;
    vector<Product*> vals;
    vector<BTreeNode*> C;
    int n;

    BTreeNode(int _t, bool _leaf);
    void traverse();
    Product* search(int k);
    void splitChild(int i, BTreeNode* y);
    void insertNonFull(Product* p);
    int findKey(int k);
    void removeFromLeaf(int idx);
    void removeFromNonLeaf(int idx);
    void fill(int idx);
    void borrowFromPrev(int idx);
    void borrowFromNext(int idx);
    void merge(int idx);
    void remove(int k);
};
