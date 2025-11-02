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
# Если строка находится в абзаце и заканчивается двумя пробелами, то
# после неё ставится <br>.
# Абзацы выделяются пустой строкой, в начале абзаца <p>, в конце </p>.
# Абзац заканчиваетя (а также не ставится <br>) перед:
# - списком
# - многострочным кодом
# - заголовком
# - разделителем
# *x* и $x$ заменяется на <i>x</i>
# **x** на <b>x</b>
# `x` на <code>x</code>, код внутри не форматируется


set tests {
    {
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
    "*Hello*, **World!**  
\$**I love XXX**\$.  
*Italic$.  "
    "<p><i>Hello</i>, <b>World!</b><br>
<i><b>I love XXX</b></i>.<br>
<i>Italic</i>.</p>"
    }
    {
    "`inline` `*inlin1\\``"
    "<p><code>inline</code> <code>*inlin1`</code></p>"
    }
    {
    "```c
/* code */  
```  text
Code.  "
    "<pre><code class=\"language-c\">/* code */</code></pre>
<p>Code.</p>"
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
    puts $fd [lindex $test 0]
    close $fd
    if {[catch {exec $bin "${fn}.md" > "${fn}.html"} error_msg]} {
        puts "test ${counter}: ${error_msg}"
        incr counter
        continue
    }
    set fd [open "${fn}.html" r]
    set output [ranl [read $fd]]
    set expected [ranl [lindex $test 1]]
    if {$output == $expected} {
        puts "\033\[92mTest ${counter} passed\033\[0m"
    } else {
        puts "\033\[91mTest ${counter} failed\033\[0m"
    }
    incr counter
}
