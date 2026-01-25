# 1md

**1md** -- транслятор моего Markdown в промежуточный код.  
В рамках проекта также разрабатывается транслятор этого промежуточного кода в HTML -- **1md2ht**.

## Сборка

Нужны:
- GCC
- sh
- [e1l](https://github.com/etar125/e1l)

```sh
git clone https://github.com/etar125/1md
cd 1md
./build.sh
```

## Тесты

Прилагается скрипт на Tcl для тестирования 1md.  
Перед тестами соберите 1md.

```sh
./build.sh
./tests.tcl ../1md
```
