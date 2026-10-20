# Sismic

Soluções dos exercícios do **Laboratório de Sistemas Microprocessados — 2026.2**
(placa MSP-EXP430F5529LP, Code Composer Studio).

Cada arquivo `.asm`/`.c` é um programa completo e independente: um arquivo
por projeto no CCS.

## Como usar no Code Composer Studio

**Programas em C (Módulos 2 e 3)**

1. *File → New → CCS Project*, alvo **MSP430F5529**, modelo **Empty Project (with main.c)**.
2. Substitua o conteúdo do `main.c` pelo arquivo desejado.

**Programas em assembly (Módulo 1)** — usam `.global main`, como no roteiro:

1. *File → New → CCS Project*, alvo **MSP430F5529**, modelo **Empty Project**
   (não use o "Empty Assembly-only Project": ele começa em `RESET` e não chama `main`).
2. Apague o `main.c` e adicione o `.asm` desejado ao projeto.
3. Em *Project → Properties → Build → MSP430 Compiler → Processor Options*,
   coloque **Code memory model = small** e **Data memory model = small**.
   Com o modelo *large* o código vai para acima de 64 KB e os `call #rotina`
   (endereço de 16 bits) dão erro *"relocation ... overflowed"*.

O `build.sh` compila exatamente nessa configuração.

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

Os exercícios 9 e 10 do Módulo 2 usam a saída de um timer no LED vermelho:
tire o jumper JP8 e ligue o pino indicado no comentário do arquivo ao pino de
**baixo** do jumper. No Visto 2 / ex. 14 do Módulo 3 isso não é preciso: o
PWM do LED vermelho é repetido em P1.0 pelas interrupções do Timer B
(`LED_VERMELHO_SOFTWARE = 1`).

## Observações

- Clocks padrão do F5529: MCLK = SMCLK ≈ 1,048576 MHz, ACLK = 32768 Hz.
  O código base do servo (Visto 2) trata o SMCLK como 1 MHz; os outros
  programas do servo seguem a mesma convenção.
- No Problema Final do Módulo 3 (Parte 2), a frase muda assim que o joystick
  sai da região neutra; se ele continuar inclinado, depois de 1 s passa a
  mudar a cada 0,5 s.
- Os códigos do controle remoto (Mód. 2, ex. 18) são os do gist indicado no
  roteiro (controle "Car mp3", NEC, usuário 00FF).
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

Os três Vistos do Módulo 3 foram testados na placa e servem de base para o
resto do módulo. Os outros exercícios usam as mesmas rotinas: a inicialização
do LCD (com as esperas do Visto 1) e a UART a 9600 bps (configuração do
Visto 3). Os demais programas dos Módulos 2 e 3 foram compilados, mas ainda
precisam ser conferidos na placa.
