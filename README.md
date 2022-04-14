<p align="center">
 <img width="100px" src="https://raw.githubusercontent.com/NekoSilverFox/NekoSilverfox/403ab045b7d9adeaaf8186c451af7243f5d8f46d/icons/silverfox.svg" align="center" alt="NekoSilverfox" />
 <h1 align="center">Cmilan</h1>
 <p align="center"><b>编译器及拓展</b></p>
</p>


<div align=center>


[![License](https://img.shields.io/badge/license-Apache%202.0-brightgreen)](LICENSE)


<div align=left>
<!-- 顶部至此截止 -->




<!-- SPbSTU  -->

 <p align="center">
  <img width="250px" src="https://github.com/NekoSilverFox/NekoSilverfox/blob/master/icons/logo_building_spbstu.png?raw=true" align="center" alt="ogo_building_spbstu" />
  </br>
  </br>
  <b><b>Санкт-Петербургский государственный политехнический университет</b></br></b>
  <b>Институт компьютерных наук и технологий</b>
 </p>
 <p align="center"></p>

</p>

<div align=left>
<!-- SPbSTU 最后一行 -->


# 编译及执行

## 编译 MiLan编译器

使用 Makefile 进行编译，进入带有 MiLan 编译器源代码的文件夹（cmilan），使用以下 `make` 命令进行编译。生成的编译器名为 `cmilan` 二进制可执行文件



Makefile 内容：

```makefile
CFLAGS	= -Wall -W -Werror -O2 -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-unused-private-field -Wno-unused-label
LDFLAGS	=

HEADERS	= scanner.h \
	  parser.h \
	  codegen.h

OBJS	= main.o \
	  codegen.o \
	  scanner.o \
	  parser.o \
	  
EXE	= cmilan

$(EXE): $(OBJS) $(HEADERS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	-@rm -f $(EXE) $(OBJS)

```



## 将 MiLan 代码编译为虚拟机汇编代码

**MiLan 代码编写的文件，应该以 `.mil` 为文件后缀存储**

在获得编译器二进制可执行文件 `cmilan` 后，使用 `./cmilan FILENAME.mil` 可生成 MiLan 虚拟机汇编代码。也可使用 `./cmilan FILENAME.mil > FILENAME.ms` 将生成的 MiLan 虚拟机汇编代码写入到文件中



## 使用 MiLan 虚拟机执行汇编代码

存放虚拟机的文件夹为 `vm`，虚拟机为内部名为 `mvm` 的二进制文件。使用 `./mvm FILENAME.ms` 可执行汇编代码






# 文件及职责



## Parser

Parser 实现了**句法分析器**，实现了 3 大功能：

- 检查程序的正确性
- 在分析过程中为虚拟机生成代码（类似汇编代码）
- 初步的错误恢复

解析器使用初始化过程中传递给它的词法分析器，一次读取一个词组，并根据米兰语法为可堆叠虚拟机生成代码。句法分析是通过递归下降法进行的。

当检测到错误时，解析器会打印出一条信息，并从下一条语句开始继续分析，以尽可能多地发现解析过程中的错误。因为错误恢复策略非常简单，所以有可能为不存在的（"诱发"）错误打印信息，或者跳过一些错误而不打印信息。如果在解析过程中发现**至少**一个错误，则不打印虚拟机的代码。



| 函数名                                    | 描述                                                         |
| ----------------------------------------- | ------------------------------------------------------------ |
| `void program()`                          | 解析程序。BEGIN statementList END。                          |
| `void statementList()`                    | 解析运算符的列表：如果运算符列表为空，下一个标记将是可能的 "结束括号 "之一：END、OD、ELSE、FI。 在这种情况下，解析的结果将是一个空块（其运算符列表为空）。 如果下一个标记不在这个列表中，我们认为它是一个运算符的开始，并调用语句方法。 【重点】最后一个声明的标志是运算符后面没有分号。 |
| `void statement()`                        | 运算符分析                                                   |
| `void expression()`                       | 解析算术表达式                                               |
| `void term()`                             | 求和                                                         |
| `void factor()`                           | 乘法分析                                                     |
| `void relation()`                         | 条件解析                                                     |
| `bool see(Token t)`                       | 当前词素与模式的比较。词素流中的当前位置不改变               |
| `bool match(Token t)`                     | 检查当前词素是否与模式匹配。如果词素和模式匹配。该词素被从流中删除 |
| `void next()`                             | 转到下一个词素                                               |
| `void reportError(const string& message)` | 报告错误                                                     |
| `void mustBe(Token t)`                    | 检查该标记是否与模式匹配。如果是这样，该令牌将从流中删除。否则就创建一个错误信息并尝试恢复 |
| `void recover(Token t)`                   | 错误恢复：跟着代码走，直到遇到这个词素或文件末端的词素       |
| `int findOrAddVariable(const string&)`    | 该函数通过`variables_`运行。如果它找到了正确的变量，就返回它的编号，否则就把它添加到数组中，增加lastVar并返回它。 |



| 字段名              | 描述                                         |
| ------------------- | -------------------------------------------- |
| `Scanner* scanner_`   | 构造函数的词法分析器                         |
| `CodeGen* codegen_`   | 指向虚拟机的指针                             |
| `ostream& output_`    | 输出流（在本例中我们使用cout）               |
| `bool error_`         | 错误标志。用于确定解析后的命令列表是否被输出 |
| `bool recovered_`     | *未使用*                                     |
| `VarTable variables_` | 程序中发现的变量数组                         |
| `int lastVar_`        | 最后记录的变量的编号                         |



## CodeGen

| 函数名                                                       | 描述                                       |
| ------------------------------------------------------------ | ------------------------------------------ |
| `void emit(Instruction instruction)`                         | 在程序末尾添加一条**无参数**的指令         |
| `void emit(Instruction instruction, int arg)`                | 在程序的末尾添加一条带有**单个参数**的指令 |
| `void emitAt(int address, Instruction instruction)`          | 向指定的地址写一条**无参数**的指令         |
| `void emitAt(int address, Instruction instruction, int arg)` | 向指定的地址写入一条带有**单个参数**的指令 |
| `int getCurrentAddress()`                                    | 获得紧随程序中最后一条指令的地址           |
| `int reserve()`                                              | 生成一条空白指令（NOP）并返回其地址        |
| `void flush()`                                               | 将一串的指令写入输出流中                   |

