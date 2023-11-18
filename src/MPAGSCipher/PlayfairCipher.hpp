#ifndef MPAGSCIPHER_PLAYFAIRCIPHER_HPP
#define MPAGSCIPHER_PLAYFAIRCIPHER_HPP

#include "CipherMode.hpp"

#include <string>
#include <map>
#include <vector>


/**
 * \file PlayfairCipher.hpp
 * \brief Contains the declaration of the PlayfairCipher class
 */

/**
 * \class PlayfairCipher
 * \brief Encrypt or decrypt text using the Playfair cipher with the given key
 */
class PlayfairCipher {
  public:

    explicit PlayfairCipher(const std::string& key);

    void setKey(const std::string& key);

    std::string applyCipher(const std::string& inputText, const CipherMode cipherMode) const;

    std::vector<int> applyRules(const std::vector<int>& coord1, const std::vector<int>& coord2) const;

    std::vector<int> applyRulesDecrypt(const std::vector<int>& coord1, const std::vector<int>& coord2) const;

  private:
    std::string key_{""};
    mutable std::string inputText_{""};
    std::map<char, std::vector<int>> mymap;
    std::map<std::vector<int>, char> reversedMap;

};

#endif