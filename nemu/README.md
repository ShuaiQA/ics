# NEMU

# 介绍

nemu是一个传入的程序是一段机器代码，然后逐步的执行该机器代码然后输出理想结果的程序。

首先是进行初始化的过程，包括下面的几个步骤

- 参数的处理 "-bhe:l:d:p:"
- 设置随机数种子，初始化内存空间
- 设置日志文件
- 初始化设备和elf文件
- 加载对应的img文件到对应的内存空间，待后续执行
- 进行diff对比测试
- 简单的sdb(simple debugger)调试

之后就是nemu模拟器按照每一条指令进行执行的过程





NEMU(NJU Emulator) is a simple but complete full-system emulator designed for teaching purpose.
Currently it supports x86, mips32, riscv32 and riscv64.
To build programs run above NEMU, refer to the [AM project](https://github.com/NJU-ProjectN/abstract-machine).

The main features of NEMU include
* a small monitor with a simple debugger
  * single step
  * register/memory examination
  * expression evaluation without the support of symbols
  * watch point
  * differential testing with reference design (e.g. QEMU)
  * snapshot
* CPU core with support of most common used instructions
  * x86
    * real mode is not supported
    * x87 floating point instructions are not supported
  * mips32
    * CP1 floating point instructions are not supported
  * riscv32
    * only RV32IM
  * riscv64
    * only RV64IM
* memory
* paging
  * TLB is optional (but necessary for mips32)
  * protection is not supported
* interrupt and exception
  * protection is not supported
* 5 devices
  * serial, timer, keyboard, VGA, audio
  * most of them are simplified and unprogrammable
* 2 types of I/O
  * port-mapped I/O and memory-mapped I/O
