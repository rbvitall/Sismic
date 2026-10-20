"""Testa as soluções em assembly do Módulo 1 no emulador (tools/emu430.py).

Cada .asm é montado pelo cl430 do CCS com um linker simplificado
(tools/lnk_teste.cmd), convertido para TI-TXT e executado. Além de rodar o
`main` de cada arquivo, as sub-rotinas são chamadas diretamente com outras
entradas (comparando com uma implementação de referência em Python).

Uso:  python tools/test_modulo1.py
"""
import os
import random
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
from emu430 import MSP430, C  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CCS = os.environ.get("CCS_ROOT", r"C:\ti\ccs2100\ccs")
CGT_DIR = os.path.join(CCS, "tools", "compiler")
CGT = os.path.join(CGT_DIR, sorted(d for d in os.listdir(CGT_DIR) if d.startswith("ti-cgt-msp430"))[-1])
INC = os.path.join(CCS, "ccs_base", "msp430", "include")
BIN = os.path.join(CGT, "bin")
TMP = tempfile.mkdtemp(prefix="sismic_")
HALT = 0xFF00  # endereço com "jmp $" usado como retorno das chamadas diretas

falhas = 0
total = 0


def build(rel):
    src = os.path.join(ROOT, rel)
    name = os.path.splitext(os.path.basename(rel))[0]
    out_dir = os.path.join(TMP, name)
    os.makedirs(out_dir, exist_ok=True)
    out = os.path.join(out_dir, name + ".out")
    mapf = os.path.join(out_dir, name + ".map")
    txt = os.path.join(out_dir, name + ".txt")
    subprocess.run(
        [os.path.join(BIN, "cl430"), "-vmspx", "--code_model=small", "--data_model=small",
         "--define=__MSP430F5529__", f"--include_path={INC}", f"--obj_directory={out_dir}",
         src, "-z", f"-m{mapf}", "--mapfile_contents=sym_defs", f"-i{INC}", "--ram_model",
         "-e", "main", "--retain=*(.data)", "-o", out, os.path.join(ROOT, "tools", "lnk_teste.cmd")],
        check=True, capture_output=True)
    subprocess.run([os.path.join(BIN, "hex430"), "--ti_txt", out, "-o", txt],
                   check=True, capture_output=True)
    syms = {}
    for m in re.finditer(r"^([0-9a-f]{8})\s+(?:local|global)\s+(\S+)", open(mapf).read(), re.M):
        syms[m.group(2)] = int(m.group(1), 16)
    return txt, syms


class Prog:
    def __init__(self, rel):
        self.rel = rel
        self.txt, self.sym = build(rel)

    def fresh(self):
        cpu = MSP430()
        cpu.load_ti_txt(self.txt)
        cpu.ww(HALT, 0x3FFF)  # jmp $
        return cpu

    def run_main(self):
        return self.fresh().run(self.sym["main"])

    def call(self, rotina, regs=None, mem=None, cpu=None):
        cpu = cpu or self.fresh()
        for addr, data in (mem or {}).items():
            for i, b in enumerate(data):
                cpu.wb(addr + i, b)
        for r, v in (regs or {}).items():
            cpu.r[r] = v & 0xFFFF
        cpu.r[1] = 0x4400 - 2
        cpu.ww(cpu.r[1], HALT)
        for r in range(4, 12):  # valores conhecidos p/ verificar PUSH/POP
            cpu.r[r] = 0xA000 + r
        cpu.run(self.sym[rotina])
        preservados = all(cpu.r[r] == 0xA000 + r for r in range(4, 12))
        check(f"{self.rel} {rotina}: R4-R11 preservados", preservados, True)
        return cpu


def check(nome, obtido, esperado):
    global falhas, total
    total += 1
    if obtido != esperado:
        falhas += 1
        print(f"FALHOU  {nome}: obtido {obtido!r}, esperado {esperado!r}")


def words(cpu, addr, n):
    return [cpu.rw(addr + 2 * i) for i in range(n)]


def bytes_(cpu, addr, n):
    return [cpu.rb(addr + i) for i in range(n)]


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def u16(v):
    return v & 0xFFFF


sys.stdout.reconfigure(encoding="utf-8")
rnd = random.Random(2026)
E = "modulo1/exercicios/"
V = "modulo1/validacao/"

# ---------------------------------------------------------------- 1 a 5
cpu = Prog(E + "ex01_enderecamento.asm").run_main()
check("ex01 memória", bytes_(cpu, 0x2400, 6), [0xFE, 0xCA, 0x34, 0x12, 0x78, 0x56])

p = Prog(E + "ex02_vetor_autoincremento.asm")
cpu = p.run_main()
check("ex02 R5..R8", cpu.r[5:9], [0x12, 0x34, 0x7856, 0xBC9A])
check("ex02 var8", cpu.rb(p.sym["var8"]), 0x33)

check("ex03 saturada", Prog(E + "ex03_soma_saturada.asm").run_main().r[4], 0xFFFF)
check("ex04 produto", Prog(E + "ex04_multiplicacao_laco.asm").run_main().r[6], 30000)
check("ex05 resultado", Prog(E + "ex05_multiplas_condicoes.asm").run_main().r[4], u16(-8))
check("exemplo1", Prog(V + "exemplo1_soma_saturada.asm").run_main().r[4], 579)
check("exemplo2", Prog(V + "exemplo2_multiplicacao.asm").run_main().r[6], 275)

# ---------------------------------------------------------------- 6 mult8
p = Prog(E + "ex06_mult8.asm")
check("ex06 main", p.run_main().r[4:7], [0, 65025, 408])
for a, b in [(0, 0), (255, 0), (1, 255), (255, 255)] + [(rnd.randrange(256), rnd.randrange(256)) for _ in range(20)]:
    check(f"ex06 mult8({a},{b})", p.call("mult8", {12: a, 13: b}).r[12], a * b)

# ---------------------------------------------------------------- 7, 8, 9
fib = [0, 1]
while len(fib) < 60:
    fib.append(fib[-1] + fib[-2])
cpu = Prog(E + "ex07_fib.asm").run_main()
check("ex07 FIB", words(cpu, 0x2400, 20), fib[:20])
check("ex08 FIB16", Prog(E + "ex08_fib16.asm").run_main().r[12], max(f for f in fib if f < 2**16))
cpu = Prog(E + "ex09_fib32.asm").run_main()
check("ex09 FIB32", (cpu.r[13] << 16) | cpu.r[12], max(f for f in fib if f < 2**32))

# ---------------------------------------------------------------- 10, 11, 12, 13
for rel in (E + "ex10_reduceSum8.asm", V + "visto1_reduceSum8.asm"):
    p = Prog(rel)
    check(f"{rel} main", p.run_main().r[12], 55)
    for n in (0, 1, 7, 20):
        lim = 255 if "exercicios" in rel else 12  # visto: soma cabe em 8 bits
        v = [rnd.randrange(lim + 1) for _ in range(n)]
        cpu = p.call("reduceSum8", {12: 0x2500, 13: n}, {0x2500: v})
        check(f"{rel} reduceSum8 n={n}", cpu.r[12], sum(v))

p = Prog(E + "ex11_reduceSum16.asm")
cpu = p.run_main()
check("ex11 main", (cpu.r[13] << 16) | cpu.r[12], 200010)
for n in (0, 1, 5, 40):
    v = [rnd.randrange(65536) for _ in range(n)]
    raw = b"".join(x.to_bytes(2, "little") for x in v)
    cpu = p.call("reduceSum16", {12: 0x2500, 13: n}, {0x2500: raw})
    check(f"ex11 reduceSum16 n={n}", (cpu.r[13] << 16) | cpu.r[12], sum(v))

p = Prog(E + "ex12_mapSub8.asm")
cpu = p.run_main()
check("ex12 main", bytes_(cpu, p.sym["vetor_s"], 5), [9, 18, 27, 36, 0xFF])

p = Prog(E + "ex13_mapSum16.asm")
cpu = p.run_main()
v1 = [7, 65000, 50054, 26472, 53000, 60606, 814, 41121]
v2 = [7, 226, 3400, 26472, 470, 1020, 44444, 12345]
check("ex13 endereços", (p.sym["vetor1"], p.sym["vetor2"], p.sym["soma"]), (0x2400, 0x2410, 0x2420))
check("ex13 soma", words(cpu, 0x2420, 8), [u16(a + b) for a, b in zip(v1, v2)])

# ---------------------------------------------------------------- 14 m2m4
p = Prog(E + "ex14_m2m4.asm")
check("ex14 main", p.run_main().r[12:14], [6, 3])
v = [rnd.randrange(256) for _ in range(30)]
cpu = p.call("m2m4", {12: 0x2500, 13: 30}, {0x2500: v})
check("ex14 aleatório", cpu.r[12:14], [sum(x % 2 == 0 for x in v), sum(x % 4 == 0 for x in v)])

# ---------------------------------------------------------------- 15 rot16


def rot_ref(x, opc):
    n, tipo, direita = opc & 0xF, (opc >> 8) & 0xF, (opc >> 12) & 0xF
    for _ in range(n):
        if direita:
            entra = {0: 0, 1: 1, 2: x >> 15, 4: x & 1}[tipo]
            x = (x >> 1) | (entra << 15)
        else:
            entra = {0: 0, 1: 1, 2: 0, 4: x >> 15}[tipo]
            x = ((x << 1) & 0xFFFF) | entra
    return x


p = Prog(E + "ex15_rot16.asm")
check("ex15 main", p.run_main().r[4:8], [0x3412, 0x0000, 0xF000, 0xF00F])
for _ in range(60):
    x = rnd.randrange(65536)
    opc = (rnd.choice([0, 1]) << 12) | (rnd.choice([0, 1, 2, 4]) << 8) | rnd.randrange(16)
    check(f"ex15 rot16({x:04X},{opc:04X})", p.call("rot16", {12: x, 13: opc}).r[12], rot_ref(x, opc))

# ---------------------------------------------------------------- 16, 17, 18
p = Prog(E + "ex16_menor.asm")
check("ex16 main", p.run_main().r[12:14], [3, 3])
for _ in range(10):
    v = [rnd.randrange(8) * 30 for _ in range(rnd.randrange(1, 15))]
    cpu = p.call("menor", {12: 0x2500, 13: len(v)}, {0x2500: v})
    check("ex16 aleatório", cpu.r[12:14], [min(v), v.count(min(v))])

p = Prog(E + "ex17_maior16.asm")
check("ex17 main", p.run_main().r[12:14], [0xFA11, 1])
for _ in range(10):
    v = [rnd.choice([0, 1, 0x7FFF, 0x8000, 0xFFFF, 1234]) for _ in range(rnd.randrange(1, 12))]
    raw = b"".join(x.to_bytes(2, "little") for x in v)
    cpu = p.call("maior16", {12: 0x2500, 13: len(v)}, {0x2500: raw})
    check("ex17 aleatório", cpu.r[12:14], [max(v), v.count(max(v))])

for rel, rot in ((E + "ex18_extremos.asm", "extremos"), (V + "visto2_EXTREMOS.asm", "EXTREMOS")):
    p = Prog(rel)
    check(f"{rel} main", [s16(x) for x in p.run_main().r[12:14]], [-32768, 30000])
    for _ in range(15):
        v = [rnd.randrange(-32768, 32768) for _ in range(rnd.randrange(1, 12))]
        raw = b"".join(u16(x).to_bytes(2, "little") for x in v)
        cpu = p.call(rot, {12: 0x2500, 13: len(v)}, {0x2500: raw})
        check(f"{rel} aleatório", [s16(x) for x in cpu.r[12:14]], [min(v), max(v)])

# ---------------------------------------------------------------- 19, 20
p = Prog(E + "ex19_W16_ASC.asm")
cpu = p.run_main()
check("ex19 main", bytes_(cpu, p.sym["saida"], 4), [0x38, 0x39, 0x41, 0x42])
for x in (0, 0xFFFF, 0x1234, 0xA0F9):
    cpu = p.call("W16_ASC", {12: x, 13: 0x2500})
    check(f"ex19 {x:04X}", bytes(bytes_(cpu, 0x2500, 4)).decode(), f"{x:04X}")

p = Prog(E + "ex20_ASC_W16.asm")
cpu = p.run_main()
check("ex20 main", (cpu.r[12], cpu.f(C)), (0x89AB, 1))
for s, ok in (("89AB", 1), ("0000", 1), ("FFFF", 1), ("1F2E", 1), ("8;AB", 0),
              ("89aB", 0), ("G000", 0), ("/000", 0), ("000@", 0), ("9:00", 0)):
    cpu = p.call("ASC_W16", {12: 0x2500}, {0x2500: s.encode()})
    check(f"ex20 '{s}' carry", cpu.f(C), ok)
    if ok:
        check(f"ex20 '{s}' valor", cpu.r[12], int(s, 16))

# ---------------------------------------------------------------- 21 ORDENA
p = Prog(E + "ex21_ordena.asm")
cpu = p.run_main()
check("ex21 main", bytes_(cpu, p.sym["vetor"], 10), sorted([4, 7, 3, 5, 1, 255, 0, 128, 99, 200]))
for n in (0, 1, 2, 5, 30):
    v = [rnd.randrange(256) for _ in range(n)]
    cpu = p.call("ORDENA", {12: 0x2500, 13: n}, {0x2500: v})
    check(f"ex21 n={n}", bytes_(cpu, 0x2500, n), sorted(v))

# ---------------------------------------------------------------- 22, 23 romanos


def to_rom(n):
    out = ""
    for val, sym in ((1000, "M"), (900, "CM"), (500, "D"), (400, "CD"), (100, "C"), (90, "XC"),
                     (50, "L"), (40, "XL"), (10, "X"), (9, "IX"), (5, "V"), (4, "IV"), (1, "I")):
        while n >= val:
            out += sym
            n -= val
    return out


p22 = Prog(E + "ex22_ROM_ARAB.asm")
check("ex22 main", p22.run_main().r[12], 1479)
p23 = Prog(E + "ex23_ARAB_ROM.asm")
cpu = p23.run_main()
check("ex23 main", bytes(bytes_(cpu, p23.sym["saida"], 9)), b"MCDLXXIX\0")
for n in list(range(1, 60)) + [3999, 3888, 1994, 2026, 444, 999] + [rnd.randrange(1, 4000) for _ in range(40)]:
    cpu = p23.call("ARAB_ROM", {12: n, 13: 0x2500})
    rom = to_rom(n)
    check(f"ex23 {n}", bytes(bytes_(cpu, 0x2500, len(rom) + 1)), rom.encode() + b"\0")
    cpu = p22.call("ROM_ARAB", {12: 0x2500}, {0x2500: rom.encode() + b"\0"})
    check(f"ex22 {rom}", cpu.r[12], n)

# ---------------------------------------------------------------- 24, 25 matrizes
for rel in (E + "ex24_MAT_TRANSP.asm", V + "problema_final_MAT_TRANSP.asm"):
    p = Prog(rel)
    cpu = p.run_main()
    check(f"{rel} main original", words(cpu, 0x2400, 6), [1, 2, 3, 4, 5, 6])
    check(f"{rel} main transposta", words(cpu, 0x240C, 6), [1, 4, 2, 5, 3, 6])
    for lin, col in ((1, 1), (1, 4), (4, 1), (3, 3), (3, 5), (5, 2)):
        a = [rnd.randrange(65536) for _ in range(lin * col)]
        raw = b"".join(x.to_bytes(2, "little") for x in a)
        cpu = p.call("MAT_TRANSP", {12: 0x2500, 13: lin, 14: col}, {0x2500: raw})
        t = [a[i * col + j] for j in range(col) for i in range(lin)]
        check(f"{rel} {lin}x{col}", words(cpu, 0x2500, 2 * lin * col), a + t)

p = Prog(E + "ex25_SUM_SUB.asm")
check("ex25 main", p.run_main().r[12:16], [28, 24, 16, 12])
for lin, col in ((2, 2), (2, 5), (4, 3), (5, 5)):
    a = [rnd.randrange(1000) for _ in range(lin * col)]
    raw = b"".join(x.to_bytes(2, "little") for x in a)
    cpu = p.call("SUM_SUB", {12: 0x2500, 13: lin, 14: col}, {0x2500: raw})
    el = lambda li, ci: a[li * col + ci]  # noqa: E731
    esp = [
        sum(el(i, j) for i in range(1, lin) for j in range(1, col)),
        sum(el(i, j) for i in range(1, lin) for j in range(0, col - 1)),
        sum(el(i, j) for i in range(0, lin - 1) for j in range(1, col)),
        sum(el(i, j) for i in range(0, lin - 1) for j in range(0, col - 1)),
    ]
    check(f"ex25 {lin}x{col}", cpu.r[12:16], [u16(x) for x in esp])

print(f"{total - falhas}/{total} verificações passaram")
sys.exit(1 if falhas else 0)
