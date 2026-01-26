#!/bin/sh

URL_FILE=${1:-comic-urls.txt}
COMIC_FILE=${2:-comics.html}

[ -f $URL_FILE ] || { echo "$URL_FILE not found"; exit 1; }

N_COLS=4
N_URLS=`wc -l $URL_FILE | cut -d' ' -f1`
N_ROWS=$(($N_URLS + $N_COLS - 1))
N_ROWS=$((N_ROWS / N_COLS))

out_start() {
    echo
    echo "<div class=\"comics\">"
    echo "<ul>"
}

out_end() {
    echo "</ul>"
    echo "</div>"
}

(
# Header
cat <<EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
  <meta http-equiv="Content-type" content="text/html;charset=UTF-8">
  <link rel="stylesheet" type="text/css" href="seanm.css">
  <title>The good stuff: comics</title>
</head>
<body bgcolor="#C0C0C0">

<h1>Comics</h1>

<hr>
EOF

row=0
while read href; do
    [ $row -eq 0 ] && out_start

    echo "<li>$href"

    row=$(($row + 1))
    if [ $row -eq $N_ROWS ]; then
	out_end
	row=0
    fi
done <$URL_FILE

[ $row -eq 0 ] || out_end

# Trailer
cat <<EOF

<br clear=all>
<hr>
  <small>Back to <a href="http://www.seanm.ca/">seanm.ca</a></small>
</body>
</html>
EOF
) > $COMIC_FILE
