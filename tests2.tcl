#!/bin/tclsh
# 1md2ht tests
# Copyright (c) 2026 etar125
# Licensed under ISC (see LICENSE)

set bin [lindex $argv 0]

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
    if {[catch {exec {*}$bin "${fn}.1md" > "${fn}.html"} error_msg]} {
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
