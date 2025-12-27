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

# Если после ненумерованного списка стоит `[ ]`, то добавляется пустой чекбокс,
# `<input type="checkbox" disabled="">`
# если `[x]` (вместо `x` любой ASCII символ кроме пробела), то чекбокс с галкой
# `<input type="checkbox" checked="" disabled="">`

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
    {
    1
    "# Список Linux дистрибутивов
Это **просто список**!  
Тут нет характеристик, описаний и прочего, кроме ссылок.  

## Какие дистры
В данный список добавляются все живые дистрибутивы Linux, с условием, 
что у них есть сайт или репозиторий, который спокойно открывается (в Chromium, т.к. Firefox иногда выдаёт `PR_END_OF_FILE_ERROR`).  
Я мог за чем-то не уследить и случайно добавить мёртвый дистрибутив, 
или дистрибутив умер уже после добавления.  
Если вы заметили таковой или хотите добавить какой-то другой дистро, 
пишите в \[Issues\](https://github.com/etar125/linux_distros/issues).  

## Формат
Список представлен в виде файла `list.txt`.  
В нём строки формата `название=ссылка`.  

## Сколько?
Посмотреть текущее кол-во дистрибутивов можете с помощью
скрипта `sk`.  
Текущее кол-во -- 364.

---

Последний раз список обновлялся 6 июля 2025"
    "<h1><a class=\"header-link\" id=\"Список_Linux_дистрибутивов\" href=\"#Список_Linux_дистрибутивов\">Список Linux дистрибутивов</a></h1>
<p>Это <b>просто список</b>!<br>
Тут нет характеристик, описаний и прочего, кроме ссылок.</p>
<h2><a class=\"header-link\" id=\"Какие_дистры\" href=\"#Какие_дистры\">Какие дистры</a></h2>
<p>В данный список добавляются все живые дистрибутивы Linux, с условием, 
что у них есть сайт или репозиторий, который спокойно открывается (в Chromium, т.к. Firefox иногда выдаёт <code>PR_END_OF_FILE_ERROR</code>).<br>
Я мог за чем-то не уследить и случайно добавить мёртвый дистрибутив, 
или дистрибутив умер уже после добавления.<br>
Если вы заметили таковой или хотите добавить какой-то другой дистро, 
пишите в <a href=\"https://github.com/etar125/linux_distros/issues\">Issues</a>.
</p>
<h2><a class=\"header-link\" id=\"Формат\" href=\"#Формат\">Формат</a></h2>
<p>
Список представлен в виде файла <code>list.txt</code>.<br>
В нём строки формата <code>название=ссылка</code>.
</p>
<h2><a class=\"header-link\" id=\"Сколько_\" href=\"#Сколько_\">Сколько?</a></h2>
<p>
Посмотреть текущее кол-во дистрибутивов можете с помощью
скрипта <code>sk</code>.<br>
Текущее кол-во -- 364.
</p>
<hr>
<p>Последний раз список обновлялся 6 июля 2025</p>"
    }
    {
    1
    "# vpc125
Это виртуальная машина, которую пишете вы на языке \[TinySS\](https://github.com/etar125/tinyss).  

**Проект находится на очень ранней стадии разработки!**  

## TODO

* \[x\] Урезанная версия e125
* \[ \] ВМ
* \[ \] Функции для работы с памятью
* ...

## Сборка

```sh
git clone https://github.com/etar125/vpc125.git
cd vpc125
make updtss
make
```"
    "<h1><a class=\"header-link\" id=\"vpc125\" href=\"#vpc125\">vpc125</a></h1>
<p>Это виртуальная машина, которую пишете вы на языке <a href=\"https://github.com/etar125/tinyss\">TinySS</a>.</p>

<p><b>Проект находится на очень ранней стадии разработки!</b></p>

<h2><a class=\"header-link\" id=\"TODO\" href=\"#TODO\">TODO</a></h2>

<ul>
<li><input type=\"checkbox\" checked=\"\" disabled=\"\"> Урезанная версия e125</li>
<li><input type=\"checkbox\" disabled=\"\"> ВМ</li>
<li><input type=\"checkbox\" disabled=\"\"> Функции для работы с памятью</li>
<li>...</li>
</ul>

<h2><a class=\"header-link\" id=\"Сборка\" href=\"#Сборка\">Сборка</a></h2>

<pre><code class=\"language-sh\">
git clone https://github.com/etar125/vpc125.git
cd vpc125
make updtss
make
</code></pre>"
    }
    {
    1
    "# Что такое SimpleDraw?
**SimpleDraw** - библиотека для работы с графикой, предназначенная для упрощения этой работы. Основана на \[SFD\](https://github.com/etar125/SFD).  
На самом деле я просто захотел переписать либу и упростить себе разработку игрового движка.

# Начало работы с SimpleDraw

## Установка

Для начала \[скачайте библиотеку\](https://github.com/etar125/SimpleDraw/releases).  
Создайте проект Windows Forms:  
!\[image\](https://github.com/etar125/SimpleDraw/assets/116297277/6d6db8a8-6acc-491d-ad36-02768c595bb2)
!\[image\](https://github.com/etar125/SimpleDraw/assets/116297277/d35e818c-4493-47b0-8c9c-207ee8692b8c)  
Добавьте ссылку на библиотеку:  
!\[image\](https://github.com/etar125/SimpleDraw/assets/116297277/e7753bf9-6ce2-4706-b5b4-04f2724077a1)
!\[YQCzWFD9vM\](https://github.com/etar125/SimpleDraw/assets/116297277/d670c937-c518-4366-80eb-0d538cbd1b69)  
Присоедините её:
```cs
using SimpleDraw.Drawing;
using SimpleDraw.Objects.Default;
```"
    "<h1><a class=\"header-link\" id=\"Что_такое_SimpleDraw_\" href=\"#Что_такое_SimpleDraw_\">Что такое SimpleDraw?</a></h1>
<p><b>SimpleDraw</b> - библиотека для работы с графикой, предназначенная для упрощения этой работы. Основана на <a href=\"https://github.com/etar125/SFD\">SFD</a>.<br>
На самом деле я просто захотел переписать либу и упростить себе разработку игрового движка.</p>
<h1><a class=\"header-link\" id=\"Начало_работы_с_SimpleDraw\" href=\"#Начало_работы_с_SimpleDraw\">Начало работы с SimpleDraw</a></h1>
<h2><a class=\"header-link\" id=\"Установка\" href=\"#Установка\">Установка</a></h2>
<p>Для начала <a href=\"https://github.com/etar125/SimpleDraw/releases\">скачайте библиотеку</a>.<br>
Создайте проект Windows Forms:<br>
<img src=\"https://github.com/etar125/SimpleDraw/assets/116297277/6d6db8a8-6acc-491d-ad36-02768c595bb2\" alt=\"image\">
<img src=\"https://github.com/etar125/SimpleDraw/assets/116297277/d35e818c-4493-47b0-8c9c-207ee8692b8c\" alt=\"image\"><br>
Добавьте ссылку на библиотеку:<br>
<img src=\"https://github.com/etar125/SimpleDraw/assets/116297277/e7753bf9-6ce2-4706-b5b4-04f2724077a1\" alt=\"image\">
<img src=\"https://github.com/etar125/SimpleDraw/assets/116297277/d670c937-c518-4366-80eb-0d538cbd1b69\" alt=\"YQCzWFD9vM\"><br>
Присоедините её:</p>
<pre><code class=\"language-cs\">using SimpleDraw.Drawing;
using SimpleDraw.Objects.Default;</code></pre>"
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
    close $fd
    set fd [open "${fn}_o.html" w]
    puts -nonewline $fd $output
    close $fd
    if {$output == $expected} {
        puts "\033\[92mTest ${counter} passed\033\[0m"
    } else {
        if {[lindex $test 0]} {
            puts -nonewline "\033\[91m"
        } else { puts -nonewline "\033\[93m" }
        puts "Test ${counter} failed\033\[0m"
        set fd [open "${fn}_e.html" w]
        puts -nonewline $fd $expected
        close $fd
    }
    incr counter
}
