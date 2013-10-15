#!/bin/bash

echo -e "HTTP/1.1 200 OK\r
Server: uHTTPServer\r
Content-Type: text/html\r
Accept-Ranges: bytes\r
Cache-Control: no-cache\r
Connection: close\r

<html>
<body>
Parameters:
<ul>
"

Count=0
for arg
do
Count=$(( $Count + 1 ))
echo "<li> #$Count = '$arg' </li>"
done

echo "
</ul>
<pre>"
printenv
echo "
</pre>
</body>
</html>
";
