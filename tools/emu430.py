"""Emulador mínimo da CPU MSP430 (conjunto de instruções clássico, 16 bits).

Serve para testar as sub-rotinas em assembly do Módulo 1 sem a placa:
o programa é compilado pelo cl430 (ver tools/test_modulo1.py), convertido
para TI-TXT pelo hex430 e executado a partir de `main` até cair no laço
infinito `jmp $`. Periféricos não são emulados (escritas neles são ignoradas).
"""

C, Z, N, V = 0x0001, 0x0002, 0x0004, 0x0100


class Halt(Exception):
    pass


class MSP430:
    def __init__(self):
        self.mem = bytearray(0x10000)
        self.r = [0] * 16
        self.r[1] = 0x4400  # SP no topo da RAM
        self.steps = 0

    # ---------------------------------------------------------------- memória
    def load_ti_txt(self, path):
        addr = 0
        for line in open(path):
            line = line.strip()
            if not line or line == "q":
                continue
            if line.startswith("@"):
                addr = int(line[1:], 16)
                continue
            for b in line.split():
                self.mem[addr] = int(b, 16)
                addr += 1

    def rb(self, a):
        return self.mem[a & 0xFFFF]

    def rw(self, a):
        a &= 0xFFFE
        return self.mem[a] | (self.mem[a + 1] << 8)

    def wb(self, a, v):
        self.mem[a & 0xFFFF] = v & 0xFF

    def ww(self, a, v):
        a &= 0xFFFE
        self.mem[a] = v & 0xFF
        self.mem[a + 1] = (v >> 8) & 0xFF

    def fetch(self):
        w = self.rw(self.r[0])
        self.r[0] = (self.r[0] + 2) & 0xFFFF
        return w

    # ---------------------------------------------------------------- flags
    def setf(self, mask, cond):
        if cond:
            self.r[2] |= mask
        else:
            self.r[2] &= ~mask

    def f(self, mask):
        return 1 if self.r[2] & mask else 0

    # ---------------------------------------------------------------- operandos
    def src_operand(self, reg, As, byte):
        """Retorna (valor, endereço|None)."""
        size = 1 if byte else 2
        if reg == 3:  # gerador de constantes
            return ([0, 1, 2, 0xFFFF][As] & (0xFF if byte else 0xFFFF)), None
        if reg == 2 and As >= 2:
            return (4 if As == 2 else 8), None
        if As == 0:
            v = self.r[reg]
            return (v & 0xFF if byte else v), None
        if As == 1:
            pc_ext = self.r[0]
            x = self.fetch()
            if reg == 2:
                addr = x
            elif reg == 0:
                addr = (pc_ext + x) & 0xFFFF
            else:
                addr = (self.r[reg] + x) & 0xFFFF
        else:
            if reg == 0 and As == 3:  # imediato
                v = self.fetch()
                return (v & 0xFF if byte else v), None
            addr = self.r[reg]
            if As == 3:
                inc = 2 if reg in (0, 1) else size
                self.r[reg] = (self.r[reg] + inc) & 0xFFFF
        return (self.rb(addr) if byte else self.rw(addr)), addr

    def dst_operand(self, reg, Ad, byte):
        """Retorna (valor, ('r', reg) | ('m', addr))."""
        if Ad == 0:
            v = self.r[reg]
            return (v & 0xFF if byte else v), ("r", reg)
        pc_ext = self.r[0]
        x = self.fetch()
        if reg == 2:
            addr = x
        elif reg == 0:
            addr = (pc_ext + x) & 0xFFFF
        else:
            addr = (self.r[reg] + x) & 0xFFFF
        return (self.rb(addr) if byte else self.rw(addr)), ("m", addr)

    def write(self, loc, v, byte):
        kind, where = loc
        if kind == "r":
            v &= 0xFF if byte else 0xFFFF
            if where == 3:
                return
            self.r[where] = v
        else:
            if where < 0x1000:  # periféricos: ignorar
                return
            (self.wb if byte else self.ww)(where, v)

    # ---------------------------------------------------------------- execução
    def step(self):
        pc0 = self.r[0]
        op = self.fetch()
        self.steps += 1
        if op & 0xE000 == 0x2000:  # saltos
            cond = (op >> 10) & 7
            off = op & 0x3FF
            if off & 0x200:
                off -= 0x400
            take = [
                not self.f(Z), self.f(Z), not self.f(C), self.f(C),
                self.f(N), self.f(N) == self.f(V), self.f(N) != self.f(V), True,
            ][cond]
            if take:
                target = (self.r[0] + 2 * off) & 0xFFFF
                if target == pc0:
                    raise Halt()
                self.r[0] = target
            return
        if op & 0xF000 == 0x1000:  # formato II
            opc = (op >> 7) & 7
            byte = (op >> 6) & 1
            As = (op >> 4) & 3
            reg = op & 0xF
            mask = 0xFF if byte else 0xFFFF
            msb = 0x80 if byte else 0x8000
            if opc == 4:  # PUSH
                v, _ = self.src_operand(reg, As, byte)
                self.r[1] = (self.r[1] - 2) & 0xFFFF
                (self.wb if byte else self.ww)(self.r[1], v)
                return
            if opc == 5:  # CALL
                v, _ = self.src_operand(reg, As, False)
                self.r[1] = (self.r[1] - 2) & 0xFFFF
                self.ww(self.r[1], self.r[0])
                self.r[0] = v
                return
            if opc == 6:  # RETI
                self.r[2] = self.rw(self.r[1]); self.r[1] += 2
                self.r[0] = self.rw(self.r[1]); self.r[1] += 2
                return
            # RRC/SWPB/RRA/SXT operam sobre o operando (registro ou memória)
            if As == 0:
                v, loc = self.r[reg] & mask, ("r", reg)
            else:
                v, addr = self.src_operand(reg, As, byte)
                loc = ("m", addr)
            if opc == 0:  # RRC
                r = (v >> 1) | (msb if self.f(C) else 0)
                self.setf(C, v & 1); self.setf(V, 0)
            elif opc == 1:  # SWPB
                r = ((v << 8) | (v >> 8)) & 0xFFFF
                self.write(loc, r, False)
                return
            elif opc == 2:  # RRA
                r = (v >> 1) | (v & msb)
                self.setf(C, v & 1); self.setf(V, 0)
            elif opc == 3:  # SXT
                r = (v & 0xFF) | (0xFF00 if v & 0x80 else 0)
                self.setf(N, r & 0x8000); self.setf(Z, r == 0)
                self.setf(C, r != 0); self.setf(V, 0)
                self.write(loc, r, False)
                return
            else:
                raise RuntimeError(f"instrução inválida {op:04X} em {pc0:04X}")
            self.setf(N, r & msb); self.setf(Z, r == 0)
            self.write(loc, r, byte)
            return
        if op & 0xF0E0 == 0x0040 and op & 0x0010:  # RRCM/RRAM/RLAM/RRUM .W #n,Rd
            n = ((op >> 10) & 3) + 1
            kind = (op >> 8) & 3
            reg = op & 0xF
            v = self.r[reg]
            for _ in range(n):
                if kind == 2:  # RLAM
                    c = v >> 15
                    v = (v << 1) & 0xFFFF
                else:
                    c = v & 1
                    top = {0: self.f(C) << 15, 1: v & 0x8000, 3: 0}[kind]
                    v = (v >> 1) | top
                self.setf(C, c)
            self.setf(N, v & 0x8000); self.setf(Z, v == 0); self.setf(V, 0)
            self.r[reg] = v
            return
        if op < 0x4000:
            raise RuntimeError(f"instrução MSP430X não suportada {op:04X} em {pc0:04X}")
        # formato I
        opc = op >> 12
        sreg = (op >> 8) & 0xF
        Ad = (op >> 7) & 1
        byte = (op >> 6) & 1
        As = (op >> 4) & 3
        dreg = op & 0xF
        mask = 0xFF if byte else 0xFFFF
        msb = 0x80 if byte else 0x8000
        s, _ = self.src_operand(sreg, As, byte)
        d, loc = self.dst_operand(dreg, Ad, byte)

        def arith(a, b, carry):
            r = a + b + carry
            res = r & mask
            self.setf(C, r > mask)
            self.setf(Z, res == 0)
            self.setf(N, res & msb)
            self.setf(V, (~(a ^ b) & (a ^ res)) & msb)
            return res

        if opc == 0x4:  # MOV
            self.write(loc, s, byte)
        elif opc == 0x5:
            self.write(loc, arith(d, s, 0), byte)
        elif opc == 0x6:
            self.write(loc, arith(d, s, self.f(C)), byte)
        elif opc == 0x7:  # SUBC
            self.write(loc, arith(d, (~s) & mask, self.f(C)), byte)
        elif opc == 0x8:  # SUB
            self.write(loc, arith(d, (~s) & mask, 1), byte)
        elif opc == 0x9:  # CMP
            arith(d, (~s) & mask, 1)
        elif opc == 0xA:  # DADD
            carry = self.f(C)
            res = 0
            for i in range(0, 8 if byte else 16, 4):
                dig = ((s >> i) & 0xF) + ((d >> i) & 0xF) + carry
                carry = 1 if dig > 9 else 0
                if carry:
                    dig -= 10
                res |= (dig & 0xF) << i
            self.setf(C, carry); self.setf(Z, res == 0); self.setf(N, res & msb)
            self.write(loc, res, byte)
        elif opc in (0xB, 0xF):  # BIT / AND
            res = s & d
            self.setf(N, res & msb); self.setf(Z, res == 0)
            self.setf(C, res != 0); self.setf(V, 0)
            if opc == 0xF:
                self.write(loc, res, byte)
        elif opc == 0xC:  # BIC
            self.write(loc, d & ~s & mask, byte)
        elif opc == 0xD:  # BIS
            self.write(loc, d | s, byte)
        elif opc == 0xE:  # XOR
            res = (s ^ d) & mask
            self.setf(N, res & msb); self.setf(Z, res == 0)
            self.setf(C, res != 0); self.setf(V, (s & msb) and (d & msb))
            self.write(loc, res, byte)

    def run(self, entry, max_steps=2_000_000):
        self.r[0] = entry
        try:
            while self.steps < max_steps:
                self.step()
        except Halt:
            return self
        raise RuntimeError("programa não chegou ao laço infinito (jmp $)")
