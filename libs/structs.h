/*
Esse arquivo contém as structs que servem de base para construir as informações necessárias
Para montar as informações de cada resistor
*/

// Informações do resistor puro, excluindo a potência
typedef struct {
    int resistencia_base; // Possui a base do resistor, que é multiplicada pela potencia
    const char *faixa1; // Cor do primeiro digito
    const char *faixa2; // Cor do segundo digito
} Resistor;


// Informações da potência
typedef struct {
    double multiplicador; // A potência que multiplica o valor base (10, 100, 1000...)
    const char *faixa3; // Cor da terceira faixa (potência de 10)
} Multiplicador;