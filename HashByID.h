#pragma once
#include <list>
#include <vector>

class HashByID {
private:
    int size;
    std::vector<std::list<int>> table;
    int hashFunction(int key);

public:
    HashByID(int s = 101);
    bool contains(int key);
    void insert(int key);
    void remove(int key);
};
