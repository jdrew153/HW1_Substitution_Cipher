# HW1_Substitution_Cipher
CSC1 6100 - HW1 Shift and Substitution Ciphers.

## How to use the program
1. To build and run the project, use "make && make run".

2. When the program initializes, a "keys" directory will be created
 with two files:
    "shift-key.json" and "substitution-key.json".

3. Type the file path for the cipher you want to decode following the prompt,
    - "Enter the file path or file name for the Cipher you want to decode:"
    *  Note: There is some input validation of the cipher to ensure only files containing 
        alphabetic characters. The cipher files given in the assignment are the basis 
        for the validation, so files that have similar content should pass the initial content checks.

4. Attempt to decrypt the cipher by editing the corresponding json file
    - Edit the shift-key.json file for solving Caesar Shift ciphers.
    - Edit the substitution-key.json file for the substitution cipher.
    * Note: The expected structure of the json file will be written when the files are created.
            If the json files don't have the expected structure, the program won't work as expected and will likely crash.

5. Visit the Frequency analysis tab to see a full analysis of the most frequent unigrams, digrams, and trigrams in the cipher.
    - After editing the json file with the shift key or a susbtitution, click the black button below the "Letter Frequency"
     header to see how the ngrams are decoded using your current keys.

6. When solving Caesar ciphers, visit the Caesar Shift tab to see the current numeric value of the key. However, when you edit the
    shift-key.json file, don't input the numeric value of the key, just enter the encoded letter followed by the letter you think 
    it decodes to. 
    * i.e. Based on the frequency analysis, you see that O is the most common letter, so it probably decrypts to E. So you'd 
    edit the shift-key.json file to { "O" : "E"}. After saving the shift-key.json file, click the refresh button on the 
    Caesar shift page to see the numeric value of the key.

7. When solving substitution ciphers, edit the substitution-key.json file by entering the encrypted letter, followed by the proposed
    decrypted letter.
    * i.e {"O" : "e", "A" : "i"}

8. Press the Exit button to leave the program, or press Ctl-C / Cmd-C. When you start the program back up, any current 
    key or substitution in the json files will be loaded and used to decrypt the input file. So if you want to solve a different
    substitution cipher / shift cipher or just want to start from scratch, delete the keys folder and the program will write 
    you new ones from scratch! 
    * Just be sure to delete the whole directory because the program won't write the json files if the keys directory is present, 
      but empty. 

## Project Structure
 -- include
    |
     -> json.hpp
 -- keys (will be created upon running the program)
    |
     -> shift-key.json 
    |
     -> substitution-key.json
 -- src
    |
     -> SubstitutionCipher.cpp
    | 
      -> SubstitutionCipher.h
 -- third-party
    |
     -> FXTUI (library for creating terminal gui applications.)
 -> main.cpp
 -> Makefile
 -> README.md (this file)
 



