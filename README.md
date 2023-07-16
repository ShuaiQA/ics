# ICS2022 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2022/

To initialize, run
```bash
bash init.sh subproject-name
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

测试代码进行分析
程序分为4层
1. 用户程序
1. navy-apps(用户库函数)
1. nanos-lite
1. abstract-machine
1. nemu

- 如何验证用户程序是否写的正确

直接将用户程序跑在真机上查看运行的是否符合自己的期望

- 如何验证navy-apps实现是否正确

仿照该目录下的apps和tests的Makefile文件，查看该目录下的README格式要求

```shell
make ISA=native run
```

- 如何验证nanos-lite是否正确

```shell
make ARCH=native update
make ARCH=native run
```

- 验证abstract-machine和nemu

打开difftest，如果difftest没有出现错误，那么就是abstract-machine错误

上面的每一步运行的结果都需要和上一步进行对比。实现正确的话进行下一步。

