# ⚡ Picohmimetro - BitDogLab ⚡

Este projeto é a Atividade 2 da Fase 2 do EmbarcaTech. Consiste em um ohmimetro para identificação de valores desconhecidos de resistência, assim como também a identificação do padrão de cores desse resistor.

---

## 📌 **Funcionalidades Implementadas**

✅ Utilização de um divisor de tensão conectado ao ADC da GPIO28\
✅ Identificação de valores desconhecidos de resistências através do divisor de tensão\
✅ Exibição no Display OLED dos valores lidos no ADC e o valor da resistência descoberta\
✅ Exibição na matriz de LEDs e no Display OLED da faixa de cores dos resistores

---

## 🛠 **Hardware Utilizado**

- **Placa BitDogLab**
- **ADC** para leitura de tensão e posterior conversão desses valores para descoberta da resistência desconhecida
- **Botões** utilizado na prototipagem, quando acionava o Botão A o RP2040 entrava em modo Bootsel, agilizando o boot do código atualizado
- **Matriz de LEDs endereçáveis** exibe a faixa de cores do resistor descoberto
- **Display I2C** visualização das cores que compõem a faixa de cores do resistor desconhecido, assim como o valor lido no ADC e o valor descoberto de resistência
- **PIO** criação da máquina de estados que opera a matriz de LEDs endereçáveis
- **Comunicação UART** na prototipagem enviava dados via Serial Monitor, para depurar o que acontecia e corrigir possíveis bugs

---

## 📂 **Estrutura do Código**

```
📂 PixelTracker/
├── 📄 main.c                          # Código principal do projeto
├──── 📂libs
├───── 📄 font.h                       # Fonte utilizada no Display I2C
├───── 📄 led_matrix.c                 # Funções para manipulação da matriz de LEDs endereçáveis
├───── 📄 led_matrix.h                 # Cabeçalho para o led_matrix.c
├───── 📄 ssd1306.c                    # Funções que controlam o Display I2C
├───── 📄 ssd1306.h                    # Cabeçalho para o ssd1306.c
├───── 📄 structs.h                    # Structs utilizadas no código principal
├──── 📂generated
├───── 📄 ws2812.pio.h                 # Arquivo gerado para utilização da máquina de estados
├── 📄 ws2812.pio                      # Máquina de estados para controle da matriz de LEDs
├── 📄 CMakeLists.txt                  # Configurações para compilar o código corretamente
├── 📄 diagram.json                    # Layout para simulação no wokwi
├── 📄 wokwi.toml                      # Arquivo com a localização do código compilado
└── 📄 README.md                       # Documentação do projeto
```

---

## 📽️ **Vídeo no YouTube**
[YouTube](https://youtu.be/vYgJOyqHYHo)
