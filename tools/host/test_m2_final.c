/* Testa no PC a lógica das ISRs do Problema Final do Módulo 2.
   Compilar (ver tools/run_host_tests.sh). PARTE = 1 ou 2. */
#include <stdio.h>
#include <stdlib.h>
#define main app_main
#if PARTE == 1
#include "../../modulo2/validacao/problema_final_parte1_velocidade.c"
#else
#include "../../modulo2/validacao/problema_final_parte2_one_touch.c"
#endif
#undef main

static int falhas = 0;
#define CHECK(cond, msg) do { if(!(cond)) { printf("FALHOU: %s (angulo=%d)\n", msg, angulo); falhas++; } } while(0)

static void s1(int fechada){ if(fechada) P2IN &= ~BIT1; else P2IN |= BIT1; }
static void s2(int fechada){ if(fechada) P1IN &= ~BIT1; else P1IN |= BIT1; }
static void tiques(int n){ while(n--) TA0_CCR0_ISR(); }
static int pulso(void){ return TA1CCR1 + 1; }

int main(void){
#if PARTE == 1
    servo_angulo(angulo);
    CHECK(angulo == 0 && pulso() == 500, "começa em 0 grau / 0,5 ms");
    s1(1); tiques(20);                       /* 1 s a 10 graus/s */
    CHECK(angulo == 10, "10 graus/s com S1 fechada");
    CHECK(pulso() == 500 + 10 * 2000 / 180, "pulso correspondente ao ângulo");
    s1(0); tiques(20);
    CHECK(angulo == 10, "S1 aberta: não gira");
    s2(1); tiques(3);
    CHECK(velocidade == 10, "S2 fechando não troca a velocidade");
    s2(0); tiques(1);
    CHECK(velocidade == 20, "S2 abrindo troca para 20 graus/s");
    s1(1); tiques(20);
    CHECK(angulo == 30, "20 graus/s");
    tiques(150 * 1);                         /* 7,5 s: chega a 180 e volta */
    CHECK(direcao == -1, "inverte ao chegar em 180");
    CHECK(angulo == 180 - (30 + 150 - 180), "desce após 180");
    s1(0); s2(1); tiques(1); s2(0); tiques(1);
    CHECK(velocidade == 10, "S2 de novo: volta para 10 graus/s");
    s1(1); tiques(2 * 200);                  /* 200 passos: vai até 0 e volta */
    CHECK(direcao == +1, "inverte ao chegar em 0");
    CHECK(angulo == 200 - 180, "sobe após 0");
#else
    CHECK(angulo == 90, "começa em 90");
    s1(1); tiques(10); s1(0); tiques(1);
    CHECK(angulo == 80, "S1 por 1 s desce 10 graus");
    tiques(20);
    CHECK(angulo == 80, "< 3 s: para quando solta");
    s2(1); tiques(35); s2(0);
    CHECK(angulo == 115, "S2 por 3,5 s sobe 35 graus");
    tiques(1 + 65); 
    CHECK(angulo == 180, "> 3 s: continua até 180");
    tiques(10);
    CHECK(angulo == 180, "para em 180");
    s1(1); tiques(40); s1(0); tiques(1);
    CHECK(angulo == 140, "S1 4 s");
    tiques(10);
    CHECK(angulo == 130, "auto descendo");
    s2(1); tiques(1);
    CHECK(angulo == 130, "qualquer chave interrompe o automático");
    tiques(10);
    CHECK(angulo == 130, "a mesma pressão não inicia outro movimento");
    s2(0); tiques(1); s2(1); tiques(5); s2(0); tiques(5);
    CHECK(angulo == 135, "depois de soltar, S2 volta a funcionar");
    s1(1); tiques(200); s1(0); tiques(5);
    CHECK(angulo == 0, "não passa de 0");
    s2(1); tiques(30); s2(0); tiques(300);
    CHECK(angulo == 180, "exatamente 3 s também ativa o automático");
#endif
    printf("Parte %d: %s\n", PARTE, falhas ? "FALHOU" : "ok");
    return falhas != 0;
}
