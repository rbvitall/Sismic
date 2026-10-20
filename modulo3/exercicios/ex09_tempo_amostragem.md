# Módulo 3 — Exercício 9: tempo de amostragem do ADC12

Tempo de carga do capacitor de entrada:

t_sample > (R_S + R_I) · ln(2^(n+1)) · C_I + 800 ns

Dados: R_I = 1,8 kΩ, C_I = 25 pF, n = 12 bits.

Para o potenciômetro de 10 kΩ, o pior caso é o cursor no meio (50%). O MSP
"enxerga" as duas metades em paralelo:

R_S = (R/2 ∥ R/2) = R/4 = 10 kΩ / 4 = **2,5 kΩ**

Então:

- ln(2^13) = 13 · ln 2 ≈ 9,01
- (2,5 kΩ + 1,8 kΩ) · 25 pF = 107,5 ns
- 107,5 ns · 9,01 ≈ 969 ns
- **t_sample > 969 ns + 800 ns ≈ 1,77 µs**

Com SHP = 1 e ADC12CLK = SMCLK ≈ 1 MHz (1 µs por ciclo), precisamos de pelo
menos 1,77 ciclos. O menor valor de SHT já dá 4 ciclos:

- **SHT = 0 → 4 ciclos de ADC12CLK ≈ 4 µs > 1,77 µs** ✔

(Com SMCLK = 1,048576 MHz os 4 ciclos dão ≈ 3,8 µs, ainda com folga.)

## Referência para o exercício 10 (sensores de 50 kΩ)

Considerando, no pior caso, R_S = 50 kΩ:

- (50 kΩ + 1,8 kΩ) · 25 pF · 9,01 ≈ 11,7 µs → t_sample > 12,5 µs
- SHT = 1 dá 8 ciclos (8 µs, insuficiente); **SHT = 2 → 16 ciclos ≈ 16 µs** ✔
