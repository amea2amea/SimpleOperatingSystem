/**
 * @brief SBI(Supervisor Binary Interface)の戻り値
 * @note スーパーバイザ (S モード OS) とスーパーバイザ間のシステム コール形式の呼び出し規則
 */
struct sbiret
{
    long error;
    long value;
};
/**
 * @brief SBI(Supervisor Binary Interface)関数のECALL呼び出し
 * @param eid   : Extension ID
 * @param fid   : Function ID
 * @param arg0  : 引数
 * @param arg1  : 引数
 * @param arg2  : 引数
 * @details Supervisor Execution Environment(SEE)としてEALL関数を呼び出す
 * @note ECALLは、スーパーバイザとSEE間の制御転送命令として使用するもの
 */
struct sbiret sbi_call(long eid, long fid, long arg0, long arg1, long arg2)
{
    // SBIのバイナリエンコーディング
    // バイナリエンコードとは、アセンブリからバイナリへの変換プロセスを指す
    // 本関数により、SBIコマンドやSモードの命令をバイナリ形式でエンコードし、RISC-Vにより実行
    // CALL仕様に基づいてレジスタに設定
    // a0〜a5レジスタには、引数を設定 (今回は引数３つまで)
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    // a7レジスタには、Extension IDを設定
    register long a7 __asm__("a7") = eid;
    // a6レジスタには、Function ID設定
    register long a6 __asm__("a6") = fid;

    /*  インラインアセンブラとは
        オペコードは、アセンブリの命令名、オペランドは、アセンブリ命令の引数を示します。
        出力オペランドには、通常は、"=r"と指定しますが、"+"や"-"を設定することが可能です。
        入力オペランドは、"r"のみ指定可能である。
        なお、r以外に、m,i,n,gなどがあります。
        __asm__ ( 
          ”オペコード オペランド1, オペランド2, ... ,"    <アセンブリテンプレート : ％0,%1...は、％番目の出力/入力オペランドで指定したレジスタを示す>
        : "制約"(C言語中の変数名)                       <出力オペランド : レジスタの値をC言語の変数へ出力するもの。通常は、"＝r"を指定する>
        : "制約"(C言語中の変数名)                       <入力オペランド : C言語の変数をレジスタに設定する。”r”を指定しなければならない>
        :  破壊されるレジスタのリスト                    <レジスタの値が変更されてしまい、影響を与えてしまう項目>
    */
    __asm__ __volatile__(
        "ecall"
        : "=r"(a0), "=r"(a1)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a6), "r"(a7)
        :
    );

    return (struct sbiret){.error = a0, .value = a1};
}
/**
 * @brief 1文字表示処理
 * @param ch :
 * @details １文字を表示するシステムコールを呼び出す
 */
void putchar(char ch)
{
    sbi_call(0x01, 0, ch, 0, 0);
}
/**
 * @brief 文字列表示処理
 * @param fmt   : 表示する文字列データ もしくは フォーマット指定子の設定
 * @param ...   : 可変長引数 (表示データ)
 * @details
 */
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
void printf(const char *fmt, ...)
{
    int value = 0;
    int divisor = 1;
    char* s;

    // 可変長引数の設定
    // 第1引数は、可変長の情報をまとめるための変数
    // 第2引数は、第1引数のアドレス位置を設定。通常は、可変引数の情報が始まる1つ前のアドレス位置を設定
    va_list vargs;        // 可変長リストを格納できる型(va_list)
    va_start(vargs, fmt); // 可変長引数の情報を1つの変数にまとめる
      
    // 指定した文字列データを表示する処理
    while (*fmt)
    {
        if (*fmt == '%') // フォーマット指定子の場合
        {
            // フォーマット指定子まで進める
            fmt++;
            // それぞれのフォマット指定子で表示処理
            switch (*fmt)
            {
            case '\0': // 終端文字の場合
                putchar('%');  //
                va_end(vargs); // 可変長引数の取得を終了
                return;
                break;
            case '%': // %表示の場合
                putchar('%'); //
                break;
            case 's': // 文字列表示の場合
                // 1つずつ文字を取り出す
                // 第1引数がフォーマット指定子のアドレス位置を指す
                // 第2引数は、第1引数のアドレス位置から指定したサイズ (今回はchar型サイズ)のデータを取得
                s = va_arg(vargs, const char *);
                // 1文字ずつ表示
                while (*s)
                {
                    putchar(*s);
                    s++;
                }
                break;
            case 'd': // 整数データの場合
                // 1つずつ文字を取り出す
                // 第1引数がフォーマット指定子のアドレス位置を指す
                // 第2引数は、第1引数のアドレス位置から指定したサイズ (今回はint型サイズ)のデータを取得
                value = va_arg(vargs, signed int);
                // 負の数の場合、マイナスの符号を表示
                if (value < 0)
                {
                    putchar('-');   // 負の数を表示
                    value = -value; // データをプラスにしておく
                }
                // 整数の桁数を求める
                while ((value / divisor) > 9)
                    divisor *= 10;
                // 文字に変換
                // 桁数が大きいものから順番に表示
                while (divisor > 0)
                {
                    // 文字を表示し、整数を除数で丸め、除数の桁数を下げていく
                    putchar('0' + (value / divisor));
                    value %= divisor;
                    divisor /= 10;
                }
                break;
            case 'x': // 16進数表示の場合
                // 1つずつ文字を取り出す
                // 第1引数がフォーマット指定子のアドレス位置を指す
                // 第2引数は、第1引数のアドレス位置から指定したサイズ (今回はunsigned int型サイズ)のデータを取得
                value = va_arg(vargs, unsigned int);
                // 16進数に変換
                // 4バイトなので、4ビットずつ取り出すと8回となる
                for (int i = 7; i >= 0; i--)
                {
                    // 4バイトの情報を4ビットずつ8個(0~7)取り出す
                    int nibble = (value >> (i * 4)) & 0xf;
                    putchar("0123456789abcdef"[nibble]);
                }
                break;
            default:
                break;
            }
        }
        else // フォーマット指定子でない場合
        {
            putchar(*fmt); // 文字をそのまま表示
        }
        // 指定した文字列を進める
        fmt++;
    }
    // 可変長引数の取得を終了
    va_end(vargs);
}
/**
 * @brief カーネルメイン処理
 * @param なし
 * @details 詳細説明
 */
void kernel_main(void)
{
    printf("Hello World\n");
    printf("0x%x\n", 0x1234abcd);
    printf("%d\n", 999999); 
    printf("%d\n", -999999);
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
    /*  インラインアセンブラとは
        オペコードは、アセンブリの命令名、オペランドは、アセンブリ命令の引数を示します。
        出力オペランドには、通常は、"=r"と指定しますが、"+"や"-"を設定することが可能です。
        入力オペランドは、"r"のみ指定可能である。
        なお、r以外に、m,i,n,gなどがあります。
        __asm__ ( 
          ”オペコード オペランド1, オペランド2, ... ,"    <アセンブリテンプレート : ％0,%1...は、％番目の出力/入力オペランドで指定したレジスタを示す>
        : "制約"(C言語中の変数名)                       <出力オペランド : レジスタの値をC言語の変数へ出力するもの。通常は、"＝r"を指定する>
        : "制約"(C言語中の変数名)                       <入力オペランド : C言語の変数をレジスタに設定する。”r”を指定しなければならない>
        :  破壊されるレジスタのリスト                    <レジスタの値が変更されてしまい、影響を与えてしまう項目>
    */
    __asm__ __volatile__(
        "mv sp, %0\n"                          /* boot_stackの末端をスタックポインタへ設定 */
        "call kernel_main\n"                   /* karnel_mainを呼び出す */
        :                                      /* 出力オペランドはなし */
        : "r"(&boot_stack[sizeof(boot_stack)]) /* スタックの末端アドレスを汎用レジスタに設定(スタックは末端から使用される) */
        :);
}
