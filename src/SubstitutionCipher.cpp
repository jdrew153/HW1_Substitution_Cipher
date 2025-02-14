#include "SubstitutionCipher.h"

#include "../include/json.hpp"

#include <algorithm>

using json = nlohmann::json;

SubstitutionCipher::SubstitutionCipher(const std::string& path)
{
    if (!std::filesystem::exists(path))
    {
        std::cout << "Given path: " << path << " wasn't found." << std::endl;
        return;
    }
    
    m_cipherFile = std::make_unique<std::ifstream>(path);

    if (!m_cipherFile || !m_cipherFile->is_open())
    {
        std::cout << "Failed to initialize file." << std::endl;
        return;
    }

    m_cipherFile->seekg(0, std::ios::end);
    m_cipherFileSize = static_cast<size_t>(m_cipherFile->tellg());
    m_cipherFile->seekg(0, std::ios::beg);

    AssignCipherText();
    EraseNonAlphaNumericChars();
    SetFrequencyMaps();
   
    decryptedCipher = Decode();

    // initializing stuff for caeser shift
   m_corruptionDetected = PrepareCaesarShiftKey();
   SetSubstitutionMap();
}

void SubstitutionCipher::AssignCipherText()
{
    m_encryptedCipherData.resize(m_cipherFileSize);

    if (!m_cipherFile->read(reinterpret_cast<char*>(m_encryptedCipherData.data()), m_cipherFileSize))
    {
        std::cout << "Failed to read cipher data from file" << std::endl;
        return ;
    }

    encryptedCipher.assign(m_encryptedCipherData.begin(), m_encryptedCipherData.end());

    std::cout << encryptedCipher << std::endl;
}

void SubstitutionCipher::EraseNonAlphaNumericChars()
{
    size_t startingSize = encryptedCipher.size();
    encryptedCipher.erase(std::remove_if(encryptedCipher.begin(), encryptedCipher.end(),
                    [](char c) { return c == '\r' || c == '\n'; }), 
                    encryptedCipher.end());

    if (startingSize > encryptedCipher.size())
        std::cout << "Removed " << startingSize - encryptedCipher.size() << " non-printable characters." << std::endl;  
}

void SubstitutionCipher::SetFrequencyMaps()
{
    // unigrams
    m_unigramFrequencyMap = getNGramFrequencies(encryptedCipher, 1);

    m_digramFrequencyMap = getNGramFrequencies(encryptedCipher, 2);

    m_trigramFrequencyMap = getNGramFrequencies(encryptedCipher, 3);
}

std::pair<std::string, int> SubstitutionCipher::GetMostFrequentNGram(NGRAM_TYPE type)
{
    switch (type)
    {
    case NGRAM_TYPE::UNI:
        return getMostFrequentPair(m_unigramFrequencyMap);
    case NGRAM_TYPE::DI:
        return getMostFrequentPair(m_digramFrequencyMap);
    case NGRAM_TYPE::TRI:
        return getMostFrequentPair(m_trigramFrequencyMap);
    default:
        return {};
    }
}

void SubstitutionCipher::PrintNGramFrequencyMap(NGRAM_TYPE type, int threshold)
{
    switch (type)
    {
    case NGRAM_TYPE::UNI:
        PrintMap(m_unigramFrequencyMap, threshold);
        break;
    case NGRAM_TYPE::DI:
        PrintMap(m_digramFrequencyMap, threshold);
        break;
    case NGRAM_TYPE::TRI:
        PrintMap(m_trigramFrequencyMap, threshold);
        break;
    default:
        break;
    }
}

std::unordered_map<std::string, std::string> SubstitutionCipher::GetSubstitutionMap()
{
    std::ifstream file(SUBSTITUTION_MAPPING_PATH);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << SUBSTITUTION_MAPPING_PATH << std::endl;
        return {};
    }


    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << fileSize << std::endl;

    if (fileSize <= 0)
    {
        std::cout << "json file empty" << std::endl;
        return m_substitutionMap;
    }

    json j;
    file >> j;

    if (j.size() == 0)
        return{};
    
    for (auto& [key, value] : j.items())
    {
        m_substitutionMap[key] = value;
    }

    file.close();

    return m_substitutionMap;
}

void SubstitutionCipher::UpdateSubstitutionMap(const std::string& letter, const std::string& inverse)
{
    std::cout << "Mapping " << letter << " to " << inverse << "\n";
    if (inverse.empty()) return; 

    
    if (inverse.size() != 1) {
        std::cout << "Error: Only single-character mappings are allowed.\n";
        return;
    }

    char newChar = inverse[0];

    for (const auto& [key, value] : m_substitutionMap) {
        if (value == inverse) {
            std::cout << "Skipping assignment: " << letter << " -> " << inverse
                      << " (Letter '" << newChar << "' is already assigned to '" << key << "')\n";
            return;
        }
    }

    // Remove previous mapping if `letter` was mapped
    if (m_substitutionMap.find(letter) != m_substitutionMap.end()) {
        std::string oldMappedValue = m_substitutionMap[letter];
        m_usedValues.erase(oldMappedValue[0]); // Remove from used set
    }

    // Assign new mapping
    m_substitutionMap[letter] = inverse;

    json j; 
    for (const auto& [key, value] : m_substitutionMap) {
        j[key] = value;
    }

    std::ofstream file(SUBSTITUTION_MAPPING_PATH, std::ios::trunc); 
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open keys/substitution-keys.json for writing.\n";
        return;
    }

    
    try {
        file << j.dump(4);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Write Error: " << e.what() << std::endl;
        file.close();
        return;
    }
   
    file.close();

    std::cout << "Mapping updated successfully!\n";
}


std::string SubstitutionCipher::GetSubstitionLetterMapping(const std::string& key)
{
    auto itr = m_substitutionMap.find(key);
    if (itr != m_substitutionMap.end())
        return itr->second;
    else
        return {};
}


std::string SubstitutionCipher::Decode()
{
    decryptedCipher.clear();
    m_substitutionMap.clear();
    
    SetSubstitutionMap();

    std::string decodedCipher;

    for (const auto& c : encryptedCipher)
    {
        auto itr = m_substitutionMap.find(std::string(1, c));

        if (itr != m_substitutionMap.end())
        {
            decodedCipher += itr->second;
        }
        else
        {
            decodedCipher += '-';
        }
    }

    return decodedCipher;
}

std::vector<std::pair<std::string, int>> SubstitutionCipher::GetNNGram(NGRAM_TYPE type)
{
    std::vector<std::pair<std::string, int>> nnGrams;

    if (type == NGRAM_TYPE:: UNI)
        return nnGrams;

    if (type == NGRAM_TYPE::DI)
    {
        for (const auto& [c, ct] : m_digramFrequencyMap)
        {
            if (c[0] == c[1])
                nnGrams.emplace_back(c, ct);
        }
    } else if (type == NGRAM_TYPE::TRI)
    {
        for (const auto& [c, ct] : m_trigramFrequencyMap)
        {
            if (c[0] == c[1] || c[1] == c[2])
                nnGrams.emplace_back(c, ct);
        }
    }
    
    std::sort(nnGrams.begin(), nnGrams.end(), [](const auto& a, const auto& b) {
        return a.second > b.second; 
    });

    return nnGrams;
}

std::unordered_map<std::string, int>  SubstitutionCipher::GetNGramFrequencyMap(NGRAM_TYPE type)
{
    switch (type)
    {
    case NGRAM_TYPE::UNI:
        return m_unigramFrequencyMap;
    case NGRAM_TYPE::DI:
        return m_digramFrequencyMap;
    case NGRAM_TYPE::TRI:
        return m_trigramFrequencyMap;
    default:
        return {};
    }
}

std::vector<std::pair<std::string, int>> SortMapByValue(const std::unordered_map<std::string, int>& m)
{
    std::vector<std::pair<std::string, int>> sortedVec(m.begin(), m.end());

    std::sort(sortedVec.begin(), sortedVec.end(), 
        [](const auto& a, const auto& b) {
            return a.second > b.second;  
    });
    
    return sortedVec;
}

void SubstitutionCipher::SetSubstitutionMap()
{
    std::filesystem::path mapFilePath(SUBSTITUTION_MAPPING_PATH);

    if (!std::filesystem::exists(mapFilePath))
    {
        std::ofstream mapFile(mapFilePath);
        if (!mapFile)
        {
            std::cerr << "Couldn't create the substitution mapping file!" << std::endl;
            std::cerr << "Try creating the mapping file manually, make sure it is named: " << SUBSTITUTION_MAPPING_PATH << " and is in the same directory as the main executable." << std::endl;
            return;
        }
        json j;
        j["O"] = "e";
        mapFile << j.dump(4);
        mapFile.close();
        return;
    }
    
    std::ifstream file(mapFilePath);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << mapFilePath << std::endl;
        return;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << fileSize << std::endl;

    if (fileSize <= 0)
    {
        std::cout << "json file empty" << std::endl;
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing Error for sub map: " << e.what() << std::endl;
        file.close();
        return;
    }

    if (j.is_null() || j.empty()) {
        std::cout << "JSON file contains no data." << std::endl;
        file.close();
        return;
    }
    
    for (auto& [key, value] : j.items())
    {
        m_substitutionMap[key] = value;
    }

    file.close();
}



//// adding some functions for trying to solve the cipher using the Caesar Shift

bool SubstitutionCipher::PrepareCaesarShiftKey()
{   
    std::filesystem::path keysDirPath(KEYS_DIRNAME);
    if (!std::filesystem::exists(keysDirPath))
    {
        if (!std::filesystem::create_directory(keysDirPath))
        {
            std::cout << "Failed to create keys directory. Try creating it manually in the same directory the main executable is located." << std::endl;
            return false;
        }
        
    }
    
    std::filesystem::path shiftKeyPath(CAESAR_SHIFT_KEY_PATH);

    // If the file doesn't exist, create it with a default empty mapping
    if (!std::filesystem::exists(shiftKeyPath))
    {
        std::ofstream shiftKeyFile(shiftKeyPath);
        if (!shiftKeyFile)
        {
            std::cerr << "Couldn't create the caesar shift key file!" << std::endl;
            return false;
        }
        json j;
        j["E"] = "A"; // Empty object for manual mappings
        shiftKeyFile << j.dump(4);
        shiftKeyFile.close();
    }

    std::ifstream file(shiftKeyPath);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << shiftKeyPath << std::endl;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parsing Error for caesar: " << e.what() << std::endl;
        file.close();
        return false;
    }

    
    m_ShiftKey = j;

    file.close();
}


int SubstitutionCipher::GetCasesarShiftKey()
{
    PrepareCaesarShiftKey();

    // Example: {"O": "E"} means O -> E (Shift = 14 - 4 = 10)
    if (m_ShiftKey.empty())
    {
        std::cerr << "No shift key found in JSON!" << std::endl;
        return 0;
    }

    auto mapping = m_ShiftKey.begin(); // Use the first mapping
    char encryptedLetter = mapping->first[0]; // "O"
    char decryptedLetter = mapping->second[0]; // "E"

    int encryptedIndex = encryptedLetter - 'A';
    int decryptedIndex = decryptedLetter - 'A';

    return (encryptedIndex - decryptedIndex + 26) % 26;
}



std::string SubstitutionCipher::CaesarShiftDecode()
{
    int shiftValue = GetCasesarShiftKey(); 
    std::string decodedCipher;

    m_substitutionMap.clear(); 

    for (const auto& letter : encryptedCipher)
    {
        if (!std::isalpha(letter))
        {
            decodedCipher += letter; 
            continue;
        }

        int alphaIndex = letter - 'A';
        int decodedLetterIndex = (alphaIndex - shiftValue + 26) % 26;
        char decodedLetter = static_cast<char>('A' + decodedLetterIndex);

        m_substitutionMap[std::string(1, letter)] = std::string(1, decodedLetter);

        decodedCipher += decodedLetter;
    }

    return decodedCipher;
}




int SubstitutionCipher::GetLetterIndexInAlphabet(const std::string& letter)
{
   int index;
   for (int i = 0; i < m_alphabet.size(); i++)
   {
    if (m_alphabet[i] == letter)
    {
        index = i;
        break;
    }
   }

   return index;
}

std::unordered_map<std::string, int> getNGramFrequencies(const std::string& cipher, int length)
{
    std::unordered_map<std::string, int> nGramFreqs;

    for (int i = 0; i < cipher.size() - length; i++)
    {
        nGramFreqs[cipher.substr(i, length)]++;
    }

    return nGramFreqs;
}

std::pair<std::string, int> getMostFrequentPair(const std::unordered_map<std::string, int>& m)
{
    auto itr = std::max_element(m.begin(), m.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return b.second > a.second;
    });

    return *itr;
}

void PrintMap(const std::unordered_map<std::string, int>& m, int threshold)
{
    for (const auto& [c, ct] : m)
    {
       if (ct >= threshold)
            std::cout << c << " : " << ct << "\n";
    }
}