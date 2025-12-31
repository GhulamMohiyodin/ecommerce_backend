#include "HashByID.h"

HashByID::HashByID(int s) : size(s), table(s) {}

int HashByID::hashFunction(int key) {
    return key % size;
}

bool HashByID::contains(int key) {
    int index = hashFunction(key);
    for (int id : table[index]) {
        if (id == key)
            return true;
    }
    return false;
}

void HashByID::insert(int key) {
    if (contains(key))
        return;
    table[hashFunction(key)].push_back(key);
}

void HashByID::remove(int key)
{
    table[hashFunction(key)].remove(key);
}
