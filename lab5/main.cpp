#include <algorithm>
#include <fstream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <stack>
#include <string_view>
#include <vector>

template <class T>
struct Option{
    bool some;
    T value;

    T unwrap(){
        if (some) return value;
        assert(false);
    }
};


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

void Scanner_next_raw(Scanner* scanner, usize n = 1){
    for (usize i = 0; i < n; i++){
        if (scanner->source.empty()) scanner->current = 0;
        else scanner->current = scanner->source[0];
        scanner->source.remove_prefix(1);

        //if (std::isspace(scanner->current)) i--; // skip whitespace
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
        Scanner_next_raw(scanner);
        std::string str;

        while (scanner->current != '\"') {
            str += scanner->current;
            Scanner_next_raw(scanner);
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
            production.elems.push_back(elem);
            //add_to_elems(&production.elems, elem);
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

Option<Grammar_Production> Grammar_get_production(Grammar grammar, Grammar_Elem nonterminal, usize i = 0){
    assert(nonterminal.kind == Grammar_Elem::Kind::Nonterminal);
    for (let prod : grammar.productions){
        if (prod.nonterminal.str == nonterminal.str){
            if (i == 0)
                return {true, prod};
            else i--;
        }
    }

    return {false};
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

enum struct RDP_State{
    Normal,
    Final,
    Back,
    Error,
};

struct RDP_Elem{
    Grammar_Elem elem;

    /* if elem is a nonterminal we need to store which 
     * production needs to be used because if we
     * go back we need to move to the next one */
    usize production_i = 0; 
};

struct Recursive_Descent_Parser{
    RDP_State state;
    usize i = 0;
    std::vector<RDP_Elem> working_stack;
    std::vector<RDP_Elem> input_stack;

    std::string sequence;
    Grammar grammar;
};

Recursive_Descent_Parser RDP_create(Grammar grammar, std::string sequence){
    let rdp =  Recursive_Descent_Parser{
        .state = RDP_State::Normal,
        .i = 0,
        .sequence = sequence,
        .grammar = grammar,
    };
    rdp.input_stack.push_back({grammar.starting_nonterminal, 0});
    return rdp;
}

void RDP_step(Recursive_Descent_Parser* rdp){
    switch (rdp->state) {
    case RDP_State::Normal:{
        if (rdp->input_stack.empty()){
            if (rdp->i == rdp->sequence.size()) {
                rdp->state = RDP_State::Final;
            } else {
                rdp->state = RDP_State::Error;
            }
            break;
        }


        let head = rdp->input_stack.back();
        switch (head.elem.kind) {
        case Grammar_Elem::Kind::Nonterminal:{
            rdp->input_stack.pop_back();

            let production = Grammar_get_production(
                rdp->grammar, 
                head.elem, head.production_i
            ).unwrap();


            for (let i = 0; i < production.elems.size(); i++){
                let pos = production.elems.size() - i - 1;
                rdp->input_stack.push_back(RDP_Elem{
                    .elem = production.elems[pos],
                    .production_i = 0,
                });
            }

            rdp->working_stack.push_back(head);

        }break;
        case Grammar_Elem::Kind::Terminal:{
            let terminal = head.elem.str[0]; // terminal is a character
            if (terminal == rdp->sequence[rdp->i]){
                rdp->input_stack.pop_back();
                rdp->i++;
                rdp->working_stack.push_back(head);
            }else {
                rdp->state = RDP_State::Back;
            }
        }break;
        }
    }break;

    case RDP_State::Back:{
        let head = &rdp->working_stack.back();
        switch (head->elem.kind) {
        case Grammar_Elem::Kind::Nonterminal:{
            //clean top
            {
                let production = Grammar_get_production(
                    rdp->grammar, 
                    head->elem, head->production_i
                ).unwrap();
                for (let i = 0; i < production.elems.size(); i++){
                    rdp->input_stack.pop_back();
                }
            }

            head->production_i++;

            let production_option = Grammar_get_production(
                rdp->grammar, 
                head->elem, head->production_i
            );

            if (production_option.some){
                let production = production_option.value;
                for (let i = 0; i < production.elems.size(); i++){
                    let pos = production.elems.size() - i - 1;
                    rdp->input_stack.push_back(RDP_Elem{
                        .elem = production.elems[pos],
                        .production_i = 0,
                    });
                }
                rdp->state = RDP_State::Normal;
            } else if (rdp->grammar.starting_nonterminal.str == head->elem.str && rdp->i == 0){
                rdp->state = RDP_State::Error;
            } else {
                rdp->working_stack.pop_back();
                rdp->input_stack.push_back({head->elem, 0});
            }
        }break;
        case Grammar_Elem::Kind::Terminal:{
            rdp->i--;
            rdp->working_stack.pop_back();
            rdp->input_stack.push_back(*head);
        }break;
        }

    }break;

    case RDP_State::Error:{

    }break;

    case RDP_State::Final:{

    }break;
    }
}

void RDP_print(Recursive_Descent_Parser rdp){
    let tab = [](){std::cout << "    ";};
    std::cout << "Recursive_Descent_Parser{" << std::endl;
    tab();
    std::cout << ".state = ";
    switch (rdp.state) {
    case RDP_State::Normal: std::cout << "Normal" << std::endl; break;
    case RDP_State::Back: std::cout << "Back" << std::endl; break;
    case RDP_State::Error: std::cout << "Error" << std::endl; break;
    case RDP_State::Final: std::cout << "Final" << std::endl; break;
    }
    tab();
    std::cout << ".i = " << rdp.i << std::endl;
    tab();
    std::cout << ".sequence = " << rdp.sequence << std::endl;
    tab();
    std::cout << ".input_stack = [" << std::endl;
    for (let el : rdp.input_stack){
        tab(); tab();
        std::cout << el.elem.str << ", "<< std::endl;
    }               
    tab();
    std::cout << "]" <<std::endl;
    tab();
    std::cout << ".working_stack = [ " << std::endl;
    for (let el : rdp.working_stack){
        tab(); tab();
        std::cout << el.elem.str << ", "<< std::endl;
    }               
    tab();
    std::cout << "]" <<std::endl;
    std::cout << "}" <<std::endl;
                    
}

struct Node{
    Grammar_Elem elem;
    std::vector<Node> children;
};

void node_print(Node node, usize tab = 0){
    let print_tab = [](usize tab){
        for (let i = 0; i < tab; i++)
            std::cout << " ";
    };

    print_tab(tab);
    std::cout << node.elem.str;
    if (node.elem.kind == Grammar_Elem::Kind::Nonterminal){
        std::cout << " -> {" << std::endl;
        for (let child : node.children) {
            print_tab(tab + 1);
            node_print(child, tab + 1);
        }
        print_tab(tab);
        std::cout << "}";
    }
    std::cout << "," << std::endl;
}


void rdp_create_tree_aux(Node* current, Recursive_Descent_Parser parser, usize* i){
    let rdp_elem = parser.working_stack[*i];
    let grammar_elem = rdp_elem.elem;
    assert(grammar_elem.kind == Grammar_Elem::Kind::Nonterminal);
    current->elem = grammar_elem;

    let production = Grammar_get_production(parser.grammar, grammar_elem, rdp_elem.production_i)
                        .unwrap();

    for (let prod_elem : production.elems){
        (*i)++;
        let g = parser.working_stack[*i];
        assert(g.elem.kind == prod_elem.kind);

        if (prod_elem.kind == Grammar_Elem::Kind::Terminal){
            let node = Node{
                .elem = g.elem,
            };
            current->children.push_back(node);
        } else if (prod_elem.kind == Grammar_Elem::Kind::Nonterminal){
            let node = Node{
                .elem = g.elem,
            };
            rdp_create_tree_aux(&node, parser, i);
            current->children.push_back(node);
        }
    }
}

Node rdp_create_tree(Recursive_Descent_Parser parser){
    assert(parser.state == RDP_State::Final);

    Node root;
    usize i = 0;
    rdp_create_tree_aux(&root, parser, &i);
    return root;
}

Recursive_Descent_Parser rdp_parse(Grammar grammar, std::string sequence){
    let rdp = RDP_create(grammar, sequence);
    while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
        //RDP_print(rdp);
        RDP_step(&rdp);
    }

    return rdp;
}

void test(){
    let grammar = Grammar_read_from_file("g1.txt");
    {
        let rdp = RDP_create(grammar, "ac");
        while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
            RDP_step(&rdp);
        }
        assert(rdp.state == RDP_State::Final);
    }
    {
        let rdp = RDP_create(grammar, "acbc");
        while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
            RDP_step(&rdp);
        }
        assert(rdp.state == RDP_State::Final);
    }
    {
        let rdp = RDP_create(grammar, "aacbc");
        while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
            RDP_step(&rdp);
        }
        assert(rdp.state == RDP_State::Final);
    }
    {
        let rdp = RDP_create(grammar, "ca");
        while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
            RDP_step(&rdp);
        }
        assert(rdp.state == RDP_State::Error);
    }
    {
        let rdp = RDP_create(grammar, "a");
        while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
            RDP_step(&rdp);
        }
        assert(rdp.state == RDP_State::Error);
    }
    {
        let rdp = RDP_create(grammar, "aca");
        while (rdp.state != RDP_State::Error && rdp.state != RDP_State::Final) {
            RDP_step(&rdp);
        }
        assert(rdp.state == RDP_State::Error);
    }
}

int main(){
    //test();

    let grammar = Grammar_read_from_file("minilang.txt");
    let rdp     = rdp_parse(grammar, "let a []i32 = 1;");
    let root    = rdp_create_tree(rdp);
    node_print(root);
}
