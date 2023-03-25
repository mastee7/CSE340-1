/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */

/*
    Author: Noel Ngu, Woojeh Chung
    CSE340 
    Project #2: Predictive Parser
*/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "lexer.h"

// ARS
#include <utility>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>

using namespace std;

std::vector<Token> tokens;
std::vector<std::pair<string, std::vector<Token>>> rules;

std::unordered_map<string, std::vector<string>> first_cache;
std::unordered_map<string, bool> first_hash_cache;

bool is_useless = false;
std::unordered_set<std::string> reachable;
std::vector<std::pair<string, std::vector<Token>>> new_rules;

std::unordered_map<std::string, std::unordered_set<std::string>> globow;

// Function for checking syntax errors
bool SyntaxCheck()
{
    int index = 0;
    while (index < tokens.size())
    {
        if (index < tokens.size() && tokens[index].token_type == HASH)
        {
            if (index + 1 < tokens.size() && tokens[index + 1].token_type == END_OF_FILE && index + 1 == tokens.size() - 1)
                return true;
            return false;
        }

        if (index >= tokens.size() || tokens[index].token_type != ID)
            return false;
        index++;

        if (index >= tokens.size() || tokens[index].token_type != ARROW)
            return false;
        index++;

        while (index < tokens.size() && tokens[index].token_type == ID)
            index++;

        if (index >= tokens.size() || tokens[index].token_type != STAR)
            return false;
        index++;
    }
    return true;
}

// read grammar
void ReadGrammar()
{
    LexicalAnalyzer analyze;

    // This will keep track of which rule we are referring to
    string current_rule = "";
    // Vector that stores the tokens that belongs to a current token
    std::vector<Token> current_tokens;

    Token token;
    do
    {
        token = analyze.GetToken();

        // keep track of the tokens that we acquired  at tokens
        tokens.push_back(token);

        if (token.token_type == TokenType::ARROW)
            continue;

        // this will keep track of the tokens that belong to the current_rule
        current_tokens.push_back(token);
        if (analyze.peek(1).token_type == TokenType::ARROW)
        {
            current_tokens.pop_back();
            if (current_tokens.size() > 1)
                current_tokens.pop_back(); // to get rid of the star
            // If there is a rule, then add it to the rules
            if (current_rule != "")
                rules.push_back({current_rule, current_tokens});
            current_rule = token.lexeme;
            current_tokens.clear();
        }

        // Reading the hash(end of file)
        if (analyze.peek(1).token_type == TokenType::HASH)
        {
            if (current_tokens.size() > 1)
                current_tokens.pop_back(); // to get rid of the star
            if (current_rule != "")
                rules.push_back({current_rule, current_tokens});
        }

    } while (token.token_type != TokenType::END_OF_FILE);

    // for (auto rule : rules) std::cout << rule.first << "\n";
    // std::cout << "\n\n\n\n\n\n";
}

// Auxiliary function for checking if it's a non_terminal
// by accessing rule.first where it contains the rules of the grammar
bool is_non_terminal(string text)
{
    for (auto rule : rules)
        if (rule.first == text)
            return true;
    return false;
}

// Task 1: Printing terminals and non_terminals
void printTerminalsAndNoneTerminals()
{
    // set to keep track of the string terminals/non-terminals
    std::unordered_set<string> done_set;

    // Printing terminals
    for (auto token : tokens)
    {
        // if the string is a non_terminal and it doesn't exist inside the set
        if (!is_non_terminal(token.lexeme) && done_set.count(token.lexeme) == 0 && token.token_type == ID)
        {
            std::cout << token.lexeme << " ";
            // if we have printed the terminal(string) then insert it into a set
            done_set.insert(token.lexeme);
        }
    }
    done_set.clear();

    // Printing nonterminals
    for (auto token : tokens)
    {
        // We are printing the nonterminals(string) that doesn't exist inside the set
        if (is_non_terminal(token.lexeme) && done_set.count(token.lexeme) == 0 && token.token_type == ID)
        {
            std::cout << token.lexeme << " ";
            // if we have printed the non_terminal(string) then insert it into a set
            done_set.insert(token.lexeme);
        }
    }
}

// for(auto token : pairs.second){
//     if(pairs.second.size() == 1 && pairs.second.at(0).token_type == TokenType::STAR){
//         new_tokens.push_back("*");
//     } else   new_tokens.push_back(token.lexeme);
// }

// A function that performs removing useless symbols
// We have created a separate function so that we can utilize it for predictive parser
void DoRemoveUseless()
{
    // 1. Calculate generating symbols
    // Remove non-generating symbols
    // new_rules vector will contain rules that only contain generating symbols
    std::unordered_set<std::string> terminates;

    // insert temrinals to the set
    for (auto rule : rules)
    {
        for (auto token : rule.second)
        {
            if (!is_non_terminal(token.lexeme))
                terminates.insert(token.lexeme);
        }
    }

    // boolean success will be used to check if a terminal is generating
    for (int i = 0; i < rules.size() + 1; i++)
    {
        for (auto rule : rules)
        {
            bool success = true;
            for (auto token : rule.second)
            {
                if (terminates.count(token.lexeme) == 0)
                    success = false;
            }
            if (success)
                terminates.insert(rule.first);
        }
    }

    for (auto rule : rules)
    {
        bool success = true;
        for (auto token : rule.second)
        {
            if (terminates.count(token.lexeme) == 0)
                success = false;
        }
        if (terminates.count(rule.first) != 0 && success)
            new_rules.push_back(rule);
    }

    // 2. Determine reachable symbols
    std::vector<std::string>
        new_tokens;

    // Do BFS. Make sure there are no loops
    queue<std::string> commands;
    reachable.insert(rules[0].first);
    commands.push(rules[0].first);

    while (!commands.empty())
    {
        std::string command = commands.front();
        commands.pop();

        for (auto rule : new_rules)
        {
            if (rule.first == command)
            {
                for (auto token : rule.second)
                {
                    if (is_non_terminal(token.lexeme) && reachable.count(token.lexeme) == 0)
                    {
                        reachable.insert(token.lexeme);
                        commands.push(token.lexeme);
                    }
                }
            }
        }
    }

    for (auto rule : new_rules)
    {
        if (reachable.count(rule.first) == 0 || terminates.count(rule.first) == 0)
        {
            is_useless = true;
            continue;
        }
    }
}

// Task 2: Eliminating usless symbols
void RemoveUselessSymbols()
{
    DoRemoveUseless();

    // Printing out the result of removing useless symbols
    for (auto rule : new_rules)
    {
        if (reachable.count(rule.first) == 0)
        {
            is_useless = true;
            continue;
        }

        cout << rule.first << " ->";
        for (auto token : rule.second)
        {
            if (token.lexeme != "")
                cout << " " << token.lexeme;
        }

        int count = 0;
        for (auto str : rule.second)
        {
            if (str.lexeme != "")
                count++;
        }

        if (count == 0)
            cout << " #";
        cout << "\n";
    }
}

bool hasHash(std::vector<string> &list)
{
    for (auto element : list)
        if (element == "")
            return true;
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
        if (first_hash_cache[tokens[index].lexeme])
            return FIRST_helper(tokens, index + 1, original);
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

// This is a helper function that will return a vector containing the first symbol
std::vector<string> FIRST(string key)
{
    if (!is_non_terminal(key))
        return {key};

    std::vector<string> to_return;
    for (auto rule : rules)
    {
        if (rule.first != key)
            continue;

        vector<string> helped_first = FIRST_helper(rule.second, 0, rule.first);
        to_return.insert(to_return.end(), helped_first.begin(), helped_first.end());
    }

    std::vector<string> to_cache;
    std::unordered_set<string> done_set;

    for (string t : to_return)
    {
        if (done_set.count(t) == 0)
            to_cache.push_back(t);
        done_set.insert(t);
    }

    first_cache[key] = to_cache;

    return first_cache[key];
}

// Task 3
void CalculateFirstSets(bool stop)
{
    for (auto token : tokens)
    {
        if (!is_non_terminal(token.lexeme))
            first_hash_cache[token.lexeme] = false;
    }

    for (int i = 0; i < 100; i++)
    {
        for (auto rule : rules)
            FIRST(rule.first);
    }

    if (stop)
        return;

    std::unordered_set<string> done_set;
    for (Token token : tokens)
    {
        if (!is_non_terminal(token.lexeme))
            continue;
        if (done_set.count(token.lexeme) > 0)
            continue;

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
        std::unordered_set<string> dupe_set;
        for (string s : lexemes)
            in_set.insert(s);

        for (Token t : tokens)
        {
            if (is_non_terminal(t.lexeme))
                continue;
            if (in_set.count(t.lexeme) == 0 || dupe_set.count(t.lexeme) != 0)
                continue;

            if (!first)
                std::cout << ", ";
            std::cout << t.lexeme;

            dupe_set.insert(t.lexeme);
            first = false;
        }

        std::cout << " }\n";

        done_set.insert(token.lexeme);
    }
}

void add_to_set(std::unordered_set<std::string> &a, std::unordered_set<std::string> &b)
{
    for (std::string s : a)
        b.insert(s);
}

// Task 4
void CalculateFollowSets(bool stop)
{
    for (int i = 0; i < rules.size(); i++)
    {
        for (auto rule : rules)
            FIRST(rule.first);
    }

    std::unordered_map<std::string, std::unordered_set<std::string>> follow;
    for (auto rule : rules)
        follow[rule.first] = {};

    // Inserting the $ to starting symbol
    follow[rules[0].first].insert("$");

    for (int i = 0; i < rules.size(); i++)
    {
        // RULE II & RULE III
        for (auto rule : rules)
        {
            int index = rule.second.size() - 1;
            while (index >= 0)
            {
                if (index + 1 < rule.second.size() && !first_hash_cache[rule.second[index + 1].lexeme])
                    break;
                if (is_non_terminal(rule.second[index].lexeme))
                    add_to_set(follow[rule.first], follow[rule.second[index].lexeme]);
                index--;
            }
        }

        // RULE IV
        for (auto rule : rules)
        {
            bool check = false;
            int index = 0;

            for (int i = 0; i < rule.second.size(); i++)
            {
                if (check)
                {
                    vector<std::string> first = FIRST(rule.second[i].lexeme);
                    for (auto str : first)
                        follow[rule.second[i - 1].lexeme].insert(str);
                    check = false;
                }
                else
                    check = false;
                if (is_non_terminal(rule.second[i].lexeme))
                    check = true;
            }
        }

        // RULE V
        for (auto rule : rules)
        {
            for (int i = 0; i < rule.second.size(); i++)
            {
                for (int j = i + 1; j < rule.second.size(); j++)
                {
                    auto first = FIRST(rule.second[j].lexeme);
                    for (auto str : first)
                        follow[rule.second[i].lexeme].insert(str);
                    if (!first_hash_cache[rule.second[j].lexeme])
                        break;
                }
            }
        }
    }

    if (stop)
        return;
    // PRINT IT
    std::unordered_set<std::string> done_set;
    for (auto token1 : tokens)
    {
        if (!is_non_terminal(token1.lexeme))
            continue;
        if (done_set.count(token1.lexeme) != 0)
            continue;
        done_set.insert(token1.lexeme);

        std::cout << "FOLLOW(" << token1.lexeme << ") = { ";

        bool first = true;
        if (follow[token1.lexeme].count("$") != 0)
        {
            std::cout << "$";
            first = false;
        }

        std::unordered_set<std::string> dupe_set = follow[token1.lexeme];
        for (auto token : tokens)
        {
            if (dupe_set.count(token.lexeme) == 0)
                continue;
            if (!first)
                std::cout << ", ";

            std::cout << token.lexeme;

            dupe_set.erase(token.lexeme);

            first = false;
        }

        std::cout << " }\n";
    }
}

// Task 5
bool CheckIfGrammarHasPredictiveParser()
{
    DoRemoveUseless();

    
    for (auto rule : new_rules)
    {
        if (reachable.count(rule.first) == 0)
        {
            is_useless = true;
            continue;
        }

        int count = 0;
        for (auto str : rule.second)
        {
            if (str.lexeme != "")
                count++;
        }
    }

    if (is_useless) return false;
    if (new_rules.size() != rules.size()) return false;

    CalculateFirstSets(true);
    CalculateFollowSets(true);

    // CONDITION 1
    for (int i = 0; i < rules.size(); i++)
    {
        unordered_set<std::string> set_1;

        int index1 = 0;
        bool falsed = false;
        while (index1 < rules[i].second.size())
        {
            auto a = FIRST(rules[i].second[index1].lexeme);
            for (auto b : a)
                set_1.insert(b);

            if (first_hash_cache[rules[i].second[index1].lexeme])
                falsed = true;
            if (!first_hash_cache[rules[i].second[index1].lexeme] || !is_non_terminal(rules[i].second[index1].lexeme))
                break;

            index1++;
        }

        for (int j = i + 1; j < rules.size(); j++)
        {
            if (rules[j].first != rules[i].first)
                continue;

            int index2 = 0;
            while (index2 < rules[j].second.size())
            {
                auto a = FIRST(rules[j].second[index2].lexeme);
                for (auto b : a)
                    if (set_1.count(b) != 0)
                        return false;

                if (falsed && first_hash_cache[rules[j].second[index2].lexeme])
                    return false;
                if (!first_hash_cache[rules[j].second[index2].lexeme] || !is_non_terminal(rules[j].second[index2].lexeme))
                    break;
                index2++;
            }
        }
    }

    // CONDITION 2
    for (auto rule : rules)
    {
        if (first_hash_cache[rule.first])
        {
            unordered_set<std::string> first_a;
            unordered_set<std::string> follow_a = globow[rule.first];

            if (follow_a.count("$") != 0) return false;

            vector<std::string> first_a_vect = FIRST(rule.first);
            for (auto a : first_a_vect)
                if (follow_a.count(a) != 0)
                    return false;
        }
    }

    return true;
}

int main(int argc, char *argv[])
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

    ReadGrammar(); // Reads the input grammar from standard input
                   // and represent it internally in data structures
                   // ad described in project 2 presentation file

    if (!SyntaxCheck())
    {
        printf("SYNTAX ERROR !!!\n");
        return 0;
    }

    switch (task)
    {
    case 1:
        printTerminalsAndNoneTerminals();
        break;

    case 2:
        RemoveUselessSymbols();
        break;

    case 3:
        CalculateFirstSets(false);
        break;

    case 4:
        CalculateFollowSets(false);
        break;

    case 5:
        if (CheckIfGrammarHasPredictiveParser())
            std::cout << "YES";
        else
            std::cout << "NO";
        break;

    default:
        cout << "Error: unrecognized task number " << task << "\n";
        break;
    }
    return 0;
}
