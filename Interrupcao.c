#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"

#define NUM_PIXELS 25
#define WS2812_PIN 7
#define IS_RGBW false

// Definições de hardware
static PIO pio = pio0;
static uint sm;
static uint offset;

const uint pinoLedVermelho = 13;
const uint pinoLedVerde = 11;
const uint pinoLedAzul = 12;
const uint botao_A = 5;
const uint botao_B = 6;

// Variáveis globais
static volatile int numero = 0;
static volatile uint ultimo_tempo = 0;
static volatile uint a = 1;
static volatile uint b = 1;

// Mapeamento dos LEDs
const int mapeamento_LEDS[NUM_PIXELS] = {
    24, 23, 22, 21, 20,  
    15, 16, 17, 18, 19,  
    14, 13, 12, 11, 10,  
    5,  6,  7,  8,  9,  
    4,  3,  2,  1,  0   
};

// Representação dos dígitos de 0 a 9
static int padroes_digitos[10][NUM_PIXELS] = {
    { 0,1,1,1,0,  1,0,0,0,1,  1,0,0,0,1,  1,0,0,0,1,  0,1,1,1,0 }, // 0
    { 1,1,1,0,0,  0,0,1,0,0,  0,0,1,0,0,  0,0,1,0,0,  0,1,1,1,0 }, // 1
    { 1,1,1,1,0,  0,0,0,0,1,  0,1,1,1,0,  1,0,0,0,0,  1,1,1,1,1 }, // 2
    { 1,1,1,1,0,  0,0,0,0,1,  0,1,1,1,0,  0,0,0,0,1,  1,1,1,1,0 }, // 3
    { 1,0,0,1,0,  1,0,0,1,0,  1,1,1,1,1,  0,0,0,1,0,  0,0,0,1,0 }, // 4
    { 1,1,1,1,1,  1,0,0,0,0,  1,1,1,1,0,  0,0,0,0,1,  1,1,1,1,0 }, // 5
    { 0,1,1,1,0,  1,0,0,0,0,  1,1,1,1,0,  1,0,0,0,1,  0,1,1,1,0 }, // 6
    { 1,1,1,1,1,  0,0,0,0,1,  0,0,0,1,0,  0,0,1,0,0,  0,1,0,0,0 }, // 7
    { 0,1,1,1,0,  1,0,0,0,1,  0,1,1,1,0,  1,0,0,0,1,  0,1,1,1,0 }, // 8
    { 0,1,1,1,0,  1,0,0,0,1,  0,1,1,1,1,  0,0,0,0,1,  0,1,1,1,0 }  // 9
};

// Prototipação de funções
static inline void colocar_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void definir_leds_digito(int *padrao, uint8_t r, uint8_t g, uint8_t b);
void controle_interrupcao_gpio(uint gpio, uint32_t eventos);
void inicializar_hardware();

int main() {
    stdio_init_all();
    inicializar_hardware();
    definir_leds_digito(padroes_digitos[numero], 255, 0, 255);

    while (true) {
        gpio_put(pinoLedVermelho, true);
        sleep_ms(100);
        gpio_put(pinoLedVermelho, false);
        sleep_ms(100);
    }
}

// Configura todo o hardware necessário para o programa
void inicializar_hardware() {

    // LEDs vermelho, verde e azul como saída digital 
    gpio_init(pinoLedVermelho);
    gpio_set_dir(pinoLedVermelho, GPIO_OUT);
    gpio_init(pinoLedVerde);
    gpio_set_dir(pinoLedVerde, GPIO_OUT);
    gpio_init(pinoLedAzul);
    gpio_set_dir(pinoLedAzul, GPIO_OUT);

    // Botões A e B como entrada digital com pull-up
    gpio_init(botao_A);
    gpio_set_dir(botao_A, GPIO_IN);
    gpio_pull_up(botao_A);

    gpio_init(botao_B);
    gpio_set_dir(botao_B, GPIO_IN);
    gpio_pull_up(botao_B);

    // Configuração do PIO para controlar os LEDs
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(botao_A, GPIO_IRQ_EDGE_FALL, true, &controle_interrupcao_gpio);
    gpio_set_irq_enabled_with_callback(botao_B, GPIO_IRQ_EDGE_FALL, true, &controle_interrupcao_gpio);

   
}

// Função chamada quando ocorre interrupção nos botões
void controle_interrupcao_gpio(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    
    if (tempo_atual - ultimo_tempo > 200000) {
        ultimo_tempo = tempo_atual;

        if (gpio == botao_A) {
            numero = (numero + 1) % 10;
            definir_leds_digito(padroes_digitos[numero], 255, 0, 255);
        } else if (gpio == botao_B) {
            numero = (numero - 1 + 10) % 10;
            definir_leds_digito(padroes_digitos[numero], 255, 0, 255);
        }
    }
}

// Função para colocar um pixel na saída do PIO
static inline void colocar_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para converter um valor RGB de 8 bits para um valor de 32 bits
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função para definir a cor de um dígito nos LEDs
void definir_leds_digito(int *padrao, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t cor = urgb_u32(r,g,b);

    for (int i = 0; i < NUM_PIXELS; i++) {
        colocar_pixel(padrao[mapeamento_LEDS[i]] ? cor : 0);
    }
}
