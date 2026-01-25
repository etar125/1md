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

Прилагаются скрипты на Tcl для тестирования 1md и 1md2ht.  
Перед тестами соберите проект.

```sh
./build.sh
./tests.tcl ../1md
./tests2.tcl ../1md2ht
```
