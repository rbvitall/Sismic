# Sismic

Soluções dos exercícios do **Laboratório de Sistemas Microprocessados — 2026.2**
(placa MSP-EXP430F5529LP, Code Composer Studio).

Cada arquivo `.asm`/`.c` é um programa completo e independente: no CCS, crie
um projeto para o MSP430F5529 (vazio, em assembly ou em C) e copie o arquivo
para dentro dele.

## Estrutura

```
modulo1/   Assembly
  exercicios/   Exercícios 1 a 25 do Modulo1.pdf
  validacao/    Exemplos 1 e 2, Vistos 1 e 2 e Problema Final
modulo2/   C — GPIO, timers, interrupções
  exercicios/   Exercícios 1 a 18 do Modulo2.pdf
  validacao/    Exemplos 1 a 3, Vistos 1 e 2 e Problema Final (partes 1 e 2)
modulo3/   C — I2C/LCD, ADC, UART
  exercicios/   Exercícios 1 a 19 do Modulo3.pdf
  validacao/    Vistos 1 a 3 e Problema Final (partes 1 e 2)
tools/     Emulador e testes (não precisam da placa)
build.sh   Compila tudo com o compilador do CCS
```

### Lista de validação

| Módulo | Item | Arquivo |
|---|---|---|
| 1 | Exemplo 1 – soma saturada | `modulo1/validacao/exemplo1_soma_saturada.asm` |
| 1 | Exemplo 2 – multiplicação por laço | `modulo1/validacao/exemplo2_multiplicacao.asm` |
| 1 | Visto 1 – `reduceSum8` | `modulo1/validacao/visto1_reduceSum8.asm` |
| 1 | Visto 2 – `EXTREMOS` | `modulo1/validacao/visto2_EXTREMOS.asm` |
| 1 | Problema Final – `MAT_TRANSP` | `modulo1/validacao/problema_final_MAT_TRANSP.asm` |
| 2 | Exemplos 1, 2 e 3 | `modulo2/validacao/exemplo*.c` |
| 2 | Visto 1 – duas chaves / dois LEDs | `modulo2/validacao/visto1_duas_chaves.c` |
| 2 | Visto 2 – servo (Ex. 11) | `modulo2/validacao/visto2_servo.c` |
| 2 | Problema Final – parte 1 (velocidade) | `modulo2/validacao/problema_final_parte1_velocidade.c` |
| 2 | Problema Final – parte 2 (one-touch) | `modulo2/validacao/problema_final_parte2_one_touch.c` |
| 3 | Visto 1 – mensagem no LCD | `modulo3/validacao/visto1_lcd.c` |
| 3 | Visto 2 – joystick controla os LEDs (Ex. 14) | `modulo3/validacao/visto2_joystick_leds.c` |
| 3 | Visto 3 – frase no terminal ao apertar S1 | `modulo3/validacao/visto3_uart_pc.c` |
| 3 | Problema Final – parte 1 (chaves) | `modulo3/validacao/problema_final_parte1_chaves.c` |
| 3 | Problema Final – parte 2 (joystick) | `modulo3/validacao/problema_final_parte2_joystick.c` |

## Pinos da LaunchPad usados

| Recurso | Pino |
|---|---|
| LED1 vermelho | P1.0 |
| LED2 verde | P4.7 |
| Chave S1 | P2.1 (também TA1.2) |
| Chave S2 | P1.1 |
| Servo (PWM, TA1.1) | P2.0 — marrom GND, vermelho 5 V, laranja P2.0 |
| I2C do LCD (USCI_B0) | P3.0 = SDA, P3.1 = SCL — endereço 0x27 ou 0x3F |
| Joystick | VRx → P6.0 (A0), VRy → P6.1 (A1), **"+5V" do joystick no 3V3** |
| UART entre placas (USCI_A0) | P3.3 = TX, P3.4 = RX |
| UART com o PC (USCI_A1) | interna (eZ-FET), 9600 bps 8N1 |

Os exercícios que usam a saída de um timer no LED vermelho (Mód. 2 ex. 9/10 e
Mód. 3 ex. 14/Visto 2) pedem para tirar o jumper JP8 e ligar o pino indicado
no comentário do arquivo ao pino de **baixo** do jumper.

## Observações

- Clocks padrão do F5529: MCLK = SMCLK ≈ 1,048576 MHz, ACLK = 32768 Hz.
  O código base do servo (Visto 2) trata o SMCLK como 1 MHz; os outros
  programas do servo seguem a mesma convenção.
- **Assembly no CCS:** se o link der *"relocation ... overflowed"* em um
  `call #rotina`, mude o *code model* do projeto para `small`
  (Properties → Build → MSP430 Compiler → Processor Options).
- No Módulo 3 Parte 2 do Problema Final, a frase muda uma vez por movimento
  do joystick (é preciso voltar à região neutra para mudar de novo), igual
  ao comportamento das chaves na Parte 1.
- O exercício 9 do Módulo 3 é só de cálculo: `modulo3/exercicios/ex09_tempo_amostragem.md`.

## Compilar e testar sem a placa

Com o CCS instalado em `C:\ti\ccs2100` (ou defina `CCS_ROOT`), no Git Bash:

```sh
./build.sh                    # compila os 74 programas com o cl430
tools/run_host_tests.sh       # roda os testes
```

- `tools/test_modulo1.py` monta cada `.asm` do Módulo 1 com o cl430 e executa
  o binário em um emulador do MSP430 (`tools/emu430.py`), comparando os
  resultados com implementações de referência (incluindo entradas aleatórias).
- `tools/host/test_m2_final.c` simula no PC as rotinas de interrupção do
  Problema Final do Módulo 2 para conferir a máquina de estados do servo.

Os programas dos Módulos 2 e 3 dependem de periféricos (timers, I2C, ADC,
UART), então foram compilados mas precisam ser conferidos na placa.
