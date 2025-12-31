#include"BTreeNode.h"


BTreeNode::BTreeNode(int _t, bool _leaf)
    : t(_t), leaf(_leaf), keys(2 * _t - 1, 0),
    vals(2 * _t - 1, nullptr), C(2 * _t, nullptr), n(0) {
}
void BTreeNode::traverse() {
    for (int i = 0; i < n; ++i) {
        if (!leaf && C[i])
            C[i]->traverse();
        if (vals[i])
            vals[i]->DisplayProduct();
    }
    if (!leaf && C[n])
        C[n]->traverse();
}

Product* BTreeNode::search(int k)
{
    int i = 0;
    while (i < n && k > keys[i])
        i++;

    if (i < n && keys[i] == k)
        return vals[i];
    if (leaf)
        return nullptr;
    if (!C[i])
        return nullptr;
    return C[i]->search(k);
}
void BTreeNode::splitChild(int i, BTreeNode* y)
{
    BTreeNode* z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;
    for (int j = 0; j < t - 1; ++j)
    {
        z->keys[j] = y->keys[j + t];
        z->vals[j] = y->vals[j + t];
        y->keys[j + t] = 0;
        y->vals[j + t] = nullptr;
    }
    if (!y->leaf) {
        for (int j = 0; j < t; ++j) {
            z->C[j] = y->C[j + t];
            y->C[j + t] = nullptr;
        }
    }
    for (int j = n; j >= i + 1; --j) {
        C[j + 1] = C[j];
    }
    C[i + 1] = z;
    for (int j = n - 1; j >= i; --j) {
        keys[j + 1] = keys[j];
        vals[j + 1] = vals[j];
    }
    keys[i] = y->keys[t - 1];
    vals[i] = y->vals[t - 1];
    y->keys[t - 1] = 0;
    y->vals[t - 1] = nullptr;
    y->n = t - 1;
    n++;
}

void BTreeNode::insertNonFull(Product* p)
{
    int k = p->GetProductID();
    int i = n - 1;

    if (leaf)
    {
        while (i >= 0 && keys[i] > k) {
            keys[i + 1] = keys[i];
            vals[i + 1] = vals[i];
            --i;
        }
        keys[i + 1] = k;
        vals[i + 1] = p;
        n++;
    }
    else {
        while (i >= 0 && keys[i] > k) --i;
        ++i;

        if (!C[i]) {
            C[i] = new BTreeNode(t, true);
        }

        if (C[i]->n == 2 * t - 1) {
            splitChild(i, C[i]);
            if (k > keys[i]) ++i;
        }
        C[i]->insertNonFull(p);
    }
}
int BTreeNode::findKey(int k) {
    int idx = 0;
    while (idx < n && keys[idx] < k)
        ++idx;
    return idx;
}

void BTreeNode::removeFromLeaf(int idx)
{
    for (int i = idx + 1; i < n; ++i) {
        keys[i - 1] = keys[i];
        vals[i - 1] = vals[i];
    }
    keys[n - 1] = 0;
    vals[n - 1] = nullptr;
    --n;
}

void BTreeNode::removeFromNonLeaf(int idx)
{
    int k = keys[idx];

    if (C[idx] && C[idx]->n >= t)
    {
        BTreeNode* cur = C[idx];
        while (!cur->leaf) cur = cur->C[cur->n];
        int predKey = cur->keys[cur->n - 1];
        Product* predVal = cur->vals[cur->n - 1];
        keys[idx] = predKey;
        vals[idx] = predVal;

        C[idx]->remove(predKey);
    }
    else if (C[idx + 1] && C[idx + 1]->n >= t)
    {
        BTreeNode* cur = C[idx + 1];
        while (!cur->leaf) cur = cur->C[0];
        int succKey = cur->keys[0];
        Product* succVal = cur->vals[0];
        keys[idx] = succKey;
        vals[idx] = succVal;
        C[idx + 1]->remove(succKey);
    }
    else
    {
        merge(idx);
        if (C[idx]) C[idx]->remove(k);
    }
}

void BTreeNode::fill(int idx)
{
    if (idx != 0 && C[idx - 1] && C[idx - 1]->n >= t)
        borrowFromPrev(idx);
    else if (idx != n && C[idx + 1] && C[idx + 1]->n >= t)
        borrowFromNext(idx);
    else {
        if (idx != n)
            merge(idx);
        else
            merge(idx - 1);
    }
}

void BTreeNode::borrowFromPrev(int idx)
{
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx - 1];
    if (!child || !sibling)
        return;
    for (int i = child->n - 1; i >= 0; --i)
    {
        child->keys[i + 1] = child->keys[i];
        child->vals[i + 1] = child->vals[i];
    }
    if (!child->leaf)
    {
        for (int i = child->n; i >= 0; --i) {
            child->C[i + 1] = child->C[i];
        }
    }
    child->keys[0] = keys[idx - 1];
    child->vals[0] = vals[idx - 1];
    if (!child->leaf)
    {
        child->C[0] = sibling->C[sibling->n];
        sibling->C[sibling->n] = nullptr;
    }
    keys[idx - 1] = sibling->keys[sibling->n - 1];
    vals[idx - 1] = sibling->vals[sibling->n - 1];
    sibling->keys[sibling->n - 1] = 0;
    sibling->vals[sibling->n - 1] = nullptr;
    child->n += 1;
    sibling->n -= 1;
}

void BTreeNode::borrowFromNext(int idx)
{
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx + 1];
    if (!child || !sibling) return;
    child->keys[child->n] = keys[idx];
    child->vals[child->n] = vals[idx];
    if (!child->leaf)
    {
        child->C[child->n + 1] = sibling->C[0];
        sibling->C[0] = nullptr;
    }
    keys[idx] = sibling->keys[0];
    vals[idx] = sibling->vals[0];
    for (int i = 1; i < sibling->n; ++i)
    {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->vals[i - 1] = sibling->vals[i];
    }
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; ++i) {
            sibling->C[i - 1] = sibling->C[i];
        }
        sibling->C[sibling->n] = nullptr;
    }
    sibling->keys[sibling->n - 1] = 0;
    sibling->vals[sibling->n - 1] = nullptr;
    child->n += 1;
    sibling->n -= 1;
}

void BTreeNode::merge(int idx)
{
    BTreeNode* child = C[idx];
    BTreeNode* sibling = C[idx + 1];
    if (!child || !sibling)
        return;
    child->keys[t - 1] = keys[idx];
    child->vals[t - 1] = vals[idx];
    for (int i = 0; i < sibling->n; ++i)
    {
        child->keys[i + t] = sibling->keys[i];
        child->vals[i + t] = sibling->vals[i];

        sibling->keys[i] = 0;
        sibling->vals[i] = nullptr;
    }
    if (!child->leaf)
    {
        for (int i = 0; i <= sibling->n; ++i) {
            child->C[i + t] = sibling->C[i];
            sibling->C[i] = nullptr;
        }
    }
    keys[idx] = 0;
    vals[idx] = nullptr;
    for (int i = idx + 1; i < n; ++i) {
        keys[i - 1] = keys[i];
        vals[i - 1] = vals[i];
    }
    for (int i = idx + 2; i <= n; ++i)
    {
        C[i - 1] = C[i];
    }
    C[n] = nullptr;
    child->n = child->n + sibling->n + 1;
    n--;
    delete sibling;
}

void BTreeNode::remove(int k)
{
    int idx = findKey(k);
    if (idx < n && keys[idx] == k)
    {
        if (leaf) removeFromLeaf(idx);
        else removeFromNonLeaf(idx);
    }
    else
    {
        if (leaf) return;
        bool flag = (idx == n);
        if (!C[idx])
            return;
        if (C[idx]->n < t)
            fill(idx);
        if (flag && idx > n)
        {
            if (C[idx - 1]) C[idx - 1]->remove(k);
        }
        else
        {
            if (C[idx]) C[idx]->remove(k);
        }
    }
}

