#!/bin/env tclsh
# 1md tests
# Copyright (c) 2025 etar125
# Licensed under ISC (see LICENSE)

set bin [lindex $argv 0]

proc ranl {text} {
    return [regsub -all "\\n" $text ""]
}

#            Экранирование
# Исходный символ    Экранированный символ
#         >                  &gt;
#         <                  &lt;
#         &                  &amp;

#                 Заголовки
# Должен генерироваться следующий код:
# <hX><a class="header-link" id="..." href="...">...</a></hX>
# X - уровень заголовка
# id - текст заголовка, где символы "? <>&" заменены на _
# href - ссылка на этот заголовок, "#id"
# В <a>...</a> экранированный текст заголовка

#                     Форматирование

# Пробелы и табы в начале строки НЕ РАЗРЕШЕНЫ,
# но есть случаи, когда таки можно ставить.

# Если строка находится в абзаце и заканчивается двумя пробелами, то
# после неё ставится <br>.

# Абзацы выделяются пустой строкой, в начале абзаца <p>, в конце </p>.
# Абзац заканчиваетя (а также не ставится <br>) перед:
# - списком
# - многострочным кодом
# - заголовком
# - разделителем
# * может перед чем-то другим, я забыл

# *x* и $x$ заменяется на <i>x</i>
# **x** на <b>x</b>

# `x` на <code>x</code>, код внутри не форматируется

# ```lang
# code
# ```
# заменяется на <pre><code class="language-lang">
# code
# </code></pre>, код внутри не форматируется, <br> не ставится.
# Если язык не указан, то class не пишется.
# Текст после последнего ``` игнорируется до новой строки.

# ```lang code``` - аналог inline кода, но указывается язык, как в полноценном блоке кода

#                     Списки
# Ненумерованные (можно смешивать):
# - 1
# - 2
# * 3
#     - 1
#     - 2
#       Hallo
# >>>
# <ul>
# <li>1</li>
# <li>2</li>
# <li>3
# <ul>
# <li>1</li>
# <li>2
# Hallo</li>
# </ul></li>
# </ul>

# Нумерованные (числа игнорируются):
# 0. 1
# 0. 2
#     0. 3
#     113. 88
#          ??
# >>>
# То же самое, что и ненумерованный, но ol вместо ul.

# Уровень списка определяется кол-вом табов в начале строки.
# 4 пробела считается за один таб.
# После обозначения списка ОБЯЗАТЕЛЬНО должен идти пробел.
# Т.е. `-el` недопустимо, надо `- el`.
# Текст элемента должен находится на одной вертикальной линии
# (начало после обозначения списка и пробела)
# 
#     - qweqwwee  
#       qweqewqqaaa aaa
#     - bye
# Таким образом можно вместить больше чем одну строку в текст элемента

# Если после ненумерованного списка стоит [ ], то добавляется пустой чекбокс,
# если [x], то чекбокс с галкой

#                     Разделители
# три `*` или `-` или `_` в начале строки, текст после них игнорируется
# Превращается в <hr>

#                         Ссылки
# [text](URL) заменяется на <a href="URL">text</a>
# ![alt](URL) заменяется на <img alt="alt" href="URL">

# implemented 0 false 1 true
# source md
# expected html

set tests {
    {
    1
    "# Заголовок 1-го уровня
## 2 > 3
### 9==3
#### Привет
##### Пока-пока!
###### a b c биба & боба"
    "<h1><a class=\"header-link\" id=\"Заголовок_1-го_уровня\" href=\"#Заголовок_1-го_уровня\">Заголовок 1-го уровня</a></h1>
<h2><a class=\"header-link\" id=\"2___3\" href=\"#2___3\">2 &gt; 3</a></h2>
<h3><a class=\"header-link\" id=\"9==3\" href=\"#9==3\">9==3</a></h3>
<h4><a class=\"header-link\" id=\"Привет\" href=\"#Привет\">Привет</a></h4>
<h5><a class=\"header-link\" id=\"Пока-пока!\" href=\"#Пока-пока!\">Пока-пока!</a></h5>
<h6><a class=\"header-link\" id=\"a_b_c_биба___боба\" href=\"#a_b_c_биба___боба\">a b c биба &amp; боба</a></h6>"
    }
    {
    1
    "*Hello*, **World!**  
\$**I love XXX**\$.  
*Italic$.  "
    "<p><i>Hello</i>, <b>World!</b><br>
<i><b>I love XXX</b></i>.<br>
<i>Italic</i>.</p>"
    }
    {
    1
    "`inline` `*inlin1\\``"
    "<p><code>inline</code> <code>*inlin1`</code></p>"
    }
    {
    1
    "```c
/* code */  
```  text
Code.  "
    "<pre><code class=\"language-c\">
/* code */  
</code></pre>
<p>Code.</p>"
    }
    {
    0
    "```c puts(\"Hello, World!\");``` prints `Hello, World!`.  "
    "<p><code class=\"language-c\">puts(\"Hello, World!\");</code> prints <code>Hello, World!</code>.</p>"
    }
    {
    1
    "*Из README от e125.*  
У вас могут возникнуть проблемы с:  
* управляющими символами ANSI(как решение, можно вместо системной CMD использовать ConEmu)
* русским языком
* компиляцией"
    "<p><i>Из README от e125.</i><br>
У вас могут возникнуть проблемы с:</p>
<ul>
<li>управляющими символами ANSI(как решение, можно вместо системной CMD использовать ConEmu)</li>
<li>русским языком</li>
<li>компиляцией</li>
</ul>"
    }
    {
    1
    "Сами \"процессоры\"(расширения) пишутся на языке [TinySS](https://github.com/etar125/tinyss).  "
    "<p>Сами \"процессоры\"(расширения) пишутся на языке <a href=\"https://github.com/etar125/tinyss\">TinySS</a>.</p>"
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
    puts $fd [lindex $test 1]
    close $fd
    if {[catch {exec $bin "${fn}.md" > "${fn}.html"} error_msg]} {
        puts "test ${counter}: ${error_msg}"
        incr counter
        continue
    }
    set fd [open "${fn}.html" r]
    set output [ranl [read $fd]]
    set expected [ranl [lindex $test 2]]
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
