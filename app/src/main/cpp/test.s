.text
 .globl addme
      .extern addme

  addme:
    ;ldr r0,[pc,#12]
    bx r0
    add r0, r1 //r0=r0+r1
    bx  lr //返回方法调用入口，结果使用 r0 返回
.word 0x12345678

