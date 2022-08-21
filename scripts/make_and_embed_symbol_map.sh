#/bin/sh

tmpA=$(mktemp)
tmpB=$(mktemp)
"$NM" -C -l -n "${BIN_DIR}"/sysroot/boot/kernel | awk '{ if ($2 != "a" && $2 != "V" && $2 != "d" && $2 != "D") print; }' | uniq | sed -r 's/(\s+)?\S+//2' > "$tmpA"
cat "$tmpA" > "${BIN_DIR}"/printable_symbol_map
printf "%08x\n" "$(wc -l "$tmpA" | awk '{print $1}')" > "$tmpB"
cat "$tmpA" >> "$tmpB"
cat "$tmpB" | tr '\n' '\000' | tr '\t' '\000' > "${BIN_DIR}"/sysroot/boot/symbol_map
"$OBJCOPY" --update-section .kernel_symbols="${BIN_DIR}"/sysroot/boot/symbol_map "${BIN_DIR}"/sysroot/boot/kernel
rm -f "$tmpA"
rm -f "$tmpB"
