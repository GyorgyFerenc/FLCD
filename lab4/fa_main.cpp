#include "FA.hpp"

int main(){
    let fa = FA{};
    bool running = true;
    while (running) {
        std::cout << "0. exit" << std::endl;
        std::cout << "1. read FA" << std::endl;
        std::cout << "2. print FA" << std::endl;
        std::cout << "3. check sequence" << std::endl;
        std::cout << ">> ";
        
        int input;
        std::cin >> input;

        switch (input) {
            case 0:{
                running = false;
            }break;

            case 1:{
                std::cout << "Path to fa >> ";
                std::string path;
                std::cin >> path;
                fa = FA::read_from_file(path);
            }break;

            case 2:{
                std::cout << "States: ";
                for (let state : fa.states){
                    std::cout << state << ", ";
                }
                std::cout << std::endl;

                std::cout << "Alphabet: ";
                for (let letter : fa.alphabet){
                    std::cout << letter << ", ";
                }
                std::cout << std::endl;

                std::cout << "Final states: ";
                for (let state : fa.final_states){
                    std::cout << state << ", ";
                }
                std::cout << std::endl;

                std::cout << "Transitions: " << std::endl;
                for (let edge : fa.edges){
                    std::cout << edge.start 
                              << " -- " 
                              << edge.letter
                              << " --> " 
                              << edge.end << std::endl;
                }

                std::cout << "Initial state: " << fa.initial_state << std::endl;
            }break;

            case 3:{
                std::cout << "Sequence >> ";
                std::string sequence;
                std::cin >> sequence;
                let correct = fa.accept(sequence);
                std::cout << "Correct: " << correct << std::endl;
            }break;

            default:{
                std::cout << "Invalid option" << std::endl;
            }break;
        }
    }
}
