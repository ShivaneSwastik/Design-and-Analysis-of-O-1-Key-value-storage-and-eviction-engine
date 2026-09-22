#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

// ==============================================================================
// 1. CORE $O(1)$ LRU ENGINE
// ==============================================================================

template <typename K, typename V>
struct Node {
    K key;
    V value;
    Node* prev;
    Node* next;
    Node(K k, V v) : key(k), value(v), prev(nullptr), next(nullptr) {}
};

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
    Node<K, V>* getTail() { return tail; }
};

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

// ==============================================================================
// 2. CSV BENCHMARK RUNNER
// ==============================================================================

struct CSVRecord {
    string key;
    string val;
};

int main() {
    string filename;
    cout << "Enter the path/name of your CSV file (e.g., dataset_100000_entries.csv): ";
    cin >> filename;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: File '" << filename << "' not found.\n";
        return 1;
    }

    cout << "[+] Pre-loading CSV records into RAM...\n";
    vector<CSVRecord> dataset;
    string line, k, v;

    // Skip Header line
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        if (getline(ss, k, ',') && getline(ss, v, ',')) {
            dataset.push_back({k, v});
        }
    }
    file.close();

    size_t totalOps = dataset.size();
    if (totalOps == 0) {
        cout << "Error: CSV file contains no records.\n";
        return 1;
    }

    cout << "[✓] Loaded " << totalOps << " records into RAM memory.\n";

    // Capacity is set to half the dataset size to force 50% evictions
    int cacheCapacity = totalOps / 2;
    LRUCache<string, string> cache(cacheCapacity);

    cout << "[+] Benchmarking in-memory $O(1)$ operations...\n";

    auto start = high_resolution_clock::now();

    for (size_t i = 0; i < totalOps; i++) {
        cache.put(dataset[i].key, dataset[i].val);
        string dummy;
        cache.get(dataset[i / 2].key, dummy); // Force hit/miss lookup
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    // Calculate metrics
    double totalUs = (double)duration.count();
    double avgLatencyUs = totalUs / totalOps;
    size_t memoryBytes = cache.getMemoryOverheadBytes();
    double memoryMB = (double)memoryBytes / (1024.0 * 1024.0);

    cout << "\n================ BENCHMARK RESULTS ================\n";
    cout << "Total Operations Processed : " << totalOps << "\n";
    cout << "Average Latency Per Op    : " << avgLatencyUs << " microseconds (us)\n";
    cout << "Memory Overhead            : " << memoryBytes << " Bytes (" << memoryMB << " MB)\n";
    cout << "===================================================\n";

    // Write results to benchmark_results.csv for plot_benchmarks.py
    ofstream outFile("benchmark_results.csv");
    outFile << "Operations,Latency_Microseconds,Memory_Overhead_Bytes\n";
    outFile << totalOps << "," << avgLatencyUs << "," << memoryBytes << "\n";
    outFile.close();

    cout << "\n[✓] Results saved to 'benchmark_results.csv'. You can now run 'python3 plot_benchmarks.py'!\n";

    return 0;
}