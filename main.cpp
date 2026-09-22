#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <functional>

using namespace std;
using namespace std::chrono;

// 1. CUSTOM DOUBLY LINKED LIST (DLL) NODE
template <typename K, typename V>
struct Node {
    K key;
    V value;
    Node* prev;
    Node* next;

    Node(K k, V v) : key(k), value(v), prev(nullptr), next(nullptr) {}
};

// 2. CUSTOM DOUBLY LINKED LIST IMPLEMENTATION
template <typename K, typename V>
class DoublyLinkedList {
private:
    Node<K, V>* head;
    Node<K, V>* tail;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr) {}

    ~DoublyLinkedList() {
        Node<K, V>* current = head;
        while (current != nullptr) {
            Node<K, V>* nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }

    void addFirst(Node<K, V>* node) {
        node->next = head;
        node->prev = nullptr;
        if (head != nullptr) head->prev = node;
        head = node;
        if (tail == nullptr) tail = head;
    }

    void remove(Node<K, V>* node) {
        if (node->prev != nullptr) node->prev->next = node->next;
        else head = node->next;

        if (node->next != nullptr) node->next->prev = node->prev;
        else tail = node->prev;
    }

    void moveToHead(Node<K, V>* node) {
        if (node == head) return;
        remove(node);
        addFirst(node);
    }

    Node<K, V>* getTail() {
        return tail;
    }
};

// 3. CUSTOM HASH MAP IMPLEMENTATION (SEPARATE CHAINING)
template <typename K, typename V>
struct HashNode {
    K key;
    Node<K, V>* dllNodePtr;
    HashNode* next;

    HashNode(K k, Node<K, V>* ptr) : key(k), dllNodePtr(ptr), next(nullptr) {}
};

template <typename K, typename V>
class CustomHashMap {
private:
    HashNode<K, V>** buckets;
    size_t numBuckets;
    size_t currentSize;
    const float MAX_LOAD_FACTOR = 0.75;

    size_t getBucketIndex(K key) {
        std::hash<K> hashFunc;
        return hashFunc(key) % numBuckets;
    }

    void resize() {
        size_t oldNumBuckets = numBuckets;
        HashNode<K, V>** oldBuckets = buckets;

        numBuckets *= 2;
        buckets = new HashNode<K, V>*[numBuckets]();
        currentSize = 0;

        for (size_t i = 0; i < oldNumBuckets; i++) {
            HashNode<K, V>* head = oldBuckets[i];
            while (head != nullptr) {
                put(head->key, head->dllNodePtr);
                HashNode<K, V>* temp = head;
                head = head->next;
                delete temp; 
            }
        }
        delete[] oldBuckets;
    }

public:
    CustomHashMap(size_t capacity = 16) : numBuckets(capacity), currentSize(0) {
        buckets = new HashNode<K, V>*[numBuckets]();
    }

    ~CustomHashMap() {
        for (size_t i = 0; i < numBuckets; i++) {
            HashNode<K, V>* head = buckets[i];
            while (head != nullptr) {
                HashNode<K, V>* temp = head;
                head = head->next;
                delete temp;
            }
        }
        delete[] buckets;
    }

    void put(K key, Node<K, V>* dllNodePtr) {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* head = buckets[index];

        while (head != nullptr) {
            if (head->key == key) {
                head->dllNodePtr = dllNodePtr;
                return;
            }
            head = head->next;
        }

        HashNode<K, V>* newNode = new HashNode<K, V>(key, dllNodePtr);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        currentSize++;

        if ((float)currentSize / numBuckets > MAX_LOAD_FACTOR) resize();
    }

    Node<K, V>* get(K key) {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* head = buckets[index];

        while (head != nullptr) {
            if (head->key == key) return head->dllNodePtr;
            head = head->next;
        }
        return nullptr;
    }

    void erase(K key) {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* head = buckets[index];
        HashNode<K, V>* prev = nullptr;

        while (head != nullptr) {
            if (head->key == key) {
                if (prev == nullptr) buckets[index] = head->next;
                else prev->next = head->next;
                delete head;
                currentSize--;
                return;
            }
            prev = head;
            head = head->next;
        }
    }

    size_t getBucketCount() { return numBuckets; }
};

// 4. O(1) LRU STORAGE ENGINE
template <typename K, typename V>
class LRUCache {
private:
    int capacity;
    int currentSize;
    DoublyLinkedList<K, V> dll;
    CustomHashMap<K, V> map;

    void evict() {
        Node<K, V>* tail = dll.getTail();
        if (tail != nullptr) {
            map.erase(tail->key);
            dll.remove(tail);
            delete tail;
            currentSize--;
        }
    }

public:
    LRUCache(int cap) : capacity(cap), currentSize(0), map(cap * 2) {}

    bool get(K key, V& outValue) {
        Node<K, V>* node = map.get(key);
        if (node == nullptr) return false;
        
        dll.moveToHead(node);
        outValue = node->value;
        return true; 
    }

    void put(K key, V value) {
        Node<K, V>* node = map.get(key);
        if (node != nullptr) {
            node->value = value;
            dll.moveToHead(node);
        } else {
            if (currentSize >= capacity) evict();
            Node<K, V>* newNode = new Node<K, V>(key, value);
            dll.addFirst(newNode);
            map.put(key, newNode);
            currentSize++;
        }
    }

    size_t getMemoryOverheadBytes() {
        size_t nodeCost = currentSize * (sizeof(K) + sizeof(V) + 2 * sizeof(Node<K, V>*));
        size_t hashCost = currentSize * sizeof(HashNode<K, V>);
        size_t arrayCost = map.getBucketCount() * sizeof(HashNode<K, V>*);
        return nodeCost + hashCost + arrayCost;
    }
};

// 5. INTERFACE & BENCHMARKING SUITE
void runInteractiveMode() {
    cout << "\n--- Interactive Mode (String Keys/Values) ---\n";
    cout << "Enter Cache Capacity: ";
    int cap; cin >> cap;
    
    LRUCache<string, string> cache(cap);
    int choice;
    
    do {
        cout << "\n1. Put | 2. Get | 3. Show Memory Tax | 4. Exit\nChoice: ";
        cin >> choice;
        if (choice == 1) {
            string k, v;
            cout << "Key: "; cin >> k;
            cout << "Value: "; cin >> v;
            cache.put(k, v);
            cout << "[Inserted]\n";
        } else if (choice == 2) {
            string k, v;
            cout << "Key: "; cin >> k;
            if (cache.get(k, v)) cout << "[HIT] Value: " << v << "\n";
            else cout << "[MISS] Not found.\n";
        } else if (choice == 3) {
            cout << "Memory Overhead: " << cache.getMemoryOverheadBytes() << " Bytes\n";
        }
    } while (choice != 4);
}

void runStressTest() {
    cout << "\n--- Stress Test (Up to 10 Million Requests) ---\n";
    ofstream file("benchmark_results.csv");
    file << "Operations,Avg_Latency_Microseconds,Memory_Overhead_Bytes\n";
    
    vector<int> workloads = {1000, 100000, 1000000, 10000000}; // Scales to 10 Million
    
    for (int ops : workloads) {
        LRUCache<int, float> stressCache(ops / 2); // 50% hit ratio capacity
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < ops; i++) {
            stressCache.put(i, (float)i * 1.5);
            float val;
            stressCache.get(i / 2, val); // Force reads, promotions, and evictions
        }
        
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        double avgLatency = (double)duration.count() / ops;
        size_t memOverhead = stressCache.getMemoryOverheadBytes();
        
        file << ops << "," << avgLatency << "," << memOverhead << "\n";
        cout << "Processed " << ops << " requests. Avg Latency: " << avgLatency << " us/op.\n";
    }
    file.close();
    cout << "Results saved to 'benchmark_results.csv'.\n";
}

int main() {
    int mode;
    cout << "1. Interactive Mode\n2. Automated Stress Test\nChoice: ";
    cin >> mode;
    if (mode == 1) runInteractiveMode();
    else if (mode == 2) runStressTest();
    return 0;
}