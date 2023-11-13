#include <cassert>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define let   auto
using usize = size_t;
using u8 = uint8_t;
using u32 = uint32_t;

struct FA{
    struct Edge{
        char letter;
        char start;
        char end;
    };

    std::vector<Edge> edges;
    std::unordered_set<char>  final_states;
    std::unordered_set<char> states;
    std::unordered_set<char> alphabet;

    char initial_state;

void add_edge(char start, char end, char letter){
    states.insert(start);
    states.insert(end);
    alphabet.insert(letter);

    this->edges.push_back({
        .letter = letter,
        .start  = start,
        .end    = end,
    });
}

void set_initial_state(char state){
    states.insert(state);
    this->initial_state = state;
}

void add_final_state(char state){
    states.insert(state);
    this->final_states.insert(state);
}

bool is_final_state(char state){
    return this->final_states.find(state) != this->final_states.end();
}

bool accept(std::string str){
    struct Node{
        char             state;
        std::string_view remaining;
    };
    
    let initial = Node{
        .state     = this->initial_state,
        .remaining = std::string_view{str},
    };

    std::stack<Node> s;
    s.push(initial);

    while (!s.empty()) {
        let current = s.top();
        s.pop();
        if (current.remaining.empty() && is_final_state(current.state)) return true;

        for (let edge : this->edges){
            if (edge.start == current.state &&
                edge.letter == current.remaining[0]){
                s.push({
                    .state = edge.end,
                    .remaining = current.remaining.substr(1, current.remaining.size() - 1),
                });
            }
        }
    }

    return false;
}


static
FA read_from_file(std::string file){
    struct FA_Scanner{
        std::string_view source;
        char current = 0;

    void next(usize n = 1){
        for (usize i = 0; i < n; i++){
            if (source.empty()) current = 0;
            else current = source[0];
            source.remove_prefix(1);

            if (std::isspace(current)) i--; // skip whitespace
        }
    }


    char current_expect(char chr){
        assert(chr == this->current && "current_expect");
        return this->current;
    }

    char current_expect_character(){
        let greater    = 'A' <= current && current <= 'Z';
        let smaller    = 'a' <= current && current <= 'z';
        let digit      = '0' <= current && current <= '9';
        let underscore = current == '_';
        let plus       = current == '+';
        let minus      = current == '-';
        if (greater || smaller || digit || underscore || plus || minus) 
            return this->current;
        std::cout << "Error: current was " << current;
        exit(-1);
    }

    };

    std::ifstream input{file};
    std::stringstream buffer;
    buffer << input.rdbuf();
    let source = buffer.str();

    let scanner = FA_Scanner{
        .source  = std::string_view{source},
    };
    scanner.next();

    let fa = FA{};

    let initial_state = scanner.current_expect_character();          
    scanner.next();
    scanner.current_expect(';');
    scanner.next();
    fa.set_initial_state(initial_state);

    while (true){
        let final_state = scanner.current_expect_character();
        scanner.next();
        fa.add_final_state(final_state);
        if (scanner.current == ';') break;
    }
    scanner.next();

    while (true) {
        if (scanner.current == ';') break;
        let letter = scanner.current_expect_character();       
        scanner.next();
        scanner.current_expect(':');
        scanner.next();

        let start = scanner.current_expect_character();       
        scanner.next();

        let end = scanner.current_expect_character();       
        scanner.next();

        fa.add_edge(start, end, letter);
    }
    

    return fa;
}

};

