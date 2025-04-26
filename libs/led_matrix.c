#include "led_matrix.h"

#define MATRIX_PIN 7

// Variável que define a intensidade do brilho dos LEDS
static const uint8_t intensidade_max = 10;

// Quantidade de pixels
#define NUM_PIXELS 25

// Buffer que armazena o frame atual
bool led_buffer[NUM_PIXELS] = {
    0, 0, 0, 0, 0, // Linha 0 (Certo)
    0, 0, 0, 0, 0, // Linha 1 (Espelhado)
    1, 1, 1, 1, 1, // Linha 2 (Certo)
    0, 0, 0, 0, 0, // Linha 3 (Espelhado)
    0, 0, 0, 0, 0, // Linha 4 (Certo)
};

static inline void put_pixel(uint32_t pixel_grb){
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função que vai transformar valores correspondentes ao padrão RGB em dados binários
uint32_t urgb_u32(double r, double g, double b){
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função que vai imprimir cada pixel na matriz
void set_leds(Cores matriz_rgb[25]){
    float intensidade = 0.05; // Multiplicador de intensidade para os leds
    uint32_t color; // Armazena os valores das cores


    // Define todos os LEDs com a cor especificada
    // Faz o processo de virar de cabeça para baixo o arranjo
    for (int i = NUM_PIXELS-1; i >= 0; i--){
        if (led_buffer[i]){
            color = urgb_u32(intensidade*matriz_rgb[i].red, intensidade*matriz_rgb[i].green, intensidade*matriz_rgb[i].blue); // Converte as cores para o padrão aceito pela matriz
            put_pixel(color); // Liga o LED com um no buffer
        }
        else{
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}