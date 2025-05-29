#include <iostream>

int main(){

    char num_P;

    std::cout << "Selecione o tipo de algoritmo que você deseja aplicar no seu processo : \n";
    std::cout << "1 - Alternância circular \n";
    std::cout << "2 - Prioridade \n";
    std::cout << "3 - Loteria \n";
    std::cout << "4 - CFS (Completely Fair Scheduler) \n";
    std::cin >> num_P; 

    switch (num_P) {
        case '1':
            std::cout << "Você escolheu o algoritmo de Alternância Circular.\n";
            break;
        case '2':
            std::cout << "Você escolheu o algoritmo de Prioridade.\n";
            break;
        case '3':
            std::cout << "Você escolheu o algoritmo de Loteria.\n";
            break;
        case '4':
            std::cout << "Você escolheu o algoritmo CFS (Completely Fair Scheduler).\n";
            break;
        default:
            std::cout << "Opção inválida. Por favor, selecione uma opção válida.\n";
    }

    return 0;
}