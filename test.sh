#!/bin/sh
./build.sh
./1md test.md > test.1md
./1md2ht test.1md > test.html
cat > fulltest.html << EOF
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8"/>
<title>1md test</title>
</head>
<body>

EOF
cat test.html >> fulltest.html
cat >> fulltest.html << EOF

</body>
</html>

EOF
dillo fulltest.html
