#include "lrucache.h"



LRUCache::LRUCache(size_t size)
{
    count = 0;
    entries_ = new Node[size];
    for (size_t i = 0; i < size; i++) {
        free_entries.push_back(entries_ + i);
    }

    head_ = new Node();
    tail_ = new Node();
    head_->pre = nullptr;
    head_->next = tail_;
    tail_->pre = head_;
    tail_->next = nullptr;

}

LRUCache::~LRUCache()
{
    delete head_;
    delete tail_;
    delete[] entries_;
}

void LRUCache::Put(std::string key, const std::string &cacheDir)
{
    std::lock_guard<std::mutex> mutex(lru_mutex);
    Node *node = nullptr;
    if (hashmap.find(key) != hashmap.end()) {
        node = hashmap[key];
    }

    // cache hit
    if (node != nullptr) {
        detach(node);
        node->key = key;
    } else {
        // update
        if (free_entries.empty()) { // cache full
            node = tail_->pre;
            detach(node);
            hashmap.erase(node->key);
        } else {
            // unfull
            node = free_entries.back();
            free_entries.pop_back();
        }
        // 插入新节点
        node->key = key;
//        node->data = data;
        hashmap.emplace(key);
        attach(node);
    }


}

void LRUCache::attach(Node *node)
{
    node->next = head_->next;
    node->pre = head_;
    head_->next = node;
    node->next->pre = node;
}

void LRUCache::detach(Node *node)
{
    node->pre->next = node->next;
    node->next->pre = node->pre;
}


