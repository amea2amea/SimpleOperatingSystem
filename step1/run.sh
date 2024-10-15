#!/bin/bash
# 1行目の#!は、#(hash:ハッシュ/sharp:シャープ)と!(bang:バン)の短縮系で一般的にはshebang:シェバンと呼ばれる
# スクリプトを読み込むパスの指定である

#### シェルの設定 ####
# -x : コマンドの実行時、コマンドと引数の内容を表示を表示
# -u : 未定義の変数を使おうとしたときに打ち止め
# -e : コマンドに失敗した時点でシェルスクリプトの実行を停止
set -xue

#### コンパイルの設定 ####
# kernel.cをコンパイルし、(-Tオプション)のリンカスクリプト(kernel.ld)を渡して(-Wlオプション)、ELF形式のファイルを作成
# -02 : 最適化レベル2
# -g3 : デバッグ情報
# -Wall -Wextra : 警告
# -ffreestanding : 標準ライブラリを使用しない環境でのコンパイル
# -nostdlib : 標準ライブラリを使用しない設定
# -T<script>: <script>をリンカスクプトとして使用
# -Wl,<arg> : リンカにカンマ区切りの引数を渡す。今回の場合、kernel.ld
CC=/opt/homebrew/opt/llvm/bin/clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib"
$CC $CFLAGS -Wl,-Tkernel.ld -o kernel.elf kernel.c 

#### qemuの設定・操作 ####
# qemuの起動:デフォルトのbios起動(OpenSBI)で実施
# ターミナルにシリアルコンソールとQEMU monitorを表示
# "ctrl-a x"で強制停止
# "ctrl-a c"でコンソールとモニタの切り替えが可能
# qemuの終了: "(qemu) q"
qemu-system-riscv32 -machine virt -bios default -nographic -serial mon:stdio \
 -kernel kernel.elf

#### ターミナルでのコマンド集 ####

### 実行コマンド ###
# ./run.sh

### 実行中の情報をqemuコマンドで確認 ###

## レジスタの情報 ##
# (qemu) info registers
# プログラムカウンタ(pc)のレジスタを確認(8020000c)

### ここからはqemuを(qemu) qで終了し、実行モジュールの情報をllvm関連のコマンドで確認 ###

## アドレスに関連づけているファイル名と行番号を取得 (実行ファイルは、-eオプションで確認) ##
# llvm-addr2line -e kernel.elf 8020000c(プログラムカウンタ値)
# 実行中のソース位置を確認

## 逆アセンブル表示 ##
# llvm-objdump -d kernel.elf

## シンボルの情報 ##
# llvm-nm kernel.elf
