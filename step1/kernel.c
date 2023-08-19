/**
 * @brief カーネルメイン処理
 * @param なし
 * @details 詳細説明
 */
void kernel_main(void)
{
    for (;;)
        ;
}
/**
 * @brief アライメントを４バイト境界でスタックの割り当て （グローバル変数を配置)
 * @note
 */
__attribute__((aligned(4))) char boot_stack[8149];
/**
 * @brief エントリー関数
 * @param なし
 * @details 詳細説明
 * @note __attribute__キーワードで配置先のセクション名を指定すると、指定されたセクションに変数や関数が配置
 */
__attribute__((section(".text.boot"))) /* boot関数をtext.bootに配置 (リンカスクリプトの先頭に配置) */
__attribute__((naked))                 /* 最適化を無効 */
void
boot(void)
{
    /* インラインアセンブラ
        __asm__ ( アセンブリテンプレート
        : 出力オペランド
        : 入力オペランド
        : 破壊されるレジスタのリスト
        */
    __asm__ __volatile__(
        "mv sp, %0\n"                          /* スタックポインタにスタックのトップアドレスを設定 */
        "call kernel_main\n"                   /* karnel_mainを呼び出す */
        :                                      /**/
        : "r"(&boot_stack[sizeof(boot_stack)]) /* スタックのアドレスをレジスタに設定(スタックは末端から使用される) */
        :);
}
