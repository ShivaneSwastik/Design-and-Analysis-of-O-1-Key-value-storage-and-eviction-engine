#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

// ==============================================================================
// 1. CORE ENGINE (O(1) DLL + HASHMAP)
// We use the exact same templated O(1) architecture we built previously.
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
};

// ==============================================================================
// 2. DATABASE / FILE LOADER INTERFACE
// Handles parsing of massive datasets and feeds them into the Cache Engine
// ==============================================================================

// Helper function to create synthetic database files for testing
void generateSyntheticDatabase(string filename, int numEntries, string type) {
    ofstream file(filename);
    if (!file.is_open()) return;

    file << "Key,Value\n"; // CSV Header

    for (int i = 1; i <= numEntries; i++) {
        if (type == "string") {
            // E.g., User_1, Raj_1
            file << "User_" << i << ",Name_" << i << "\n";
        } else if (type == "float") {
            // E.g., Sensor_1, 98.65
            file << "Sensor_" << i << "," << (float)i * 1.55 << "\n";
        }
    }
    file.close();
    cout << "[System] Generated synthetic database: " << filename << " (" << numEntries << " entries)\n";
}

// Data Loader for String Data (e.g., Names, Text)
void loadStringDatabase(string filename, int cacheCapacity) {
    cout << "\n--- Loading String Database: " << filename << " ---\n";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open database file.\n";
        return;
    }

    LRUCache<string, string> cache(cacheCapacity);
    string line, key, value;
    int processedCount = 0;

    // Skip the header line
    getline(file, line); 

    auto start = high_resolution_clock::now();

    // Read file line by line
    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, key, ',');   // Read up to comma
        getline(ss, value, ','); // Read rest of the line

        // Feed into our O(1) Cache Engine
        cache.put(key, value);
        processedCount++;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Successfully loaded " << processedCount << " rows into the String Cache.\n";
    cout << "Time taken for I/O + Caching: " << duration.count() << " ms.\n";
}

// Data Loader for Numerical/Float Data (e.g., Sensor Readings, Decimals)
void loadFloatDatabase(string filename, int cacheCapacity) {
    cout << "\n--- Loading Numerical Database: " << filename << " ---\n";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open database file.\n";
        return;
    }

    LRUCache<string, float> cache(cacheCapacity);
    string line, key, valueStr;
    int processedCount = 0;

    // Skip header
    getline(file, line);

    auto start = high_resolution_clock::now();

    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, key, ',');
        getline(ss, valueStr, ',');
        
        // Convert string to float
        float value = stof(valueStr); 

        // Feed into our O(1) Cache Engine
        cache.put(key, value);
        processedCount++;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Successfully loaded " << processedCount << " rows into the Numerical Cache.\n";
    cout << "Time taken for I/O + Caching: " << duration.count() << " ms.\n";
}


// ==============================================================================
// 3. MAIN EXECUTION MENU
// ==============================================================================

int main() {
    cout << "========================================================\n";
    cout << " O(1) Engine: Database & File Ingestion Interface \n";
    cout << "========================================================\n";

    // 1. Generate the databases automatically
    cout << "Initializing dataset generation...\n";
    generateSyntheticDatabase("names_1k.csv", 1000, "string");
    generateSyntheticDatabase("names_1Lakh.csv", 100000, "string");
    generateSyntheticDatabase("sensors_10Lakh.csv", 1000000, "float");

    int choice;
    cout << "\nSelect Database to Ingest:\n";
    cout << "1. 1,000 Entries (String / Names)\n";
    cout << "2. 100,000 Entries (1 Lakh - String / Names)\n";
    cout << "3. 1,000,000 Entries (10 Lakh - Numerical / Floats)\n";
    cout << "Choice: ";
    cin >> choice;

    // We set the cache capacity to 50% of the dataset size to force LRU evictions
    if (choice == 1) {
        loadStringDatabase("names_1k.csv", 500); 
    } else if (choice == 2) {
        loadStringDatabase("names_1Lakh.csv", 50000);
    } else if (choice == 3) {
        loadFloatDatabase("sensors_10Lakh.csv", 500000);
    } else {
        cout << "Invalid choice.\n";
    }

    return 0;
}