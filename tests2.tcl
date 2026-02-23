#!/bin/tclsh
# 1md2ht tests
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
    "+h 1 Привет
+h 2 Пока
+h 3 Что???
+h 7 Да нет.
"
    "<h1><a class=\"header-link\" id=\"Привет\" href=\"#Привет\">Привет</a></h1>
<h2><a class=\"header-link\" id=\"Пока\" href=\"#Пока\">Пока</a></h2>
<h3><a class=\"header-link\" id=\"Что???\" href=\"#Что???\">Что???</a></h3>
<h6><a class=\"header-link\" id=\"Да_нет-\" href=\"#Да_нет-\">Да нет.</a></h6>
"
    }
    {
    1
    "+p
+text \"Как 
+bold
+text дела
-bold
+text ?\" <вот так вот>
+eol
-p
"
    "<p>
&quot;Как <b>дела</b>?&quot; &lt;вот так вот&gt;
</p>
"
    }
    {
    1
    "+p
+text Список 1:
+eol
-p
+list
+el
+text Привет 1
+eol
-el
+el
+text Привет 2
+eol
+list
+el
+text Подсписок
+eol
+text Да...
+eol
-el
-list
+text Нет?
+eol
-el
+el
+text Пока
+eol
+newline
+text Пока!
+eol
-el
-list
+p
+text Список 2:
+eol
-p
+nlist
+el
+text Привет 1
+eol
-el
+el
+text Привет 2
+eol
+nlist
+el
+text Подсписок
+eol
+text Да...
+eol
-el
-nlist
+text Нет?
+eol
-el
+el
+text Пока
+eol
+newline
+text Пока!
+eol
-el
-nlist
+p
+bold
+text Да
-bold
+text ...
+eol
-p
"
    "<p>
Список 1:
</p>
<ul>
<li>
Привет 1
</li>
<li>
Привет 2
<ul>
<li>
Подсписок
Да...
</li>
</ul>
Нет?
</li>
<li>
Пока
<br>
Пока!
</li>
</ul>
<p>
Список 2:
</p>
<ol>
<li>
Привет 1
</li>
<li>
Привет 2
<ol>
<li>
Подсписок
Да...
</li>
</ol>
Нет?
</li>
<li>
Пока
<br>
Пока!
</li>
</ol>
<p>
<b>Да</b>...
</p>
"
    }
    {
    1
    "+p
+text Code:
+eol
-p
+mlcode sh
+text ./1md \"my file.md\" > \"my file.1md\"
+eol
+text   # yes
+eol
-mlcode
+p
+text Ques
+ilcode
+text t
-ilcode
+text ion.
+eol
-p
"
    "<p>
Code:
</p>
<pre><code class=\"language-sh\">
./1md &quot;my file.md&quot; &gt; &quot;my file.1md&quot;
  # yes
</code></pre>
<p>
Ques<code>t</code>ion.
</p>
"
    }
    {
    1
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
    "<p>
Hallo. <a href=\"https://etar125.ru\">etar<b>125</b>.ru</a>
<br>
Desktop: <img src=\"https://media.etar125.ru/mgr1.jpg\" alt=\"Desk\">
</p>
"
    }
    {
    1
    "+list
+el
+uncheckedbox
+text Unchecked
+eol
-el
+el
+checkedbox
+text Checked
+eol
-el
-list
"
    "<ul>
<li>
<input type=\"checkbox\" disabled=\"\">
Unchecked
</li>
<li>
<input type=\"checkbox\" disabled=\"\" checked=\"\">
Checked
</li>
</ul>
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
    set fd [open "${fn}.1md" w]
    puts -nonewline $fd [lindex $test 1]
    close $fd
    if {[catch {exec $bin "${fn}.1md" > "${fn}.html"} error_msg]} {
        puts "test ${counter}: ${error_msg}"
        incr counter
        continue
    }
    set fd [open "${fn}.html" r]
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
