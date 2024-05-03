section .data
    ; データセクションに変数を定義
    num1    db  10             ; 符号付き8ビット整数
    num2    db  -5             ; 符号付き8ビット整数
    result  dw  0              ; 符号付き16ビット整数、結果を格納する変数

section .text
    global _start

_start:
    ; num1をAXレジスタにロード
    mov al, [num1]
    mov ah, 0        ; AHレジスタを0にクリア

    ; num2を乗算し、結果をAXレジスタに格納
    imul al, [num2]
    
    ; 結果をresult変数に格納
    mov [result], ax

    ; AXレジスタの値を符号拡張し、結果をDXレジスタに格納
    movsx dx, ax

    ; 結果がゼロであれば、"Result is zero."と出力
    test dx, dx
    jz zero_result

    ; 結果がゼロでなければ、"Result is not zero."と出力
    mov     eax, 4          ; システムコール4（write）のシステムコール番号
    mov     ebx, 1          ; 標準出力へのファイルディスクリプタ
    mov     ecx, result_msg ; 出力するメッセージのアドレス
    mov     edx, result_msg_len ; メッセージの長さ
    int     0x80            ; システムコールを呼び出す
    jmp     end_program     ; プログラムを終了

zero_result:
    ; 結果がゼロの場合のメッセージを出力
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, zero_msg
    mov     edx, zero_msg_len
    int     0x80

end_program:
    ; プログラムの終了
    mov     eax, 1          ; システムコール1（exit）のシステムコール番号
    xor     ebx, ebx        ; 終了コード0を指定
    int     0x80            ; システムコールを呼び出す

section .data
    ; 結果がゼロの場合のメッセージ
    zero_msg        db  "Result is zero.", 10
    zero_msg_len    equ $ - zero_msg

    ; 結果がゼロでない場合のメッセージ
    result_msg      db  "Result is not zero.", 10
    result_msg_len  equ $ - result_msg

