#include "PlayfairCipher.hpp"
#include "CipherMode.hpp"

#include <string>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <map>
#include <vector>


PlayfairCipher::PlayfairCipher(const std::string& key){
    setKey(key);
}

void PlayfairCipher::setKey(const std::string& key)

{
// store the original key
key_ = key;

// Append the alphabet
std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
key_.append(alphabet);

// Make sure the key is upper case
std::transform(key_.begin(), key_.end(), key_.begin(), ::toupper);

// Remove non-alpha characters
auto func = [](char c) {
    if (std::isalpha(c))
        return false;
    else
        return true;
};

auto iter = std::remove_if(key_.begin(), key_.end(), func);
key_.erase(iter, key_.end());

// Change J -> I
auto JI = [](char c) {
    if (c == 'J')
        return 'I';
    else
        return c;
};

std::transform(key_.begin(), key_.end(), key_.begin(), JI );

// Remove duplicated letters
std::string dupes;

auto isDuplicate = [&dupes](char c) {
    if (dupes.find(c) == std::string::npos){
        dupes += c;
        return false;
    } else {
        return true;
    }
};

auto iter2 = std::remove_if(key_.begin(), key_.end(), isDuplicate);
key_.erase(iter2, key_.end());

// Store the coords of each letter
std::vector<int> coord;
int row = 0;
int col = 0;

for (char c : key_) {
    coord.clear();
    coord.push_back(row);
    coord.push_back(col);
    auto p{std::make_pair(c, coord)};
    mymap.insert(p);
    col++;
    if (col == 5) {
        col = 0;
        row++;
    }
}

// Store the playfair cipher key map
    for (const auto& p : mymap) {
        reversedMap[p.second] = p.first;
    }
}

// Apply the playfair cipher
std::string PlayfairCipher::applyCipher(
const std::string& inputText,
const CipherMode cipherMode ) const
{
    inputText_= inputText;

    // Change J → I
    auto JI = [](char c) {
        if (c == 'J')
            return 'I';
        else
            return c;
    };
    
    std::transform(inputText_.begin(), inputText_.end(), inputText_.begin(), JI);

    std::string newText_;

    // If repeated chars in a digraph add an X or Q if XX
    char previousChar = '\0';

    for (char c : inputText_) {
        if (c == previousChar) {
            if (previousChar == 'X'){
                newText_ += 'Q';
                newText_ += c;
            } else {
                newText_ += 'X';
                newText_ += c;
            }
        } else {
            newText_ += c;
        }

        previousChar = c;
    }
    // Idea being if even add XZ which will be removed later to preserve even ending inputs ending in Z
    if ( newText_.size() % 2 == 0 && newText_.back() == 'Z'){
        newText_ += "XZ";
    }
    // if the size of input is odd and doesn't end in Z, add a trailing Z
    if ( newText_.size() % 2 != 0 && newText_.back() != 'Z'){
        newText_ += 'Z';
    }
    // if the size of input is odd and does end in Z, add an X, to preserve z ending
    if ( newText_.size() % 2 != 0 && newText_.back() == 'Z'){
        newText_ += 'X';
    }

    std::string finalText_{""};

    // Loop over the input in Digraphs
    std::string::size_type i = 0;
    while (i+1 < newText_.size()) {
        char firstChar = newText_[i];
        char secondChar = newText_[i + 1];

        // - Find the coords in the grid for each digraph
        auto firstIter = mymap.find(firstChar);
        auto secondIter = mymap.find(secondChar);

        if (firstIter != mymap.end() && secondIter != mymap.end()) {
            std::vector<int> firstCoords = firstIter->second;    // Use a pointer to second position "vector<int> in mymap"
            std::vector<int> secondCoords = secondIter->second;

            // Apply the rules to these coords to get new coords
            std::vector<int> newCoords;

            // Switch case for encrypt, decrypt
            switch (cipherMode) {
                case CipherMode::Encrypt:
                    newCoords = applyRules(firstCoords, secondCoords);
                    break;
                case CipherMode::Decrypt:
                    newCoords = applyRulesDecrypt(firstCoords, secondCoords);
                    break;
            }

            // Find the letter associated with the new coords
            auto encryptedFirstCharIter = reversedMap.find({newCoords[0], newCoords[1]});
            auto encryptedSecondCharIter = reversedMap.find({newCoords[2], newCoords[3]});

            if (encryptedFirstCharIter != reversedMap.end() && encryptedSecondCharIter != reversedMap.end()) {
                char encryptedFirstChar = encryptedFirstCharIter->second;    // Use a pointer to second position "char in reversedMap"
                char encryptedSecondChar = encryptedSecondCharIter->second;

                finalText_ += encryptedFirstChar;
                finalText_ += encryptedSecondChar;            
        }
        }
        i += 2;
    }
    size_t pos0;
    size_t pos1;

    switch (cipherMode) {
        case CipherMode::Decrypt:

            // If ends with Z remove Z as if original word ends in Z we appended X for odd or XZ for even
            if (finalText_.back() == 'Z'){
                finalText_.pop_back();
            }
            // Will remove X if at the end of string and comes after Z aka if true ending is Z
            if (finalText_.size() >= 2 && finalText_.substr(finalText_.size() - 2) == "ZX") {
                finalText_.pop_back(); // Remove 'X'
                }

            // Remove X when between identical letters
            pos0 = finalText_.find("X");
            while (pos0 != std::string::npos) {
                if (pos0 > 0 && pos0 + 1 < finalText_.size() && finalText_[pos0 - 1] == finalText_[pos0 + 1]) {
                    finalText_.erase(pos0, 1);
                }
                pos0 = finalText_.find("X", pos0 + 1);
            }

            // Remove Q which has been placed in XQX
            pos1 = finalText_.find("XQX");
            while (pos1 != std::string::npos) {
                finalText_.erase(pos1 + 1, 1);
                pos1 = finalText_.find("XQX", pos1 + 1);
            }

            // Still some problems with certain combinations of X's and Z's e.g. XZZX returns XZZ since it removes X off end but most dictionary words are foreseen to be covered
            break;

        case CipherMode::Encrypt:
    
        break;
    }

    // return the text
    return finalText_;
}

// Apply rules function - Could put in separate file
std::vector<int> PlayfairCipher::applyRules(const std::vector<int>& coord1, const std::vector<int>& coord2) const {
    int row1 = coord1[0];
    int col1 = coord1[1];
    int row2 = coord2[0];
    int col2 = coord2[1];

    std::vector<int> newCoords;

    // Rule 1
    if (row1 == row2) {
        std::vector<int> newCoord1 = {row1, (col1 + 1) % 5};
        std::vector<int> newCoord2 = {row2, (col2 + 1) % 5};
        newCoords.push_back(newCoord1[0]); 
        newCoords.push_back(newCoord1[1]);
        newCoords.push_back(newCoord2[0]);
        newCoords.push_back(newCoord2[1]);
    // Rule 2
    } else if (col1 == col2) {
        std::vector<int> newCoord1 = {(row1 - 1 + 5) % 5, col1};
        std::vector<int> newCoord2 = {(row2 - 1 + 5) % 5, col2};
        newCoords.push_back(newCoord1[0]);
        newCoords.push_back(newCoord1[1]);
        newCoords.push_back(newCoord2[0]);
        newCoords.push_back(newCoord2[1]);
    // Rule 3
    } else {
        std::vector<int> newCoord1 = {row1, (col2)};
        std::vector<int> newCoord2 = {row2, (col1)};
        newCoords.push_back(newCoord1[0]);
        newCoords.push_back(newCoord1[1]);
        newCoords.push_back(newCoord2[0]);
        newCoords.push_back(newCoord2[1]);
    }

    return newCoords;
}

// Apply decryption rules function
std::vector<int> PlayfairCipher::applyRulesDecrypt(const std::vector<int>& coord1, const std::vector<int>& coord2) const {
    int row1 = coord1[0];
    int col1 = coord1[1];
    int row2 = coord2[0];
    int col2 = coord2[1];

    std::vector<int> newCoords; 

    // Rule 1
    if (row1 == row2) {
        std::vector<int> newCoord1 = {row1, (col1 - 1 + 5) % 5};
        std::vector<int> newCoord2 = {row2, (col2 - 1 + 5) % 5};
        newCoords.push_back(newCoord1[0]); 
        newCoords.push_back(newCoord1[1]);
        newCoords.push_back(newCoord2[0]);
        newCoords.push_back(newCoord2[1]);
    // Rule 2
    } else if (col1 == col2) {
        std::vector<int> newCoord1 = {(row1 + 1 ) % 5, col1};
        std::vector<int> newCoord2 = {(row2 + 1 ) % 5, col2};
        newCoords.push_back(newCoord1[0]);
        newCoords.push_back(newCoord1[1]);
        newCoords.push_back(newCoord2[0]);
        newCoords.push_back(newCoord2[1]);
    // Rule 3
    } else {
        std::vector<int> newCoord1 = {row1, (col2)};
        std::vector<int> newCoord2 = {row2, (col1)};
        newCoords.push_back(newCoord1[0]);
        newCoords.push_back(newCoord1[1]);
        newCoords.push_back(newCoord2[0]);
        newCoords.push_back(newCoord2[1]);
    }

    return newCoords;
}