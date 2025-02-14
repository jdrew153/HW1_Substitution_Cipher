#include <iostream>
#include <vector>
#include <string>

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>




#include "src/SubstitutionCipher.h"

using namespace ftxui;


ButtonOption Style() {
    auto option = ButtonOption::Animated();
    option.transform = [](const EntryState& s) {
        auto element = text(s.label);
        if (s.focused) {
        element |= bold;
        }
        return element | center | borderEmpty | flex;
    };
    return option;
}

std::vector<std::vector<Element>> PopulateFrequencyTable(
    const std::unordered_map<std::string, int>& m,
    const std::unordered_map<std::string, std::string>& substitutionMap,
    int threshold)
{

    // Construct Table Rows
    std::vector<std::vector<Element>> table_rows;
    
    // Table Header
    table_rows.push_back({
        text(" Letter ") | bold | center,
        text(" Frequency ") | bold | center,
        text("Mapped To") | bold | center
    });

    table_rows.push_back({
        separator() ,
        separator(),
        separator()

    });
    
     std::vector<std::pair<std::string, int>> sortedFrequencies(
        m.begin(), m.end());

    std::sort(sortedFrequencies.begin(), sortedFrequencies.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

 
    std::erase_if(sortedFrequencies, [&threshold](const auto& a) {
        return a.second < threshold; // Remove elements where frequency < threshold
    });

    for (size_t i = 0; i < sortedFrequencies.size(); ++i) {
        const auto& [letter, frequency] = sortedFrequencies[i];

        // Look up the mapping in the substitution map
        std::string mappedValue; 
        
        if (letter.length() == 1)
        {
            mappedValue = (substitutionMap.find(letter) != substitutionMap.end()) ? substitutionMap.at(letter) : "-";  // Show "-" if not mapped
        } else if (letter.length() == 2)
        {
            mappedValue += (substitutionMap.find(std::string(1,letter[0])) != substitutionMap.end()) ? substitutionMap.at(std::string(1, letter[0])) : "-";
            mappedValue += (substitutionMap.find(std::string(1,letter[1])) != substitutionMap.end()) ? substitutionMap.at(std::string(1, letter[1])) : "-";
        } else if (letter.length() == 3)
        {
            mappedValue += (substitutionMap.find(std::string(1,letter[0])) != substitutionMap.end()) ? substitutionMap.at(std::string(1, letter[0])) : "-";
            mappedValue += (substitutionMap.find(std::string(1,letter[1])) != substitutionMap.end()) ? substitutionMap.at(std::string(1, letter[1])) : "-";
            mappedValue += (substitutionMap.find(std::string(1,letter[2])) != substitutionMap.end()) ? substitutionMap.at(std::string(1, letter[2])) : "-";
        }
       

        table_rows.push_back({
            text(letter) | center,
            text(std::to_string(frequency)) | center,
            text(mappedValue) | center
        });
    }

    return table_rows;
}

std::vector<std::vector<Element>> PopulateNGramTable(const std::vector<std::pair<std::string, int>>& nGrams) {
    // Table Rows
    std::vector<std::vector<Element>> table_rows;
    
    // Table Header
    table_rows.push_back({
        text(" N-Gram ") | bold | center,
        text(" Frequency ") | bold | center
    });

    // Populate Table with Data
    for (const auto& [nGram, frequency] : nGrams) {
        table_rows.push_back({
            text(nGram) | center,
            text(std::to_string(frequency)) | center
        });
    }

    return table_rows;
}

std::string WrapText(const std::string& text, size_t max_width) {
    std::string wrapped;
    size_t line_length = 0;

    std::istringstream words(text);
    std::string word;

    while (words >> word) {
        if (line_length + word.length() >= max_width) {
            wrapped += " "; // Insert line break
            line_length = 0;
        }
        wrapped += word + " ";
        line_length += word.length() + 1;
    }

    return wrapped;
}



int main()
{
    bool validFilePathGiven = false;
    std::string inputPath;

retry:
   while (!validFilePathGiven)
   {
    std::cout << "Enter the file path or file name for the Cipher you want to decode: ";
    std::cin >> inputPath;

    std::filesystem::path cipherPath(inputPath);
    
    if (std::filesystem::exists(cipherPath))
    {
        std::ifstream cipherFile(cipherPath);

        if (!cipherFile.is_open()) 
        {
            std::cout << "Error: Could not open file.\n";
            continue;
        }

        cipherFile.seekg(0, std::ios::end);
        size_t cipherFileSize = static_cast<size_t>(cipherFile.tellg());
        cipherFile.seekg(0, std::ios::beg);

        size_t bufferSize = (cipherFileSize > 1024) ? 1024 : cipherFileSize;
        std::vector<char> buffer(bufferSize);

        if(!cipherFile.read(buffer.data(), bufferSize))
        {
            std::cout << "Invalid file detected. Make sure cipher file only contains alphabetic characters. " << std::endl;
        } else
        {
            std::erase_if(buffer, [] (char c) {
                return c == '\n' || c == '\t' || c == ' ' || c == '\r' ;
            });

            for (char c : buffer) {
                if (!std::isalpha(static_cast<unsigned char>(c))) { 
                    std::cout << "Invalid file detected. Make sure cipher file only contains alphabetic characters.\n";
                    cipherFile.close();
                    goto retry; 
                }
            }
            validFilePathGiven = true; 
        }
    }
    else
    {
        inputPath.clear();
        std::cout << "The filename or file path you gave was invalid." << std::endl;
    }
   }

   std::cout << "Let's get crackin :)" << std::endl;

    auto screen = ScreenInteractive::Fullscreen();

    SubstitutionCipher subCipher(inputPath);

    if (subCipher.m_corruptionDetected)
        exit(EXIT_FAILURE);

    std::string encryptedCipher = subCipher.encryptedCipher;
    std::string decryptedCipher = subCipher.decryptedCipher.length() > 0 ? subCipher.decryptedCipher : "Press 'Decode' to get started.";

    /// ------ Decryption ------

    auto decodeButton = Button("Decode - Substitution", [&] { 
        decryptedCipher = subCipher.Decode();
     }, Style());

     auto decodeCaesarButton =  Button("Decode - Caesar Shift", [&] { 
        decryptedCipher = subCipher.CaesarShiftDecode();
     }, Style());

    
    auto button = Container::Horizontal({
        decodeButton | flex,
        decodeCaesarButton | flex,
    });

    
    auto splitText = [](const std::string& text) {
        size_t midIndex = text.size() / 2;
        return std::make_pair(text.substr(0, midIndex), text.substr(midIndex));
    };
    
    auto [encFirstHalf, encSecondHalf] = splitText(encryptedCipher);

    auto decryptionRenderer = Renderer(button, [&] {
      
        auto [decFirstHalf, decSecondHalf] = splitText(decryptedCipher);

        return vbox({
            window(text(" Encrypted Text "), 
                vbox({
                    paragraphAlignLeft(encFirstHalf),  // First line
                    paragraphAlignLeft(encSecondHalf) // Second line
                }) | flex
            ),
            separator(),
            window(text(" Decrypted Text "), 
                vbox({
                    paragraphAlignLeft(decFirstHalf),  // First line
                    paragraphAlignLeft(decSecondHalf) // Second line
                }) | flex
            ) | border,
            button->Render() | flex
        }) | flex | border;
    });
    
    
    
    
   // Substitution Mappings
   std::unordered_map<std::string, std::string> substitutionMap = subCipher.GetSubstitutionMap();

    // ---- Frequency analysis ----
    std::unordered_map<std::string, int> unigramFrequencyMap = subCipher.GetNGramFrequencyMap(NGRAM_TYPE::UNI);
    std::unordered_map<std::string, int> digramFrequencyMap = subCipher.GetNGramFrequencyMap(NGRAM_TYPE::DI);
    std::unordered_map<std::string, int> trigramFrequencyMap = subCipher.GetNGramFrequencyMap(NGRAM_TYPE::TRI);

    std::vector<std::pair<std::string, int>>  nnGrams = subCipher.GetNNGram(NGRAM_TYPE::DI);
    std::vector<std::pair<std::string, int>>  nnnGrams = subCipher.GetNNGram(NGRAM_TYPE::TRI);


    auto refreshButton1 = Button("Refresh Mappings", [&]{
        substitutionMap = subCipher.GetSubstitutionMap();
    }, Style());

    auto refreshContainer1 = Container::Vertical({
        refreshButton1 | flex,
    });   
    
   
    // Wrap the letter frequencies inside a `Renderer`
    auto frequencyRenderer = Renderer(refreshContainer1, [&] {
       

        auto uni_rows = PopulateFrequencyTable(unigramFrequencyMap, substitutionMap, 0);
        auto di_rows = PopulateFrequencyTable(digramFrequencyMap, substitutionMap, 0);
        auto tri_rows = PopulateFrequencyTable(trigramFrequencyMap, substitutionMap, 0);


        auto nn_rows = PopulateNGramTable(nnGrams);
        auto nnn_rows = PopulateNGramTable(nnnGrams);
        // Slice the elements to only show a subset
        std::vector<std::vector<Element>> uni_elements = uni_rows;
        auto uni_table = Table(uni_elements);

        std::vector<std::vector<Element>> di_elements = di_rows;
        auto di_table = Table(di_elements);

        std::vector<std::vector<Element>> tri_elements = tri_rows;
        auto tri_table = Table(tri_elements);
        
        std::vector<std::vector<Element>> nn_elements = nn_rows;
        auto nn_table = Table(nn_elements);

        std::vector<std::vector<Element>> nnn_elements = nnn_rows;
        auto nnn_table = Table(nnn_elements);

        return vbox({
            text(" Letter Frequency Table ") | bold | center | border,
            refreshContainer1->Render() | flex,
            hbox({
                uni_table.Render() | flex,
                separator(),
                di_table.Render() | flex,
                separator(),
                tri_table.Render() | flex,
                separator(),
                vbox({
                    nn_table.Render() | flex,
                    separator(),
                    nnn_table.Render() | flex
                }) | flex,
            }) | flex | border,
        }) | flex | border;
    });

    auto refreshButton2 = Button("Refresh Mappings", [&]{
        substitutionMap = subCipher.GetSubstitutionMap();
    }, Style());

    auto refreshContainer2 = Container::Vertical({
        refreshButton2 | flex,
    });   

    // Function to render substitution mappings
    auto substitutionRenderer = Renderer(refreshContainer2, [&] {
        std::vector<Element> rows;
        
        // Header row
        rows.push_back(hbox({
            text(" Cipher ") | bold | center | flex,
            text(" Decoded ") | bold | center | flex,
          
        }));

       

        for (auto& [cipherLetter, decodedLetter] : substitutionMap) {
            rows.push_back(hbox({
                text(cipherLetter) | center  | flex,
                text(decodedLetter) | center | flex,
            }));
        }

        return vbox({
            window(text("Edit Substitution Mappings "), refreshContainer2->Render()),
            separator(),
            vbox(rows) | flex,
        }) | flex | border;
    });


    /// Set Caesar Shift Key Tab
    int shiftKey = subCipher.GetCasesarShiftKey();

    auto refreshButton3 = Button("Refresh Caesar Key", [&]{
        shiftKey = subCipher.GetCasesarShiftKey();
    }, Style());

    auto refreshContainer3 = Container::Vertical({
        refreshButton3 | flex,
    });   


    auto caesarRenderer = Renderer(refreshContainer3, [&] {

    
        return vbox({
           window(text(" Shift Key "), 
            vbox({
                filler(),
                hbox({
                    text(" Key ") | bold,
                    text(std::to_string(shiftKey))
                }) | flex | center,
                filler() | flex,
                refreshContainer3->Render() | flex,
            }) | flex
        ) | flex
        });
    });
  
   
    int tab_index = 0;
    std::vector<std::string> tab_entries = {
        "Decryption",
        "Frequency Analysis",
        "Substitution Mapping",
        "Caesar Shift"
    };
    auto tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
    auto tab_content = Container::Tab({
        decryptionRenderer,
        frequencyRenderer,
        substitutionRenderer,
        caesarRenderer
    }, &tab_index);

    auto exit_button =
      Button("Exit", [&] { screen.Exit(); }, ButtonOption::Animated());

    auto main_container = Container::Vertical({
    Container::Horizontal({
            tab_selection,
            exit_button,
        }),
        tab_content,
    });

    auto main_renderer = Renderer(main_container, [&] {
        return vbox({
            text("Encryption Analysis") | bold | hcenter,
            hbox({
                tab_selection->Render() | flex,
                exit_button->Render(),
            }),
            tab_content->Render() | flex,
        });
    });

 
    std::atomic<bool> refresh_ui_continue = true;
    std::thread refresh_ui([&] {
        while (refresh_ui_continue) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.05s);
     
        screen.Post(Event::Custom);
        }
    });
    
    screen.Loop(main_renderer);
    refresh_ui_continue = false;
    refresh_ui.join();
  
    return EXIT_SUCCESS;
}