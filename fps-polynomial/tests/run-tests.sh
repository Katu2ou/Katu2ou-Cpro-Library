#!/usr/bin/env bash
# tests/run-tests.sh
# CXX=clang++ CXXFLAGS='-O1 -g -fsanitize=address,undefined' tests/run-tests.sh
# バイナリは一時ディレクトリにのみ作成し、終了時に削除する。
set -euo pipefail
test_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/katu2ou-fps-tests.XXXXXX")
trap 'rm -rf -- "$build_dir"' EXIT
if [[ -z "${CXX:-}" ]]; then
  if command -v g++-14 >/dev/null 2>&1; then CXX=g++-14; else CXX=c++; fi
fi
read -r -a flags <<< "${CXXFLAGS:--O2}"
for name in core-sparse composition polynomial sums-relaxed edge-cases matrix-bsgs; do
  "$CXX" -std=c++17 "${flags[@]}" "$test_dir/$name-test.cpp" -o "$build_dir/$name"
  "$build_dir/$name"
done
# C++20 の std::identity と内部関数 identity の名前衝突を検出する。
"$CXX" -std=c++20 "${flags[@]}" "$test_dir/matrix-bsgs-test.cpp" -o "$build_dir/matrix-bsgs-cxx20"
"$build_dir/matrix-bsgs-cxx20"
# 依存ヘッダ不足や一括 include による重複定義も検査する。
for file in formal-power-series fps-newton fps-sqrt fps-sparse fps-composition \
            polynomial-gcd multipoint-evaluation polynomial-interpolation \
            sampling-point-shift faulhaber prefix-sum-polynomial relaxed-multiplication \
            polynomial-matrix-product fps-all; do
  printf '#include "%s.cpp"\nint main() {}\n' "$file" > "$build_dir/include.cpp"
  "$CXX" -std=c++17 -I "$test_dir/.." -fsyntax-only "$build_dir/include.cpp"
done
printf 'All FPS tests passed.\n'
