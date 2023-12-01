#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <string_view>
#include <vector>

using uint = unsigned int;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = std::size_t;
#define let auto

struct Scanner{
    std::string_view source;
    char current = 0;
};


void Scanner_next(Scanner* scanner, usize n = 1){
    for (usize i = 0; i < n; i++){
        if (scanner->source.empty()) scanner->current = 0;
        else scanner->current = scanner->source[0];
        scanner->source.remove_prefix(1);

        if (std::isspace(scanner->current)) i--; // skip whitespace
    }
}


char Scanner_current_expect(Scanner scanner, char chr){
    assert(chr == scanner.current && "current_expect");
    return scanner.current;
}

char Scanner_current_expect_character(Scanner* scanner){
    let greater    = 'A' <= scanner->current && scanner->current <= 'Z';
    let smaller    = 'a' <= scanner->current && scanner->current <= 'z';
    let digit      = '0' <= scanner->current && scanner->current <= '9';
    let underscore = scanner->current == '_';
    if (greater || smaller || digit || underscore) 
        return scanner->current;
    std::cout << "Error: scanner->current was " << scanner->current;
    exit(-1);
}

bool Scanner_match(Scanner* scanner, std::string str){
    let snapshot = *scanner;

    for (char chr: str) {
        if (scanner->current != chr){
            *scanner = snapshot;
            return false;
        }
        Scanner_next(scanner);
    }

    return true;
}

std::string Scanner_until(Scanner* scanner, char chr){
    std::string str;

    while (scanner->current != chr && scanner->current != 0) {
        str += scanner->current;
        Scanner_next(scanner);
    }

    return str;
}

struct Grammar_Elem{
    enum struct Kind{
        Terminal,
        Nonterminal,
    } kind;

    std::string str;
};

struct Grammar_Production{
    Grammar_Elem nonterminal;
    std::vector<Grammar_Elem> elems;
};

struct Grammar{
    Grammar_Elem starting_nonterminal;
    std::vector<Grammar_Elem>       elems;
    std::vector<Grammar_Production> productions;
};




Grammar Grammar_read_from_file(std::string path){
    let scan_non_terminal = [](Scanner* scanner) -> Grammar_Elem{
        let is_non_terminal_character = [](char chr){
            let greater    = 'A' <= chr && chr <= 'Z';
            let smaller    = 'a' <= chr && chr <= 'z';
            let digit      = '0' <= chr && chr <= '9';
            let underscore = chr == '_';
            return (greater || smaller || digit || underscore);
        };

        std::string str;

        while (is_non_terminal_character(scanner->current)) {
            str += scanner->current;
            Scanner_next(scanner);
        }

        return {
            .kind = Grammar_Elem::Kind::Nonterminal,
            .str = str,
        };
    };
    let scan_terminal = [](Scanner* scanner) -> Grammar_Elem{
        Scanner_current_expect(*scanner, '\"');
        Scanner_next(scanner);
        std::string str;

        while (scanner->current != '\"') {
            str += scanner->current;
            Scanner_next(scanner);
        }
        Scanner_next(scanner);

        return {
            .kind = Grammar_Elem::Kind::Terminal,
            .str = str,
        };
    };

    let add_to_elems = [](std::vector<Grammar_Elem>* v, Grammar_Elem elem){
        let pos = std::find_if(
            v->begin(),
            v->end(),
            [&elem](Grammar_Elem elem_in_v){
                return elem.kind == elem_in_v.kind &&
                       elem.str  == elem_in_v.str;
            }
        );
        if (pos == v->end())
            v->push_back(elem);
    };

    let grammar = Grammar{};
    
    std::ifstream input{path};
    std::stringstream buffer{};
    buffer << input.rdbuf();
    let source = buffer.str();

    let scanner = Scanner{
        .source = source,
    };
    Scanner_next(&scanner);

    //assert(Scanner_match(&scanner, "nonterminals:"));
    //while (scanner.current != ';') {
    //    let non_terminal = scan_non_terminal(&scanner);
    //    grammar.elems.push_back(non_terminal);
    //    Scanner_current_expect(scanner, ',');
    //    Scanner_next(&scanner);
    //}
    //Scanner_next(&scanner);
    //
    //assert(Scanner_match(&scanner, "terminals:"));
    //while (scanner.current != ';') {
    //    let terminal = scan_terminal(&scanner);
    //    grammar.elems.push_back(terminal);
    //    Scanner_current_expect(scanner, ',');
    //    Scanner_next(&scanner);
    //}
    //Scanner_next(&scanner);

    assert(Scanner_match(&scanner, "starting_nonterminal:"));
    let starting = scan_non_terminal(&scanner);
    Scanner_current_expect(scanner, ';');
    Scanner_next(&scanner);
    grammar.starting_nonterminal = starting;


    assert(Scanner_match(&scanner, "productions:"));
    while (scanner.current != ';') {
        let nonterminal = scan_non_terminal(&scanner);
        Scanner_current_expect(scanner, ':');
        Scanner_next(&scanner);

        let production = Grammar_Production{
            .nonterminal = nonterminal,
        };

        while (scanner.current != ';') {
            Grammar_Elem elem;
            if (scanner.current == '\"')
                elem = scan_terminal(&scanner);
            else 
                elem = scan_non_terminal(&scanner);
            //production.elems.push_back(elem);
            add_to_elems(&production.elems, elem);
            add_to_elems(&grammar.elems, elem);

            Scanner_current_expect(scanner, ',');
            Scanner_next(&scanner);
        }
        Scanner_next(&scanner);

        grammar.productions.push_back(production);
    }

    return grammar;
}

void Grammar_print(Grammar grammar){
    std::cout << "nonterminals:" << std::endl;
    for (let elem : grammar.elems){
        if (elem.kind == Grammar_Elem::Kind::Nonterminal){
            std::cout << "    " << elem.str << "," << std::endl;
        }   
    }
    std::cout << ";" << std::endl;

    std::cout << "terminals:" << std::endl;
    for (let elem : grammar.elems){
        if (elem.kind == Grammar_Elem::Kind::Terminal){
            std::cout << "    \"" << elem.str << "\"," << std::endl;
        }   
    }
    std::cout << ";" << std::endl;

    std::cout << "productions:" << std::endl;
    for (let production : grammar.productions){
        std::cout << "    " << production.nonterminal.str << ":" << std::endl;
        for (let elem : production.elems){
            if (elem.kind == Grammar_Elem::Kind::Terminal)
                std::cout << "        \"" << elem.str << "\"," << std::endl;
            else 
                std::cout << "        " << elem.str << "," << std::endl;
        }
        std::cout << "    ;" << std::endl;
    }
    std::cout << ";" << std::endl;
    std::cout << "starting nonterminal: " << grammar.starting_nonterminal.str << std::endl;
}

bool Grammar_cfg_check(Grammar grammar){
    let check = [](std::vector<Grammar_Elem>* v, Grammar_Elem elem){
        let pos = std::find_if(
            v->begin(),
            v->end(),
            [&elem](Grammar_Elem elem_in_v){
                return elem.kind == elem_in_v.kind &&
                       elem.str  == elem_in_v.str;
            }
        );
        return pos != v->end();
    };
    for (let& production : grammar.productions){
        if (!check(&grammar.elems, production.nonterminal) && 
            production.nonterminal.kind == Grammar_Elem::Kind::Nonterminal) 
            return false;

        for (let elem : production.elems){
            if (!check(&grammar.elems, elem)) 
                return false;
        }
    }

    return true;
}

int main(){
    let grammar = Grammar_read_from_file("g1.txt");
    Grammar_print(grammar);
    std::cout << "is cfg: " << Grammar_cfg_check(grammar) << std::endl;
}
