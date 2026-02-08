#!/bin/tclsh
# 1md tests
# Copyright (c) 2026 etar125
# Licensed under ISC (see LICENSE)

set bin [lindex $argv 0]
if {$bin == ""} {
    puts "укажи путь к бинарнику (учти, что запускает в tests.tmp)"
    exit 1
}

set tests {
    {
    1
    "# Заголовок 1-го уровня
## 2 > 3
### 9==3
#### Привет
##### Пока-пока!
####### a b c биба & боба"
    "+h 1 Заголовок 1-го уровня
+h 2 2 > 3
+h 3 9==3
+h 4 Привет
+h 5 Пока-пока!
+h 6 a b c биба & боба
"
    }
    {
    1
    "Привет, **мир**!  
$Курсив*, шекели**.**
"
    "+p
+text Привет, 
+bold
+text мир
-bold
+text !
+eol
+newline
+italic
+text Курсив
-italic
+text , шекели
+bold
+text .
-bold
+eol
-p
"
    }
    {
    1
    "Список:
- Elem 1
- Multi
line
 elem 2
  - Под
   список
   **1**  
  Ага
- Уже нет
.

Конец!"
    "+p
+text Список:
+eol
-p
+list
+el
+text Elem 1
+eol
-el
+el
+text Multi
+eol
+text line
+eol
+text elem 2
+eol
+list
+el
+text Под
+eol
+text список
+eol
+bold
+text 1
-bold
+eol
+newline
+text Ага
+eol
-el
-list
-el
+el
+text Уже нет
+eol
+text .
+eol
-el
-list
+p
+text Конец!
+eol
-p
"
    }
    {
    1
    "1. Один
2. Два
3. Три
12. 12, но так-то 4  
 00.всё ещё не подсписок
  0. А это уже подсписок
 Ну да.
1. Пока (это 5-ый)

Пока!
"
    "+nlist
+el
+text Один
+eol
-el
+el
+text Два
+eol
-el
+el
+text Три
+eol
-el
+el
+text 12, но так-то 4
+eol
+newline
+text 00.всё ещё не подсписок
+eol
+nlist
+el
+text А это уже подсписок
+eol
-el
-nlist
+text Ну да.
+eol
-el
+el
+text Пока (это 5-ый)
+eol
-el
-nlist
+p
+text Пока!
+eol
-p
"
    }
    {
    1
    "`Test`!  
Yeaah!
So, here is my code:  
```c
#include <stdio.h>
int main(void) {
    puts(\"Hello, World!\");
    return 0;
}
```
What do `y*u` think?
"
    "+p
+ilcode
+text Test
-ilcode
+text !
+eol
+newline
+text Yeaah!
+eol
+text So, here is my code:
+eol
-p
+mlcode c
+text #include <stdio.h>
+eol
+text int main(void) {
+eol
+text     puts(\"Hello, World!\");
+eol
+text     return 0;
+eol
+text }
+eol
-mlcode
+p
+text What do 
+ilcode
+text y*u
-ilcode
+text  think?
+eol
-p
"
    }
    {
    1
    "Hallo. [etar**125**.ru](https://etar125.ru)  
Desktop: ![Desk](https://media.etar125.ru/mgr1.jpg)
"
    "+p
+text Hallo. 
+link https://etar125.ru
+text etar
+bold
+text 125
-bold
+text .ru
-link
+eol
+newline
+text Desktop: 
+opt alt Desk
+img https://media.etar125.ru/mgr1.jpg
+eol
-p
"
    }
    {
    1
    "Manager: !![mgr2](https://media.etar125.ru/mgr2.jpg)
"
    "+p
+text Manager: 
+link https://media.etar125.ru/mgr2.jpg
+opt alt mgr2
+img https://media.etar125.ru/mgr2.jpg
-link
+eol
-p
"
    }
}

if {[file exists "tests.tmp"]} {
    file delete -force "tests.tmp"
}
file mkdir "tests.tmp"
cd "tests.tmp"

set counter 1
foreach test $tests {
    set fn "test${counter}"
    set fd [open "${fn}.md" w]
    puts -nonewline $fd [lindex $test 1]
    close $fd
    if {[catch {exec $bin "${fn}.md" > "${fn}.1md"} error_msg]} {
        puts "test ${counter}: ${error_msg}"
        incr counter
        continue
    }
    set fd [open "${fn}.1md" r]
    set output [read $fd]
    set expected [lindex $test 2]
    close $fd
    if {$output == $expected} {
        puts "\033\[92mTest ${counter} passed\033\[0m"
    } else {
        if {[lindex $test 0]} {
            puts -nonewline "\033\[91m"
        } else { puts -nonewline "\033\[93m" }
        puts "Test ${counter} failed\033\[0m"
    }
    incr counter
}
