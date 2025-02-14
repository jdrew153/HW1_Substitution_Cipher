#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <unordered_set>


enum NGRAM_TYPE
{
    UNI,
    DI,
    TRI,
};

const std::string KEYS_DIRNAME = "keys";
const std::string SUBSTITUTION_MAPPING_PATH = "keys/substitution-key.json";
const std::string CAESAR_SHIFT_KEY_PATH = "keys/shift-key.json";

constexpr const char* CAESAR_SHIFT_JSON_STARTER = R"(
    {
        "A": "E"
    }
)";
    

class SubstitutionCipher
{
public:
    SubstitutionCipher(const std::string& path);
    ~SubstitutionCipher() = default;


    std::string encryptedCipher;
    std::string decryptedCipher;

    void UpdateSubstitutionMap(const std::string& letter, const std::string& inverse);
    std::string GetSubstitionLetterMapping(const std::string& key);
    std::unordered_map<std::string, std::string> GetSubstitutionMap();

    std::pair<std::string, int> GetMostFrequentNGram(NGRAM_TYPE type);

    // only for digram rn
    std::vector<std::pair<std::string, int>> GetNNGram(NGRAM_TYPE type);

    void PrintNGramFrequencyMap(NGRAM_TYPE type, int threshold = 1);
    
    std::unordered_map<std::string, int>  GetNGramFrequencyMap(NGRAM_TYPE type);

    std::string Decode();
    std::string CaesarShiftDecode();
    int GetCasesarShiftKey();


    void UpdateCeasarShiftKey(std::string keyAlpha);
    bool m_corruptionDetected{};
private:
   
    std::unique_ptr<std::ifstream> m_cipherFile = nullptr;
    size_t m_cipherFileSize;

    std::vector<char> m_encryptedCipherData;

    std::unordered_map<std::string, int> m_unigramFrequencyMap;
    std::unordered_map<std::string, int> m_digramFrequencyMap;
    std::unordered_map<std::string, int> m_trigramFrequencyMap;

    std::unordered_map<std::string, std::string> m_substitutionMap;

    void AssignCipherText();

    void SetFrequencyMaps();

    void EraseNonAlphaNumericChars();

    // json file needed
    void SetSubstitutionMap();
    std::unordered_set<char> m_usedValues {};

    // Caesar shift stuff
    std::vector<std::string> m_alphabet;

    // "letter" -> "key (letter)" (easier to edit in the json file.)
    std::unordered_map<std::string, std::string> m_ShiftKey;
    bool PrepareCaesarShiftKey();

    // helper func
    int GetLetterIndexInAlphabet(const std::string& letter);


};

std::pair<std::string, int> getMostFrequentPair(const std::unordered_map<std::string, int>& m);
std::unordered_map<std::string, int> getNGramFrequencies(const std::string& cipher, int length);
void PrintMap(const std::unordered_map<std::string, int>& m, int threshold);
std::vector<std::pair<std::string, int>> SortMapByValue(const std::unordered_map<std::string, int>& m);
