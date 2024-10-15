/**
 * @brief 各種定義
 * @note
 */
#define NULL ((void *)0) // ヌルポインタ
#define STACK_SIZE 8149  // スタックサイズ
#define THREAD_MAX_NUM 8 // スレッドの最大数
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

    /*
        インラインアセンブラとは
        オペコードは、アセンブリの命令名、オペランドは、アセンブリ命令の引数を示します。
        出力オペランドは、アセンブリコードが結果を格納する場所を指定します。
        "=r"(var) のように、=rでレジスタを使用し、varに格納できます。
        なお、r(レジスタに結果を書き込む), +(オペランドが入力でも出力でもある), -(通常は使用しない)があります。
        入力オペランドは、アセンブリコードに渡される入力値を指定します。
        "r"(var) で、varの値がレジスタに格納されてアセンブリコードに渡されます。
        なお、r(レジスタオペランド)、m（メモリ）,i（即値）,n（整数定数）,g（汎用オペランド）があります。
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
        :);

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
 * @brief コンパイラが提供する組み込み関数や型
 * @details __builtinは、特定のコンパイラ（特にGCCやClang）が提供するものであり、
 *          標準C/C++言語仕様には含まれていません。
 *          特定のコンパイラに依存するため、他のコンパイラでは利用できない場合があります。
 */
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
/**
 * @brief 文字列表示処理
 * @param fmt   : 表示する文字列データ もしくは フォーマット指定子の設定
 * @param ...   : 可変長引数 (表示データ)
 * @details
 */
void printf(const char *fmt, ...)
{
    int value = 0;
    int divisor = 0;
    const char *s;

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
            case '\0':         // 終端文字の場合
                putchar('%');  //
                va_end(vargs); // 可変長引数の取得を終了
                return;
                break;
            case '%':         // %表示の場合
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
                divisor = 1;
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
 * @brief トラップハンドラ処理
 * @param なし
 * @details CPUがエラーや特別な状況を検知したときに、それをOSに通知して適切に処理するための仕組み
 */
__attribute__((aligned(4))) /* アライメントを４バイト境界でスタックの割り当て */
void
trap_handler(void)
{
    unsigned int scause = 0; // 例外や割り込み時の原因
    unsigned int sepc = 0;   // 例外や割り込み時のプログラムカウンタ

    /*
        インラインアセンブラとは
        オペコードは、アセンブリの命令名、オペランドは、アセンブリ命令の引数を示します。
        出力オペランドは、アセンブリコードが結果を格納する場所を指定します。
        "=r"(var) のように、=rでレジスタを使用し、varに格納できます。
        なお、r(レジスタに結果を書き込む), +(オペランドが入力でも出力でもある), -(通常は使用しない)があります。
        入力オペランドは、アセンブリコードに渡される入力値を指定します。
        "r"(var) で、varの値がレジスタに格納されてアセンブリコードに渡されます。
        なお、r(レジスタオペランド)、m（メモリ）,i（即値）,n（整数定数）,g（汎用オペランド）があります。
        __asm__ (
          ”オペコード オペランド1, オペランド2, ... ,"    <アセンブリテンプレート : ％0,%1...は、％番目の出力/入力オペランドで指定したレジスタを示す>
        : "制約"(C言語中の変数名)                       <出力オペランド : レジスタの値をC言語の変数へ出力するもの。通常は、"＝r"を指定する>
        : "制約"(C言語中の変数名)                       <入力オペランド : C言語の変数をレジスタに設定する。”r”を指定しなければならない>
        :  破壊されるレジスタのリスト                    <レジスタの値が変更されてしまい、影響を与えてしまう項目>
    */

    // 例外や割り込みが発生すると、scauseレジスタにはその原因を示すビットがセットされる
    // sepcレジスタは、例外や割り込みが発生した際のプログラムカウンタ（PC）の値を保持される
    __asm__ __volatile__(
        "csrr %0, scause\n"        /* %0にscauseレジスタの内容を読み込む */
        "csrr %1, sepc\n"          /* %1にsepcレジスタの内容を読み込む */
        : "=r"(scause), "=r"(sepc) /* 上記の結果をC言語の変数scauseとsepcに書き込む */
    );
    printf("trap: scause = 0x%x, sepc = 0x%x\n", scause, sepc);
    for (;;)
        ;
}
/**
 * @brief コンテキストスッチの処理
 * @param prev_sp   : 前回のスタックポインタ
 * @param next_sp   : 次回のスタックポインタ
 * @details 現在のレジスタの状態を保存し、新しいスタックポインタに切り替えてから、以前の状態を復元します。
 */
__attribute__((naked)) /* 通常の関数処理を無効化 (関数が通常の関数呼び出しや戻り処理をしない) */
void
switch_context(unsigned int *prev_sp, unsigned int *next_sp)
{
    /*
        switch_contextが呼ばれると、a0/a1レジスタにprev_sp/next_spが設定され、
        スタックポインタの位置の更新、フレームポインタの設定などが行われます。
    */
    __asm__ __volatile__(
        /* 新しいスタックフレームを作成して、レジスタの値を保存する領域を作る */
        "addi sp, sp, -13 * 4 \n"
        /* レジスタの値をスタック(メモリ)に保存 (レジスタの保存) */
        /* s1からs11の保存レジスタは、関数の呼び出し間でデータを保持するもので、関数をまたいでもその値が保持される */
        "sw ra,   0 * 4(sp)\n" /* 現在のspの位置にリターンアドレスを保存 */
        "sw s0,   1 * 4(sp)\n" /* 現在のspの位置から4バイトずらした位置にフレームポインタを保存 */
        "sw s1,   2 * 4(sp)\n" /* 現在のspの位置から8バイトずらした位置に保存レジスタを保存 */
        "sw s2,   3 * 4(sp)\n" /* 現在のspの位置から12バイトずらした位置に保存レジスタを保存 */
        "sw s3,   4 * 4(sp)\n" /* 現在のspの位置から16バイトずらした位置に保存レジスタを保存 */
        "sw s4,   5 * 4(sp)\n" /* 現在のspの位置から20バイトずらした位置に保存レジスタを保存 */
        "sw s5,   6 * 4(sp)\n" /* 現在のspの位置から24バイトずらした位置に保存レジスタを保存 */
        "sw s6,   7 * 4(sp)\n" /* 現在のspの位置から28バイトずらした位置に保存レジスタを保存 */
        "sw s7,   8 * 4(sp)\n" /* 現在のspの位置から32バイトずらした位置に保存レジスタを保存 */
        "sw s8,   9 * 4(sp)\n" /* 現在のspの位置から36バイトずらした位置に保存レジスタを保存 */
        "sw s9,  10 * 4(sp)\n" /* 現在のspの位置から40バイトずらした位置に保存レジスタを保存 */
        "sw s10, 11 * 4(sp)\n" /* 現在のspの位置から44バイトずらした位置に保存レジスタを保存 */
        "sw s11, 12 * 4(sp)\n" /* 現在のspの位置から48バイトずらした位置に保存レジスタを保存 */
        /* 現在のスタックポインタの位置をa0が指すアドレスに保存 */
        /* a0は、関数呼び出しの第1引数であり、prev_spをとなる */
        "sw sp,  (a0)\n"
        /* a1が指すデータを現在のスタックポインタにする */
        /*
            a1は、関数呼び出しの第2引数であり、next_spをとなる
            next_spのスタックのアドレスをレジスタのスタックポインタに設定する
            ここからnext_spのスタックをレジスタが処理する
        */
        "lw sp,  (a1)\n"
        /* ここから、next_spのスタックに保存していた値をレジスタにロード (レジスタの復元) */
        "lw ra,   0 * 4(sp)\n" /* 現在のsp位置のデータをリターンアドレスに設定 */
        "lw s0,   1 * 4(sp)\n" /* 現在のsp位置から4バイト先のデータをフレームポインタに設定 */
        "lw s1,   2 * 4(sp)\n" /* 現在のsp位置から8バイト先のデータを保存レジスタに設定 */
        "lw s2,   3 * 4(sp)\n" /* 現在のsp位置から12バイト先のデータを保存レジスタに設定 */
        "lw s3,   4 * 4(sp)\n" /* 現在のsp位置から16バイト先のデータを保存レジスタに設定 */
        "lw s4,   5 * 4(sp)\n" /* 現在のsp位置から20バイト先のデータを保存レジスタに設定 */
        "lw s5,   6 * 4(sp)\n" /* 現在のsp位置から24バイト先のデータを保存レジスタに設定 */
        "lw s6,   7 * 4(sp)\n" /* 現在のsp位置から28バイト先のデータを保存レジスタに設定 */
        "lw s7,   8 * 4(sp)\n" /* 現在のsp位置から32バイト先のデータを保存レジスタに設定 */
        "lw s8,   9 * 4(sp)\n" /* 現在のsp位置から36バイト先のデータを保存レジスタに設定 */
        "lw s9,  10 * 4(sp)\n" /* 現在のsp位置から40バイト先のデータを保存レジスタに設定 */
        "lw s10, 11 * 4(sp)\n" /* 現在のsp位置から44バイト先のデータを保存レジスタに設定 */
        "lw s11, 12 * 4(sp)\n" /* 現在のsp位置から48バイト先のデータを保存レジスタに設定 */
        /* スタックポインタの位置を戻す */
        "addi sp, sp, 13 * 4 \n"
        /* 終了 */
        "ret\n");
}
/**
 * @brief 実行状態の定義
 * @note スレッドやプロセスの状態を示す
 */
typedef enum
{
    READY,     // 実行可能プロセス
    RUNNING,   // 実行中
    WAITING,   // 実行待ち
    TERMINATED // ターミネート(終了状態/未設定状態)
} ExecutionState;
/**
 * @brief 実行管理エンティティを示す構造体
 * @note スレッドやプロセスの状態とIDを管理する
 */
typedef struct
{
    ExecutionState status; // 状態
    int id;                // ID (プロセスやスレッドの識別)
} Execution;
/**
 * @brief スレッド
 * @note
 */
struct thread
{
    Execution execution;    // 実行管理エンティティ
    unsigned int sp;        // スレッドのスタックポインタ
    char stack[STACK_SIZE]; // スレッドのスタック領域
};
/**
 * @brief スレッド(グローバル変数)
 * @note　各種スレッド関連のデータ
 */
struct thread g_thread_list[THREAD_MAX_NUM]; // スレッド
struct thread *g_idle_thread;                // アイドル(何もしない)スレッド
struct thread *g_current_thread;             // 現在実行中のスレッド
/**
 * @brief スレッドリストの初期設定
 * @details グローバルのスレッドリストの情報を初期化する
 */
void init_threads(void)
{
    for (int i = 0; i < THREAD_MAX_NUM; i++)
    {
        g_thread_list[i].execution.id = 0;
        g_thread_list[i].execution.status = TERMINATED;
    }
    return;
}
/**
 * @brief スレッドの作成
 * @param void (*entry)(void)   : スレッドのエントリー関数のポインタ
 * @details スレッドリストにスレッドを設定し、スレッドを使用可能な状態にする
 */
struct thread *create_thread(void (*entry)(void))
{
    // 空いているスレッドを探す
    struct thread *thread = NULL;
    int i = 0;
    for (i = 0; i < THREAD_MAX_NUM; i++)
    {
        if (g_thread_list[i].execution.status == TERMINATED)
        {
            thread = &g_thread_list[i];
            break;
        }
    }
    // コンテキストスイッチ用のレジスタの初期設定
    unsigned int *sp = (unsigned int *)&thread->stack[sizeof(thread->stack) / sizeof(thread->stack[0])];
    *--sp = 0;                   // s11
    *--sp = 0;                   // s10
    *--sp = 0;                   // s9
    *--sp = 0;                   // s8
    *--sp = 0;                   // s7
    *--sp = 0;                   // s6
    *--sp = 0;                   // s5
    *--sp = 0;                   // s4
    *--sp = 0;                   // s3
    *--sp = 0;                   // s2
    *--sp = 0;                   // s1
    *--sp = 0;                   // s0
    *--sp = (unsigned int)entry; // ra
    // スレッドの初期設定
    thread->execution.id = i + 1;
    thread->execution.status = READY;
    thread->sp = (unsigned int)sp;
    printf("thread(sp:0x%x) 0x%x\n", thread->sp, &thread->stack[STACK_SIZE - 1]);
    return thread;
}
/**
 * @brief 全てのスレッドが終了状態であるかどうか
 * @retval  0   :   動作中のスレッドあり
 * @retval  1   :   全てのスレッドが終了状態である
 * @details 詳細説明
 */
int are_all_threads_terminated(void)
{
    // 全スレッドの状態をチェック
    for (int i = 0; i < THREAD_MAX_NUM; i++)
    {
        if ((g_thread_list[i].execution.status != TERMINATED) && (g_thread_list[i].execution.id > 0))
        {
            return 0; // まだ動作中のスレッドがある
        }
    }
    return 1; // 全スレッドが終了している
}
/**
 * @brief スレッドスケジューラ
 * @details 現在のスレッドを休ませて、次に動作するスレッドを探索し、スレッドを動作させる
 *          スレッドの切り替えを行うスケジュール関数
 */
void schedule_threads(void)
{
    struct thread *next = NULL;

    // 次に動作するスレッドを探す
    for (int i = 0; i < THREAD_MAX_NUM; i++)
    {
        struct thread *thread = &g_thread_list[(g_current_thread->execution.id + i) % THREAD_MAX_NUM];
        if ((thread->execution.status == READY) && (thread->execution.id > 0))
        {
            thread->execution.status = RUNNING;
            next = thread;
            break;
        }
    }
    // 実行可能なスレッドがない場合は、アイドルスレッドに設定
    if (next == NULL)
    {
        g_idle_thread->execution.status = RUNNING;
        next = g_idle_thread;
    }
    // コンテキストスイッチを行う
    struct thread *prev = g_current_thread;
    if (g_current_thread->execution.status == RUNNING)
    {
        prev->execution.status = READY;
    }
    g_current_thread = next;
    switch_context(&prev->sp, &next->sp);
}
/**
 * @brief アイドル(何もしない)スレッドの処理
 * @details 何もしないスレッドであるアイドルスレッドの処理
 */
void entry_idle_thread(void)
{
    while (1)
    {
        // 何もしない（省電力のためにNOPを入れることもある）
        __asm__ volatile("nop");
    }
}
/**
 * @brief スレッドのエントリー関数処理
 * @details スレッドで実施する処理内容
 */
void entry_thread(void)
{
    for (int i = 0; i < 2; i++)
    {
        // スレッドの情報
        printf("thread_start_%d(id:%d sp:0x%x) \n", i, g_current_thread->execution.id, g_current_thread->sp);
        schedule_threads();
        // スタックのデータを確認
        for (int j = STACK_SIZE; j > 0; j--)
        {
            // スタックのデータが設定されている場合
            if (g_current_thread->stack[j - 1] != 0)
            {
                printf("(thread%d)sp=0x%x stack%d:0x%x(0x%x)\n",
                       g_current_thread->execution.id,
                       g_current_thread->sp,
                       j,
                       g_current_thread->stack[j - 1],
                       &g_current_thread->stack[j - 1]);
            }
        }
        printf("-----------------------------------------\n");
    }
    g_current_thread->execution.status = TERMINATED;
}
/**
 * @brief カーネルメイン処理
 * @param なし
 * @details 詳細説明
 */
void kernel_main(void)
{
    // RISC-Vアーキテクチャにおけるトラップハンドラの設定
    __asm__ __volatile__(
        "csrw stvec, %0\n"  /* stvecレジスタにトラップハンドラのアドレスを設定 */
        ::"r"(trap_handler) /* 入力オペランド: トラップハンドラのアドレス */
    );
    // Hellow Worldの表示
    printf("Hello World\n");
    // printf機能の確認
    printf("0x%x\n", 0x1234abcd);
    printf("%d\n", 999999);
    printf("%d\n", -999999);
    // スレッドの初期化
    init_threads();
    // アイドルスレッドの作成
    g_idle_thread = create_thread(entry_idle_thread);
    g_idle_thread->execution.id = 0;
    g_current_thread = g_idle_thread;
    // スレッドの生成
    create_thread(entry_thread);
    create_thread(entry_thread);
    // スケジューラの動作
    printf("thread start\n");
    while (!are_all_threads_terminated())
    {
        schedule_threads();
    }
    printf("thread finished\n");
    // 無限ループ
    for (;;)
        ;
}
/**
 * @brief エントリー関数
 * @param なし
 * @details 詳細説明
 * @note __attribute__キーワードで配置先のセクション名を指定すると、指定されたセクションに変数や関数が配置
 */
__attribute__((aligned(4))) char boot_stack[STACK_SIZE]; /* アライメントを４バイト境界でスタックの割り当て */
__attribute__((section(".text.boot")))                   /* boot関数をtext.bootに配置 (リンカスクリプトの先頭に配置) */
__attribute__((naked))                                   /* 通常の関数処理を無効化 (関数が通常の関数呼び出しや戻り処理をしない) */
void
boot(void)
{
    /*
        インラインアセンブラとは
        オペコードは、アセンブリの命令名、オペランドは、アセンブリ命令の引数を示します。
        出力オペランドは、アセンブリコードが結果を格納する場所を指定します。
        "=r"(var) のように、=rでレジスタを使用し、varに格納できます。
        なお、r(レジスタに結果を書き込む), +(オペランドが入力でも出力でもある), -(通常は使用しない)があります。
        入力オペランドは、アセンブリコードに渡される入力値を指定します。
        "r"(var) で、varの値がレジスタに格納されてアセンブリコードに渡されます。
        なお、r(レジスタオペランド)、m（メモリ）,i（即値）,n（整数定数）,g（汎用オペランド）があります。
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
