/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "lexer.h"

// ARS
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std;

std::vector<Token> tokens;
std::vector<std::pair<string, std::vector<Token>>> rules;

std::unordered_map<string, std::vector<string>> first_cache;
std::unordered_map<string, bool> first_hash_cache;

std::vector<string>& FIRST(string key);

// read grammar
void ReadGrammar()
{
    LexicalAnalyzer anal;

    string current_rule = "";
    std::vector<Token> current_tokens;
    
    Token token;
    do
    {
        token = anal.GetToken();
        if (token.token_type == TokenType::ARROW) continue;
        tokens.push_back(token);

        current_tokens.push_back(token);
        if (anal.peek(1).token_type == TokenType::ARROW)
        {
            if (current_rule != "") rules.push_back({ current_rule, current_tokens});
            current_rule = token.lexeme;
            current_tokens.clear();
        }

    } while (token.token_type != TokenType::END_OF_FILE);  
    if (current_rule != "") rules.push_back({ current_rule, current_tokens});

    // for (auto rule : rules) std::cout << rule.first << "\n";
    // std::cout << "\n\n\n\n\n\n";
}

bool is_non_terminal(string text)
{
    for (auto rule : rules) if (rule.first == text) return true;
    return false;
}

// Task 1
void printTerminalsAndNoneTerminals()
{
    std::unordered_set<string> done_set;

    // Print terminals
    for (auto token : tokens)
    {
        if (!is_non_terminal(token.lexeme) && done_set.count(token.lexeme) == 0 && token.token_type == ID)
        {
            std::cout << token.lexeme << " ";
            done_set.insert(token.lexeme);
        }
    }
    done_set.clear();
    
    // Print nonterminals
    for (auto token : tokens)
    {
        if (is_non_terminal(token.lexeme) && done_set.count(token.lexeme) == 0 && token.token_type == ID)
        {
            std::cout << token.lexeme << " ";
            done_set.insert(token.lexeme);
        }
    }
}

// Task 2
void RemoveUselessSymbols()
{
    cout << "2\n";
}

bool hasHash(std::vector<string> &list)
{
    for (auto element : list) if (element == "") return true;
    return false;
}

std::vector<string> FIRST_helper(std::vector<Token> tokens, int index, std::string original)
{
    if (index == tokens.size() || tokens[index].lexeme == "") 
    {
        first_hash_cache[original] = true;
        return {};
    }

    if (tokens[index].lexeme == original) 
    {
        if (first_hash_cache[tokens[index].lexeme]) return FIRST_helper(tokens, index + 1, original);
        return {};
    }

    std::vector<string> to_return;
    if (is_non_terminal(tokens[index].lexeme)) 
    {
        std::vector<string> first;
        if (first_cache.count(tokens[index].lexeme) != 0) 
        {
            first = first_cache[tokens[index].lexeme];
        }

        to_return.insert(to_return.end(), first.begin(), first.end());
        if (first_hash_cache[tokens[index].lexeme])
        {
            std::vector<string> recurse = FIRST_helper(tokens, index + 1, original);
            to_return.insert(to_return.end(), recurse.begin(), recurse.end());
        }
        return to_return;
    }
    else 
    {
        return {tokens[index].lexeme};
    }
}

std::vector<string>& FIRST(string key)
{
    std::vector<string> to_return;
    for (auto rule : rules)
    {
        if (rule.first != key) continue;

        vector<string> helped_first = FIRST_helper(rule.second, 0, rule.first);
        to_return.insert(to_return.end(), helped_first.begin(), helped_first.end());
    }

    std::vector<string> to_cache;
    std::unordered_set<string> done_set;

    for (string t : to_return)
    {
        if (done_set.count(t) == 0) to_cache.push_back(t);
        done_set.insert(t);
    }

    first_cache[key] = to_cache;

    // if (!first_hash_cache[key]) first_hash_cache[key] = hasHash(to_cache);
    return first_cache[key];
}

// Task 3
void CalculateFirstSets()
{
    for (int i = 0; i < 10; i++)
    {
    for (auto rule : rules) FIRST(rule.first);
    }

    std::unordered_set<string> done_set;
    for (Token token : tokens)
    {
        if (!is_non_terminal(token.lexeme)) continue;
        if (done_set.count(token.lexeme) > 0) continue;

        std::cout << "FIRST(";
        std::cout << token.lexeme;
        std::cout << ") = { ";

        std::vector<string> lexemes = FIRST(token.lexeme);

        bool first = true;

        if (first_hash_cache[token.lexeme]) 
        {
            std::cout << "#";
            first = false;
        }

        std::unordered_set<string> in_set;
        std::unordered_set<string> doof_set;
        for (string s : lexemes) in_set.insert(s);

        for (Token t : tokens)
        {
            // if (t.token_type != ID) continue;
            if (is_non_terminal(t.lexeme)) continue;
            if (in_set.count(t.lexeme) == 0 || doof_set.count(t.lexeme) != 0) continue;

            if (!first) std::cout << ", ";
            std::cout << t.lexeme;

            doof_set.insert(t.lexeme);
            first = false;
        }

        std::cout << " }\n";

        done_set.insert(token.lexeme);
    }
}

// Task 4
void CalculateFollowSets()
{
    cout << "4\n";
}

// Task 5
void CheckIfGrammarHasPredictiveParser()
{
    cout << "5\n";
}
    
int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: printTerminalsAndNoneTerminals();
            break;

        case 2: RemoveUselessSymbols();
            break;

        case 3: CalculateFirstSets();
            break;

        case 4: CalculateFollowSets();
            break;

        case 5: CheckIfGrammarHasPredictiveParser();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

