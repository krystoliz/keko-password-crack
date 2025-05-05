#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <crypt.h>
#include <chrono>

struct ShadowEntry {
    std::string username;
    std::string hash;
};

// Parse a shadow file entry into components
ShadowEntry parseShadowEntry(const std::string& entry) {
    ShadowEntry result;
    
    // Find the first colon which separates username from hash
    size_t usernameEnd = entry.find(':');
    if (usernameEnd == std::string::npos) {
        return result;
    }
    
    result.username = entry.substr(0, usernameEnd);
    
    // Extract the hash part
    size_t hashStart = usernameEnd + 1;
    size_t hashEnd = entry.find(':', hashStart);
    if (hashEnd == std::string::npos) {
        return result;
    }
    
    result.hash = entry.substr(hashStart, hashEnd - hashStart);
    return result;
}

// Check if a password matches the hash
bool checkPassword(const std::string& password, const std::string& fullHash) {
    // Use crypt() to hash the password with the same salt and algorithm
    char* result = crypt(password.c_str(), fullHash.c_str());
    
    // If crypt() fails, it returns NULL
    if (result == nullptr) {
        return false;
    }
    
    // Compare the generated hash with the stored hash
    return std::string(result) == fullHash;
}

// Read words from a dictionary file
std::vector<std::string> loadDictionary(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string word;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open dictionary file: " << filename << std::endl;
        return words;
    }
    
    while (std::getline(file, word)) {
        // Remove any trailing whitespace or newlines
        while (!word.empty() && (word.back() == '\n' || word.back() == '\r' || word.back() == ' ')) {
            word.pop_back();
        }
        
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    
    file.close();
    return words;
}

// Create a simple dictionary file for demonstration
void createDemoDictionary(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create demo dictionary file: " << filename << std::endl;
        return;
    }
    
    // Add some common passwords for demonstration
    std::vector<std::string> commonPasswords = {
        "password", "123456", "qwerty", "admin", "welcome",
        "letmein", "monkey", "1234", "12345", "football",
        "password123", "abc123", "dragon", "secret", "summer",
        "iloveyou", "sunshine", "master", "hello", "freedom",
        // Add your own words for testing
        "testpassword", "assignment", "security", "dictionary"
    };
    
    for (const auto& password : commonPasswords) {
        file << password << std::endl;
    }
    
    file.close();
    std::cout << "Demo dictionary created with " << commonPasswords.size() << " entries." << std::endl;
}

// Run dictionary attack against a single hash
bool dictionaryAttack(const ShadowEntry& target, const std::vector<std::string>& dictionary) {
    std::cout << "Starting dictionary attack on user: " << target.username << std::endl;
    std::cout << "Hash: " << target.hash << std::endl;
    std::cout << "Dictionary size: " << dictionary.size() << " words" << std::endl;
    
    int attempts = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (const auto& word : dictionary) {
        attempts++;
        
        // Print progress every 100 attempts
        if (attempts % 100 == 0) {
            std::cout << "Tried " << attempts << " passwords..." << std::endl;
        }
        
        // Check if this word matches the hash
        if (checkPassword(word, target.hash)) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            
            std::cout << "\nPassword found!" << std::endl;
            std::cout << "Username: " << target.username << std::endl;
            std::cout << "Password: " << word << std::endl;
            std::cout << "Attempts: " << attempts << std::endl;
            std::cout << "Time taken: " << duration << " ms" << std::endl;
            return true;
        }
        
        // Also try some simple variations
        std::string variation = word + "123";
        if (checkPassword(variation, target.hash)) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            
            std::cout << "\nPassword found!" << std::endl;
            std::cout << "Username: " << target.username << std::endl;
            std::cout << "Password: " << variation << std::endl;
            std::cout << "Attempts: " << attempts << " (with variations)" << std::endl;
            std::cout << "Time taken: " << duration << " ms" << std::endl;
            return true;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    std::cout << "\nPassword not found in dictionary." << std::endl;
    std::cout << "Attempts: " << attempts << " (with variations)" << std::endl;
    std::cout << "Time taken: " << duration << " ms" << std::endl;
    return false;
}

int main() {
    // Example shadow entries from the assignment
    std::vector<std::string> shadowEntries = {
        "tes:$y$j9T$n8JOS56Hq0Vo5PCH4tiZs1$0N7jRAw3SpW7q.Wzja47eBMen2Z4hvDA6HB/vEtRai2:20205:0:99999:7:::",
        "i1E3a4:$y$j9T$fyBfQ4rpRt/yIJm5oHKri1$iwf6Shf2s3tZvFzEw0ZFY8w/CB8.qNcs4q4HlcCtKdD:20205:0:99999:7:::",
        "DTETI25:$y$j9T$of3b9UX6hQVJomGpTNbO.1$0sVgDUEVqivqjOaCT0t/l8g.qC7GkkjAaCg6Vuf65a9:20205:0:99999:7:::"
    };
    
    std::cout << "Dictionary Attack Demonstration for Educational Purposes" << std::endl;
    std::cout << "=======================================================" << std::endl << std::endl;
    
    // Create a demo dictionary file
    std::string dictionaryFile = "demo_dictionary.txt";
    createDemoDictionary(dictionaryFile);
    
    // Load dictionary
    std::vector<std::string> dictionary = loadDictionary(dictionaryFile);
    
    if (dictionary.empty()) {
        std::cerr << "Failed to load dictionary. Exiting." << std::endl;
        return 1;
    }
    
    // Parse shadow entries
    std::vector<ShadowEntry> targets;
    for (const auto& entry : shadowEntries) {
        targets.push_back(parseShadowEntry(entry));
    }
    
    // Let user choose which account to try to crack
    std::cout << "Available accounts:" << std::endl;
    for (size_t i = 0; i < targets.size(); i++) {
        std::cout << i + 1 << ". " << targets[i].username << std::endl;
    }
    
    int choice;
    std::cout << "Enter number of account to attack (1-" << targets.size() << "): ";
    std::cin >> choice;
    
    if (choice < 1 || choice > static_cast<int>(targets.size())) {
        std::cerr << "Invalid choice. Exiting." << std::endl;
        return 1;
    }
    
    // Allow user to choose their own dictionary file
    std::string useCustomDict;
    std::cout << "Use custom dictionary file? (y/n): ";
    std::cin >> useCustomDict;
    
    if (useCustomDict == "y" || useCustomDict == "Y") {
        std::string customFile;
        std::cout << "Enter path to dictionary file: ";
        std::cin >> customFile;
        
        std::vector<std::string> customDictionary = loadDictionary(customFile);
        if (!customDictionary.empty()) {
            dictionary = customDictionary;
        } else {
            std::cout << "Failed to load custom dictionary. Using demo dictionary instead." << std::endl;
        }
    }
    
    // Run dictionary attack
    std::cout << std::endl;
    dictionaryAttack(targets[choice - 1], dictionary);
    
    std::cout << std::endl;
    std::cout << "Notes on modern password security:" << std::endl;
    std::cout << "- The yescrypt algorithm ($y$) used in these hashes is very resistant to attacks" << std::endl;
    std::cout << "- Real-world attacks would need much larger dictionaries and rule-based transformations" << std::endl;
    std::cout << "- This demonstration shows why strong, unique passwords are essential" << std::endl;
    std::cout << "- Always use secure password storage in real applications" << std::endl;
    
    return 0;
}