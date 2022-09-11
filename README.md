# Simple S-Pacal Compiler 概要

## Build
* 建立build文件夹，`cd`进入，通过
```bash
cmake ..
make 
```
这时会根据makefile文件自动构建、链接程序。

## Features

- *简单的数据类型*: `boolean`, `char`, `integer`, `real`,`string`
- *控制流*: if-else, case-of, while-do, repeat-until, and for loops
- *定义*: `const`, `type`, `var`, and routine sections
- *Routine* (`function` and `procedure`) definition and invocation
  - 包括一些系统函数: `read(ln)`, `write(ln)`, `abs`, `sqrt`, `chr`, `ord`, `pred`, `succ`
- *Operators*: `+` `-` `*` `/` `div` `mod` `and` `or` `xor` `not` and comparison operators
- *类型检查*, 从 `integer` 到 `real` 隐式类型转换

## Usage

```
USAGE: spc <option> <source.pas>
OPTION:
  -emit-llvm    Emit LLVM IR (.ll)
  -S            Emit assembly code (.s)
  -c            Emit object code (.o)
  -O            (Optional) 可选的做一些优化
  -o des        name output file as des
  -ast          生成ast树
```

- For LLVM IR files, run `lli output.ll` to directly execute them.
- For assembly and object files, run `cc output.{s,o}` to generate executables.

## Dependencies

- CMake 3.7+
- Flex 2.5+
- Bison 3.0+
- LLVM 9 ( LLVM 10 Not recommended becuause of some bug in cmake scripts of it)

## TODO

- [ ] 支持array record复杂的数据类型
- [ ] 完成支持函数嵌套(Optional)
- [ ] generate detailed error messages for Flex/Bison (Optional)
- [ ] do error recovery in Bison and codegen (Optional)