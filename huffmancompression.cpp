#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct HuffmanNode {
    char ch;
    int freq;
    int order;
    HuffmanNode *left, *right;
    HuffmanNode(char c, int f, int o) : ch(c), freq(f), order(o), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        if (a->freq != b->freq) return a->freq > b->freq;
        return a->order > b->order;
    }
};

unordered_map<char, int> buildCharFreqTable(const string& text) {
    unordered_map<char, int> freqTable;
    for (char ch : text) {
        freqTable[ch]++;
    }
    return freqTable;
}

HuffmanNode* buildHuffmanTree(const unordered_map<char, int>& freqTable) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> minHeap;

    vector<pair<char, int>> symbols(freqTable.begin(), freqTable.end());
    sort(symbols.begin(), symbols.end(),
         [](const pair<char, int>& a, const pair<char, int>& b) {
             return (unsigned char)a.first < (unsigned char)b.first;
         });

    int nextOrder = 0;
    for (const auto& pair : symbols) {
        minHeap.push(new HuffmanNode(pair.first, pair.second, nextOrder++));
    }

    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.top(); minHeap.pop();
        HuffmanNode* right = minHeap.top(); minHeap.pop();
        HuffmanNode* parent = new HuffmanNode('\0', left->freq + right->freq, nextOrder++);
        parent->left = left;
        parent->right = right;
        minHeap.push(parent);
    }
    return minHeap.top();
}

void generateCodes(HuffmanNode* root, const string& str, unordered_map<char, string>& huffmanCodes) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanCodes[root->ch] = str.empty() ? "0" : str;
    }
    generateCodes(root->left, str + "0", huffmanCodes);
    generateCodes(root->right, str + "1", huffmanCodes);
}

void compressFile(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in.is_open()) {
        cerr << "Error: Could not open input file." << endl;
        return;
    }

    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    if (content.empty()) {
        cerr << "Warning: Input file is empty." << endl;
        return;
    }

    unordered_map<char, int> freqTable = buildCharFreqTable(content);
    HuffmanNode* root = buildHuffmanTree(freqTable);

    unordered_map<char, string> huffmanCodes;
    generateCodes(root, "", huffmanCodes);

    ofstream out(outputFile, ios::binary);

    size_t tableSize = freqTable.size();
    out.write(reinterpret_cast<const char*>(&tableSize), sizeof(tableSize));
    for (const auto& pair : freqTable) {
        out.put(pair.first);
        out.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
    }

    string bitstream = "";
    for (char ch : content) {
        bitstream += huffmanCodes[ch];
    }

    size_t totalBits = bitstream.length();
    out.write(reinterpret_cast<const char*>(&totalBits), sizeof(totalBits));

    unsigned char buffer = 0;
    int bitCount = 0;
    for (char bit : bitstream) {
        buffer = (buffer << 1) | (bit - '0');
        bitCount++;
        if (bitCount == 8) {
            out.put(buffer);
            buffer = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        out.put(buffer);
    }

    out.close();
    cout << "Compression successful! Saved to " << outputFile << endl;
}

void decompressFile(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in.is_open()) {
        cerr << "Error: Could not open compressed file." << endl;
        return;
    }

    size_t tableSize;
    in.read(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));

    unordered_map<char, int> freqTable;
    for (size_t i = 0; i < tableSize; ++i) {
        char ch = in.get();
        int freq;
        in.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        freqTable[ch] = freq;
    }

    HuffmanNode* root = buildHuffmanTree(freqTable);

    size_t totalBits;
    in.read(reinterpret_cast<char*>(&totalBits), sizeof(totalBits));

    ofstream out(outputFile, ios::binary);

    if (!root->left && !root->right) {
        for (size_t i = 0; i < totalBits; ++i) out.put(root->ch);
        in.close();
        out.close();
        cout << "Decompression successful! Saved to " << outputFile << endl;
        return;
    }

    HuffmanNode* current = root;
    size_t bitsRead = 0;
    char byteVal;

    while (in.get(byteVal) && bitsRead < totalBits) {
        for (int i = 7; i >= 0 && bitsRead < totalBits; --i) {
            int bit = (byteVal >> i) & 1;
            current = (bit == 0) ? current->left : current->right;

            if (!current->left && !current->right) {
                out.put(current->ch);
                current = root;
            }
            bitsRead++;
        }
    }

    in.close();
    out.close();
    cout << "Decompression successful! Saved to " << outputFile << endl;
}

int main() {
    int choice;
    cout << "1. Compress File\n2. Decompress File\nEnter choice: ";
    cin >> choice;

    string inputPath, outputPath;
    if (choice == 1) {
        cout << "Enter source text filename (e.g., input.txt): ";
        cin >> inputPath;
        cout << "Enter target compressed filename (e.g., compressed.huff): ";
        cin >> outputPath;
        compressFile(inputPath, outputPath);
    } else if (choice == 2) {
        cout << "Enter compressed filename (e.g., compressed.huff): ";
        cin >> inputPath;
        cout << "Enter target decompressed filename (e.g., restored.txt): ";
        cin >> outputPath;
        decompressFile(inputPath, outputPath);
    } else {
        cout << "Invalid choice." << endl;
    }

    return 0;
}