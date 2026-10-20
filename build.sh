#!/usr/bin/env bash
# Compila cada arquivo .asm/.c como um programa independente para o MSP430F5529
# usando o compilador TI (cl430) que vem com o Code Composer Studio.
#
# Uso:  ./build.sh                 (compila tudo)
#       ./build.sh modulo1/ex03.asm (compila só um arquivo)
#
# Variável CCS_ROOT pode apontar para outra instalação do CCS.

set -u
CCS_ROOT="${CCS_ROOT:-/c/ti/ccs2100/ccs}"
CGT="$(ls -d "$CCS_ROOT"/tools/compiler/ti-cgt-msp430_* | sort | tail -1)"
INC="$CCS_ROOT/ccs_base/msp430/include"
CL="$CGT/bin/cl430"
# gmake do CCS: o linker o usa para gerar a biblioteca RTS do modelo "small"
# na primeira compilação (pode demorar alguns minutos).
export PATH="$CCS_ROOT/utils/bin:$PATH"

FLAGS=(-vmspx --code_model=small --data_model=small --use_hw_mpy=F5 --define=__MSP430F5529__
       --include_path="$INC" --include_path="$CGT/include"
       --advice:power=none -g --diag_warning=225 --display_error_number
       --printf_support=minimal)

ROOT="$(cd "$(dirname "$0")" && pwd)"
OUT="$ROOT/build"
mkdir -p "$OUT"

if [ $# -gt 0 ]; then files=("$@"); else
  mapfile -t files < <(cd "$ROOT" && find modulo1 modulo2 modulo3 -name '*.asm' -o -name '*.c' | sort)
fi

ok=0; fail=0; failed=()
for f in "${files[@]}"; do
  name="$(echo "${f%.*}" | tr '/' '_')"
  obj_dir="$OUT/$name"; mkdir -p "$obj_dir"
  if "$CL" "${FLAGS[@]}" --obj_directory="$obj_dir" "$ROOT/$f" \
        -z -m"$obj_dir/$name.map" -i"$CGT/lib" -i"$INC" --reread_libs \
        --rom_model -o "$obj_dir/$name.out" "$INC/lnk_msp430f5529.cmd" -llibc.a \
        > "$obj_dir/build.log" 2>&1; then
    ok=$((ok+1))
    grep -i "warning" "$obj_dir/build.log" | grep -v -e "^>> Compilation" -e "#10366" | sed "s|^|[warn] $f: |"
  else
    fail=$((fail+1)); failed+=("$f")
    echo "=== FALHOU: $f"; cat "$obj_dir/build.log"
  fi
done
echo "Compilados com sucesso: $ok | Falhas: $fail"
[ $fail -eq 0 ]
