#!/bin/sh

BROWSER=dillo

echo "compiling"
gcc -g 1md.c -o 1md
echo "md > html"
./1md "test.md" > "test.html" || {
    gdb ./1md
}

echo "creating basic html"

echo "<html><body>" > "basic.html"
cat "test.html" >> "basic.html"
echo "</body></html>" >> "basic.html"

echo "preview"

$BROWSER "./basic.html"
