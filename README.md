# Display de numeral com Raspberry Pi Pico

## Descrição
Este projeto implementa um display numérico usando LEDs WS2812B controlados por um Raspberry Pi Pico. O display mostra números de 0 a 9 que podem ser incrementados ou decrementados através de dois botões.

## Conexões
- LED WS2812B: GPIO 7
- Botão A: GPIO 5 (com pull-up)
- Botão B: GPIO 6 (com pull-up)
- LED Vermelho: GPIO 13

## Funcionalidades
--> Exibição de números de 0 a 9 <br>
--> Botão A: Incrementa o número <br>
--> Botão B: Decrementa o número <br>
--> Controle de debounce por software (200ms) <br>
--> LED de status piscando <br>
--> Matriz de LEDs controlada via PIO <br>

Link do vídeo - código + funcionamento do projeto: https://drive.google.com/file/d/1C3rE8N6ZAcZIcBmapKhLazRTFZNtH4Y-/view?usp=sharing
