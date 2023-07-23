#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>


struct Node
{
    std::string key;
    Node *pre, *next;
};


class LRUCache
{
public:
    LRUCache(size_t size);
    ~LRUCache();

    void Put(std::string key, const std::string &cacheDir);
    int getCount() {
        return count;
    }

private:
    // 插入到链表头
    void attach(Node *node);
    // 删除链表节点
    void detach(Node *node);
    std::mutex lru_mutex;
    int count;
    std::unordered_map<std::string, Node *> hashmap;
    std::vector<Node *> free_entries; // 用来表示cache是否已满
    Node *head_, *tail_;
    Node *entries_;  // 申请cache的首地址

};

#endif // LRUCACHE_H
