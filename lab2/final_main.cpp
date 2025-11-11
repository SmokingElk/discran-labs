#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cctype>

struct AVLNode {
    std::string key;
    unsigned long long number;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const std::string& k, unsigned long long num)
        : key(k), number(num), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;

    int height(AVLNode* node) {
        return node ? node->height : 0;
    }

    int getBalance(AVLNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    AVLNode* rightRotate(AVLNode* y) {
        if (!y || !y->left) return y;
        
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        if (!x || !x->right) return x;
        
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;
    }

    AVLNode* insert(AVLNode* node, const std::string& key, unsigned long long number) {
        if (!node) 
            return new AVLNode(key, number);

        if (key < node->key)
            node->left = insert(node->left, key, number);
        else if (key > node->key)
            node->right = insert(node->right, key, number);
        else
            return node;  // Duplicate key, return unchanged node

        node->height = 1 + std::max(height(node->left), height(node->right));
        int balance = getBalance(node);

        if (balance > 1 && key < node->left->key)
            return rightRotate(node);

        if (balance < -1 && key > node->right->key)
            return leftRotate(node);

        if (balance > 1 && key > node->left->key) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && key < node->right->key) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    AVLNode* minValueNode(AVLNode* node) {
        if (!node) return nullptr;
        
        AVLNode* current = node;
        while (current->left != nullptr)
            current = current->left;
        return current;
    }

    AVLNode* deleteNode(AVLNode* node, const std::string& key) {
        if (!node) return nullptr;

        if (key < node->key)
            node->left = deleteNode(node->left, key);
        else if (key > node->key)
            node->right = deleteNode(node->right, key);
        else {
            if (!node->left || !node->right) {
                AVLNode* temp = node->left ? node->left : node->right;

                if (!temp) {
                    temp = node;
                    node = nullptr;
                } else {
                    node->key = temp->key;
                    node->number = temp->number;
                    node->left = temp->left;
                    node->right = temp->right;
                }
                
                delete temp;
            } else {
                AVLNode* temp = minValueNode(node->right);
                
                node->key = temp->key;
                node->number = temp->number;
                
                node->right = deleteNode(node->right, temp->key);
            }
        }

        if (!node) return nullptr;

        node->height = 1 + std::max(height(node->left), height(node->right));
        int balance = getBalance(node);

        if (balance > 1 && getBalance(node->left) >= 0)
            return rightRotate(node);

        if (balance > 1 && getBalance(node->left) < 0) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && getBalance(node->right) <= 0)
            return leftRotate(node);

        if (balance < -1 && getBalance(node->right) > 0) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    AVLNode* find(AVLNode* node, const std::string& key) {
        if (!node) return nullptr;
        if (key < node->key)
            return find(node->left, key);
        else if (key > node->key)
            return find(node->right, key);
        else
            return node;
    }

    void serialize(AVLNode* node, std::ostream& os) {
        if (!node) {
            os.put(0);
            return;
        }
        os.put(1);
        size_t keySize = node->key.size();
        os.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        os.write(node->key.c_str(), keySize);
        os.write(reinterpret_cast<const char*>(&node->number), sizeof(node->number));
        serialize(node->left, os);
        serialize(node->right, os);
    }

    AVLNode* deserialize(std::istream& is) {
        if (is.peek() == EOF) return nullptr;
        
        char marker;
        is.get(marker);
        if (marker == 0 || is.fail()) return nullptr;

        size_t keySize;
        is.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        if (is.fail() || keySize > 256) return nullptr;
        
        std::string key(keySize, '\0');
        is.read(&key[0], keySize);
        if (is.fail()) return nullptr;
        
        unsigned long long number;
        is.read(reinterpret_cast<char*>(&number), sizeof(number));
        if (is.fail()) return nullptr;
        
        AVLNode* node = new AVLNode(key, number);
        
        try {
            node->left = deserialize(is);
            node->right = deserialize(is);
            
            if (is.fail()) {
                throw std::runtime_error("Failed to deserialize child nodes");
            }
            
            node->height = 1 + std::max(height(node->left), height(node->right));
            return node;
        } catch (...) {
            clearTree(node);
            throw;
        }
    }

    std::string toLower(const std::string& str) {
        std::string result = str;
        for (char& c : result) {
            if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 'a';
            }
        }
        return result;
    }

public:
    AVLTree() : root(nullptr) {}
    
    ~AVLTree() {
        clearTree(root);
    }

    void insert(const std::string& key, unsigned long long number) {
        std::string lowerKey = toLower(key);
        if (find(root, lowerKey)) {
            std::cout << "Exist" << std::endl;
        } else {
            root = insert(root, lowerKey, number);
            std::cout << "OK" << std::endl;
        }
    }

    void remove(const std::string& key) {
        std::string lowerKey = toLower(key);
        if (find(root, lowerKey)) {
            root = deleteNode(root, lowerKey);
            std::cout << "OK" << std::endl;
        } else {
            std::cout << "NoSuchWord" << std::endl;
        }
    }

    void search(const std::string& key) {
        std::string lowerKey = toLower(key);
        AVLNode* node = find(root, lowerKey);
        if (node) {
            std::cout << "OK: " << node->number << std::endl;
        } else {
            std::cout << "NoSuchWord" << std::endl;
        }
    }

    void save(const std::string& filepath) {
        std::ofstream ofs(filepath, std::ios::binary);
        serialize(root, ofs);
        ofs.close();
        std::cout << "OK" << std::endl;
    }

    void load(const std::string& filepath) {
        try {
            std::ifstream ifs(filepath, std::ios::binary);
            if (!ifs) {
                std::cout << "ERROR: Cannot open file for reading" << std::endl;
                return;
            }
            
            AVLNode* newRoot = nullptr;
            try {
                newRoot = deserialize(ifs);
                if (!newRoot && !ifs.eof()) {
                    std::cout << "ERROR: Invalid file format" << std::endl;
                    return;
                }
            } catch (...) {
                std::cout << "ERROR: Exception during deserialization" << std::endl;
                return;
            }
            
            AVLNode* oldRoot = root;
            root = newRoot;
            clearTree(oldRoot);
            std::cout << "OK" << std::endl;
            
            ifs.close();
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "ERROR: Unknown error" << std::endl;
        }
    }
    
    void clearTree(AVLNode* node) {
        if (!node) return;
        clearTree(node->left);
        clearTree(node->right);
        delete node;
    }
};

int main() {
    AVLTree tree;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        
        char firstChar = line[0];
        
        if (firstChar == '+') {
            std::istringstream iss(line.substr(2));
            std::string word;
            unsigned long long number;
            if (iss >> word >> number) {
                if (word.length() > 256) {
                    std::cout << "ERROR: Word too long" << std::endl;
                    continue;
                }
                bool valid = true;
                for (char c : word) {
                    if (!std::isalpha(c)) {
                        valid = false;
                        break;
                    }
                }
                if (!valid) {
                    std::cout << "ERROR: Invalid word" << std::endl;
                    continue;
                }
                tree.insert(word, number);
            } else {
                std::cout << "ERROR: Invalid format" << std::endl;
            }
        } else if (firstChar == '-') {
            std::string word = line.substr(2);
            tree.remove(word);
        } else if (firstChar == '!') {
            std::istringstream iss(line.substr(2));
            std::string operation, filepath;
            if (iss >> operation >> filepath) {
                if (operation == "Save") {
                    tree.save(filepath);
                } else if (operation == "Load") {
                    tree.load(filepath);
                } else {
                    std::cout << "ERROR: Unknown operation" << std::endl;
                }
            } else {
                std::cout << "ERROR: Invalid command format" << std::endl;
            }
        } else {
            tree.search(line);
        }
    }

    return 0;
}