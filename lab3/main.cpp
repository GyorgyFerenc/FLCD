#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <fstream>

#define let   auto
using usize = size_t;
using u8 = uint8_t;

template <class T>
T* typed_malloc(usize size){
    return (T*)malloc(sizeof(T) * size);
}

template <class T>
T* typed_realloc(T* ptr, usize size){
    return (T*)realloc(ptr, sizeof(T) * size);
}


template <class T>
void typed_memset(T* ptr, T value, usize len){
    for (usize i = 0; i < len; i++){
        ptr[i] = value;
    }
}


struct Hash_Table{
    struct Key{
        usize index = 0;
        enum class Kind: u8{
            Free = 0,
            Used,
        } kind = Kind::Free;
    };
    struct Value{
        std::string str;
    };

    usize   capacity;
    usize   size;
    Key*    keys;
    Value*  values;

    static Hash_Table create(usize capacity){
        Hash_Table table;
        table.capacity  = capacity;
        table.size      = 0;
        table.keys      = new Key[capacity]; 
        table.values    = new Value[capacity];
        return table;
    }

    void destroy(){
        delete[] this->keys;
        delete[] this->values;
    }

    usize add(std::string str){
        if (this->size + 1 >= this->capacity){
            recapacity(this->capacity * 2);
        }

        let key_index = hash_to_key_index(str, this->keys, this->values, this->capacity);
        let value_index = this->keys[key_index].index;
        if (this->values[value_index].str == str){
            return value_index;
        }

        let value_pos = this->size;
        this->size++;
        this->values[value_pos].str = str;
        this->keys[key_index].index = value_pos;
        this->keys[key_index].kind  = Key::Kind::Used;

        return value_pos;
    }

    std::string get(usize index){
        return this->values[index].str;       
    }

private:
    usize hash_to_key_index(std::string str, Key* keys, Value* values, usize capacity){
        /*
         * Currently does linear probing
         */
        let key_index = this->hash(str) % capacity;
        let i = 0;
        while (keys[key_index].kind != Key::Kind::Free){
            let index = keys[key_index].index;
            if (values[index].str == str) return key_index;

            i++;
            key_index = (this->hash(str) + this->prob(i)) % capacity;
        }
        return key_index;
    }

    void recapacity(usize new_capacity){
        let new_keys   = new Key[new_capacity];
        let new_values = new Value[new_capacity];

        for (usize i = 0; i < this->size; i++)
            new_values[i] = this->values[i];

        for (usize i = 0; i < this->capacity; i++){
            let key = this->keys[i];
            if (key.kind == Key::Kind::Used){
                let key_index = hash_to_key_index(new_values[key.index].str, new_keys, new_values, new_capacity);
                new_keys[key_index] = key;
            }
        }
        
        delete[] this->values;
        delete[] this->keys;
        this->values   = new_values;
        this->keys     = new_keys;
        this->capacity = new_capacity;
    }


    inline
    usize hash(std::string str){
        std::hash<std::string> hasher;
        return hasher(str);
    }
    inline
    usize prob(usize i){
        return i;
    }

};

struct Symbol_Table{
    Hash_Table identifiers;
    Hash_Table constants;
    
    static Symbol_Table create(usize capacity){
        return {
            .identifiers = Hash_Table::create(capacity),
            .constants   = Hash_Table::create(capacity),
        };
    }
};


struct Token{
    enum class Kind{
        End_Of_File,

        Identifer,
        Constant,

        Operator_Not, // !
        Operator_Reference, // &
        Operator_Plus, // +
        Operator_Minus, // -
        Operator_Multiply, // *
        Operator_Divide, // /
        Operator_Assign, // = 
        Operator_Smaller, // <
        Operator_Greater, // >
        Operator_Smaller_Equal, // <=
        Operator_Greater_Equal, // >=
        Operator_Not_Equal, // != 
        Operator_Equal, // == 
        Operator_Modulo, // %

        Separator_Semicolon, // ;
        Separator_Curly_Open, // {
        Separator_Curly_Close, // }
        Separator_Round_Open, // (
        Separator_Round_Close, // )
        Separator_Square_Open, // [
        Separator_Square_Close, //]
        Separator_Comma, // ,

        Keyword_Let,
        Keyword_If,
        Keyword_Else,
        Keyword_Do,
        Keyword_For,
        Keyword_I32,
        Keyword_Bool,
        Keyword_In,
        Keyword_Read,
        Keyword_Print,
        Keyword_True,
        Keyword_False,
    } kind;
    std::string str;

    union {
        struct {
            usize id;
        } identifier;
        struct {
            usize id;
        } constant;
    };

};

using PIF = std::vector<Token>;

struct Scanner{
    Symbol_Table table;
    std::string source;
    PIF   pif;
    char  current        = 0;
    char  peek           = 0;
    usize source_index   = 0;

static
Scanner create(std::string source){
    let scanner = Scanner {
        .table = Symbol_Table::create(100),
        .source = source,
    };

    // TODO(Ferenc): handle source.size() < 2
    scanner.current      = source[0];
    scanner.peek         = source[1];
    scanner.source_index = 2;

    return scanner;
}


#define SCAN_ONE(c, k)                         \
        {                                      \
            if (current == c){                 \
                pif.push_back({                \
                    .kind = k,                 \
                    .str = std::string{c},   \
                });                            \
                next();                        \
                continue;                      \
            }                                  \
        }                                      \

#define SCAN_TWO(c, p, k)                      \
        {                                      \
            if (current == c && peek == p){    \
                pif.push_back({                \
                    .kind = k,                 \
                    .str = std::string{c, p},  \
                });                            \
                next(2);                       \
                continue;                      \
            }                                  \
        }                                      

#define SCAN_KEYWORD(str, expected, k)  \
        {                               \
            if (str == expected){       \
                pif.push_back({         \
                    .kind = k,          \
                    .str = str,         \
                });                     \
                continue;               \
            }                           \
        }
                                       
                                        
void all(){                             
    while (true) {
        skip_whitespace();
        if (current == 0) {
            pif.push_back({
                .kind = Token::Kind::End_Of_File,
            });
            return;
        }
        SCAN_TWO('<', '=', Token::Kind::Operator_Smaller_Equal);
        SCAN_TWO('>', '=', Token::Kind::Operator_Greater_Equal);
        SCAN_TWO('!', '=', Token::Kind::Operator_Not_Equal);
        SCAN_TWO('=', '=', Token::Kind::Operator_Equal);

        SCAN_ONE('!',      Token::Kind::Operator_Not);
        SCAN_ONE('&',      Token::Kind::Operator_Reference);
        SCAN_ONE('+',      Token::Kind::Operator_Plus);
        SCAN_ONE('-',      Token::Kind::Operator_Minus);
        SCAN_ONE('*',      Token::Kind::Operator_Multiply);
        SCAN_ONE('/',      Token::Kind::Operator_Divide);
        SCAN_ONE('=',      Token::Kind::Operator_Assign);
        SCAN_ONE('<',      Token::Kind::Operator_Smaller);
        SCAN_ONE('>',      Token::Kind::Operator_Greater);
        SCAN_ONE('%',      Token::Kind::Operator_Modulo);

        SCAN_ONE(';',      Token::Kind::Separator_Semicolon);
        SCAN_ONE('{',      Token::Kind::Separator_Curly_Open);
        SCAN_ONE('}',      Token::Kind::Separator_Curly_Close);
        SCAN_ONE('(',      Token::Kind::Separator_Round_Open);
        SCAN_ONE(')',      Token::Kind::Separator_Round_Close);
        SCAN_ONE('[',      Token::Kind::Separator_Square_Open);
        SCAN_ONE(']',      Token::Kind::Separator_Square_Close);
        SCAN_ONE(',',      Token::Kind::Separator_Comma);

        std::string str = "";
        while (!is_separator_or_operator(current)) {
            str += current;
            next();
        }

        SCAN_KEYWORD(str, "let",   Token::Kind::Keyword_Let);
        SCAN_KEYWORD(str, "if",    Token::Kind::Keyword_If);
        SCAN_KEYWORD(str, "else",  Token::Kind::Keyword_Else);
        SCAN_KEYWORD(str, "do",    Token::Kind::Keyword_Do);
        SCAN_KEYWORD(str, "for",   Token::Kind::Keyword_For);
        SCAN_KEYWORD(str, "i32",   Token::Kind::Keyword_I32);
        SCAN_KEYWORD(str, "bool",  Token::Kind::Keyword_Bool);
        SCAN_KEYWORD(str, "in",    Token::Kind::Keyword_In);
        SCAN_KEYWORD(str, "read",  Token::Kind::Keyword_Read);
        SCAN_KEYWORD(str, "print", Token::Kind::Keyword_Print);

        //SCAN_KEYWORD(str, "true",  Token::Kind::Keyword_True);
        //SCAN_KEYWORD(str, "false", Token::Kind::Keyword_False);

        { // bool constant
            if (str == "true" || str == "false"){
                let index = table.constants.add(str);
                pif.push_back({
                    .kind = Token::Kind::Constant,
                    .str = str,
                    .constant = {
                        .id = index,
                    },
                });
                continue;
            }
        }

        { // integer constant
            std::regex int_constant{"0|[+-]?[1-9][0-9]*"};
            let is_match = std::regex_match(str, int_constant);
            if (is_match) {
                let index = table.constants.add(str);
                pif.push_back({
                    .kind = Token::Kind::Constant,
                    .str = str,
                    .constant = {
                        .id = index,
                    },
                });
                continue;
            }
        }
        { // identifier
            std::regex identifier{"[A-Za-z_][A-Za-z_0-9]*"};
            let is_match = std::regex_match(str, identifier);
            if (is_match) {
                let index = table.identifiers.add(str);
                pif.push_back({
                    .kind = Token::Kind::Identifer,
                    .str = str,
                    .identifier = {
                        .id = index,
                    },
                });
                continue;
            }
        }

        lexical_error(str);
    }
}

bool is_operator(char c){
    switch (c) {
        case '<': return true;
        case '>': return true;
        case '=': return true;
        case '!': return true;
        case '&': return true;
        case '+': return true;
        case '-': return true;
        case '*': return true;
        case '/': return true;
        case '%': return true;
    }        

    return false;
}

bool is_separator(char c){
    if (std::isspace(c)) return true;
    switch (c) {
        case ';': return true;
        case '{': return true;
        case '}': return true;
        case '(': return true;
        case ')': return true;
        case '[': return true;
        case ']': return true;
        case ',': return true;
    }
    return false;
}

bool is_separator_or_operator(char c){
    return is_separator(c) || is_operator(c);
}

void next(usize nr){
    for (usize i = 0; i < nr; i++){
        current = peek;
        if (source_index >= source.size()) {
            peek = 0;
            return;
        }
        peek = source[source_index];
        source_index++;
    }
}

void next(){
    next(1);
}

void skip_whitespace(){
    while (std::isspace(current)) {
        next();
    }
}

void lexical_error(std::string str){
    usize line = 1;
    for (usize i = 0; i < this->source_index; i++) {
        if (this->source[i] == '\n'){
            line++;
        }
    }
    std::cout << "Lexical error at line " << line << " at token: " << str << std::endl;
    exit(-1);
}


};


void print_pif(PIF pif){
    for (let token : pif){
        std::cout << "Token{" << std::endl;
        std::cout << "    .kind = " << (usize)token.kind << std::endl;
        std::cout << "    .str  = \"" << token.str << "\"" << std::endl;
        std::cout << "}" << std::endl;
    }
}

int main(int argc, char** argv){
    if (argc == 1){
        let scanner = Scanner::create(" let a = 12;");
        scanner.all();
        print_pif(scanner.pif);
        return 0;
    }

    std::ifstream input{argv[1]};
    std::stringstream buffer;
    buffer << input.rdbuf();
    let source = buffer.str();
    let scanner = Scanner::create(source);
    scanner.all();
    print_pif(scanner.pif);
}
