#!/bin/bash
# 1行目の#!は、#(hash:ハッシュ/sharp:シャープ)と!(bang:バン)の短縮系で一般的にはshebang:シェバンと呼ばれる
# スクリプトを読み込むパスの指定である

#### シェルの設定 ####
# -x : コマンドの実行時、コマンドと引数の内容を表示を表示
# -u : 未定義の変数を使おうとしたときに打ち止め
# -e : コマンドに失敗した時点でシェルスクリプトの実行を停止
set -xue

#### qemuの設定・操作 ####
# qemuの起動:デフォルトのbios起動(OpenSBI)で実施
# ターミナルにシリアルコンソールとQEMU monitorを表示
# "ctrl-a x"で強制停止
# "ctrl-a c"でコンソールとモニタの切り替えが可能
# qemuの終了: "(qemu) q"
qemu-system-riscv32 -machine virt -bios default -nographic -serial mon:stdio

#### ターミナルでのコマンド集 ####

### 実行コマンド ###
# ./run.sh