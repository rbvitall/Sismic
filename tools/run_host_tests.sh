#!/usr/bin/env bash
# Roda os testes que não precisam da placa:
#  1) Módulo 1: cada .asm montado pelo cl430 e executado no emulador (Python)
#  2) Módulo 2: lógica das ISRs do Problema Final simulada no PC (gcc)
set -e
cd "$(dirname "$0")"

python test_modulo1.py

cd host
for p in 1 2; do
  gcc -std=c99 -w -I. -DPARTE=$p test_m2_final.c regs.c -o "teste_m2_parte$p.exe"
  "./teste_m2_parte$p.exe"
done
