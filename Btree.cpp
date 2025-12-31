#include "BTree.h"


BTree::BTree(int _t) : root(nullptr), t(_t) {}

void BTree::destroyNode(BTreeNode* node)
{
    if (!node)
        return;
    for (int i = 0; i <= node->n; ++i)
    {
        if (node->C[i])
        {
            destroyNode(node->C[i]);
            node->C[i] = nullptr;
        }
    }
    for (int i = 0; i < node->n; ++i)
    {
        delete node->vals[i];
        node->vals[i] = nullptr;
    }
    delete node;
}


BTree::~BTree()
{
    destroyNode(root);
    root = nullptr;
}

BTreeNode* BTree::GetRoot()
{
    return root;
}

void BTree::insert(Product* p)
{
    if (!p)
        return;
    int k = p->GetProductID();
    if (!root)
    {
        root = new BTreeNode(t, true);
        root->keys[0] = k;
        root->vals[0] = p;
        root->n = 1;
        return;
    }

    if (root->n == 2 * t - 1) {
        BTreeNode* s = new BTreeNode(t, false);
        s->C[0] = root;
        s->n = 0;
        s->splitChild(0, root);
        int i = 0;
        if (k > s->keys[0])
            i = 1;
        s->C[i]->insertNonFull(p);
        root = s;
    }
    else {
        root->insertNonFull(p);
    }
}

Product* BTree::search(int k)
{
    return root ? root->search(k) : nullptr;
}

void BTree::traverse()
{
    if (root)
    {
        cout << "ID\t" << "Name\t\t\t\t" << "Price\t" << "Stock\n" << endl;
        root->traverse();
    }
    else
        cout << "Empty BTree\n";
}

void BTree::remove(int k)
{
    if (!root) return;
    root->remove(k);
    if (root->n == 0) {
        BTreeNode* tmp = root;
        if (root->leaf)
        {
            destroyNode(root);
            root = nullptr;
        }
        else
        {
            root = root->C[0];
            tmp->C[0] = nullptr;
            delete tmp;
        }
    }
}
