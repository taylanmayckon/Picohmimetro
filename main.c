#include <stdio.h>
#include "pico/stdlib.h"
#include "string.h"
#include "math.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "libs/ssd1306.h"
#include "libs/font.h"
#include "libs/structs.h"
#include "libs/led_matrix.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define BUTTON_A 5  // GPIO para botão A

// Variáveis da PIO declaradas no escopo global
PIO pio;
uint sm;
// Constantes para a matriz de leds
#define IS_RGBW false
#define LED_MATRIX_PIN 7

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
float tensao;
char str_x[5]; // Buffer para armazenar a string
char str_y[5]; // Buffer para armazenar a string
bool cor = true; // Boolenano para indicar se o display vai imprimir branco ou não
uint32_t last_time = 0; // Variável para o debounce (armazena tempo)

// Estrutura que armazena as bases de resistência e cores de cada uma
Resistor resistores_e24[] = {
    {10, "Marrom", "Preto"},
    {11, "Marrom", "Marrom"},
    {12, "Marrom", "Vermelho"},
    {13, "Marrom", "Laranja"},
    {15, "Marrom", "Verde"},
    {16, "Marrom", "Azul"},
    {18, "Marrom", "Cinza"},
    {20, "Vermelho", "Preto"},
    {22, "Vermelho", "Vermelho"},
    {24, "Vermelho", "Amarelo"},
    {27, "Vermelho", "Violeta"},
    {30, "Laranja", "Preto"},
    {33, "Laranja", "Laranja"},
    {36, "Laranja", "Azul"},
    {39, "Laranja", "Branco"},
    {43, "Amarelo", "Laranja"},
    {47, "Amarelo", "Violeta"},
    {51, "Verde", "Marrom"},
    {56, "Verde", "Azul"},
    {62, "Azul", "Vermelho"},
    {68, "Azul", "Cinza"},
    {75, "Violeta", "Verde"},
    {82, "Cinza", "Vermelho"},
    {91, "Branco", "Marrom"},
};

// Estrutura que armazena as potências e cores de cada uma
Multiplicador multiplicadores[] = {
    {1, "Preto"},
    {10, "Marrom"},
    {100, "Vermelho"},
    {1000, "Laranja"},
    {10000, "Amarelo"},
    {100000, "Verde"},
    {1000000, "Azul"},
    {10000000, "Violeta"},
    {100000000, "Cinza"},
    {1000000000, "Branco"},
    {0.1, "Dourado"},
    {0.01, "Prata"},
};

// RGB de cada cor
Cores cod_cores[] = {
    {"Preto", 0, 0, 0},
    {"Marrom", 112, 56, 0},
    {"Vermelho", 255, 0, 0},
    {"Laranja", 255, 165, 0},
    {"Amarelo", 255, 255, 0},
    {"Verde", 0, 128, 0},
    {"Azul", 0, 0, 255},
    {"Violeta", 128, 0, 128},
    {"Cinza", 128, 128, 128},
    {"Branco", 255, 255, 255},
    {"Dourado", 212, 175, 55},
    {"Prateado", 192, 192, 192}
};

// Matriz para armazenar a cor de cada LED a ser impresso
Cores matriz_cores[25];


// Interrupção de GPIO
void gpio_irq_handler(uint gpio, uint32_t events){
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtendo tempo atual (em us)
    if(current_time - last_time > 200000){ // Debounce
        last_time = current_time; 

        if(gpio == BUTTON_A){
            printf("Eu sou uma interrupção, e estou funcionando! :)\n");
        }
    }
    
}

int search_color(char *cor_procurada){
    int j=0; // Variavel para iterar o cod de cores
    while(strcmp(cor_procurada, cod_cores[j].nome) != 0){ // Sempre que for diferente de 0, as cores são diferentes, vai testando até achar a cor correta
        j++;
    }
    return j; // Retorna o index da cor no "cod_cores"
}

int main(){
    stdio_init_all();

    // Inicializando o botao A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  

    // Configurando a I2C
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializando o ADC
    adc_init();
    adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

    // Inicializando a PIO
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);

    while (true) {
        adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

        // Filtro de média móvel para as leituras do ADC
        float soma = 0.0f;
        for (int i = 0; i < 500; i++){
            soma += adc_read();
            sleep_ms(1);
        }
        float media = soma / 500.0f;

        // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
        R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // PARTE DA MATRIZ DE LEDS
        for(int i=0; i<25; i++){ // Faz a limpeza da matriz completa, apagando todas as cores
            matriz_cores[i] = cod_cores[0];
        }

        int j; // Variavel para armazenar o index dos vetores de cor correspondentes

        float base = R_x; // Vai ser operada para encontrar o valor da resistencia base no E24
        float potencia = 1; // Armazena o valor bruto da potência de 10

        // Essa parte é generalista, abranje todos os resistores E24 para normalizar
        while(base >= 91.0f){ // Valor maximo normalizado
            base /= 10; 
            potencia *= 10;
        }
        while(base <= 10.0f){ // Valor minimo normalizado
            base *= 10;
            potencia /= 10;
        }

        // Procurando o resistor mais próximo
        j = 0; // Valor inicial para o index
        int num_resistores = 24;

        for(int i=0; i<num_resistores; i++){
            double erro_percent = (double)fabs((base-resistores_e24[i].resistencia_base)/resistores_e24[i].resistencia_base); // Obtendo o percentual de erro
            if(erro_percent<=0.05f){ // Condição para encerrar o loop quando achar o resistor dentro do erro de 5%
                j=i; // Armazena o index do resistor
                break;
            }
        }

        // Alocando as novas cores na matriz
        int index = search_color(resistores_e24[j].faixa1); 
        matriz_cores[10] = cod_cores[index]; // Faixa 1 (Valor 1)

        index = search_color(resistores_e24[j].faixa2); 
        matriz_cores[11] = cod_cores[index]; // Faixa 2 (Valor 2)

        for(int i=0; i<12; i++){ // Pegando o index da cor de faixa de multiplicador
            if(potencia == multiplicadores[i].multiplicador){
                j=i;
                break;
            }
        }

        index = search_color(multiplicadores[j].faixa3); 
        matriz_cores[12] = cod_cores[index]; // Faixa 3 (Multiplicador)

        matriz_cores[13] = cod_cores[10]; // Faixa 4 (Tolerância)

        set_leds(matriz_cores);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // PARTE DO DISPLAY I2C
        sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
        sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

        //  Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6, false); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16, false);  // Desenha uma string
        ssd1306_draw_string(&ssd, "  Ohmimetro", 10, 28, false);  // Desenha uma string
        ssd1306_draw_string(&ssd, "ADC", 13, 41, false);          // Desenha uma string
        ssd1306_draw_string(&ssd, "Resisten.", 50, 41, false);    // Desenha uma string
        ssd1306_line(&ssd, 44, 37, 44, 60, cor);           // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_x, 8, 52, false);           // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 59, 52, false);          // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display
        printf("%.2f\n", R_x);
        sleep_ms(700);
    }
}
