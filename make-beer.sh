#!/bin/sh

URL_FILE=${1:-beer-urls.txt}
COMIC_FILE=${2:-beer.html}

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
  <title>Ottawa Breweries</title>
</head>
<body bgcolor="#C0C0C0">

<h1>Ottawa Breweries</h1>

<p>This is not an exhaustive list. So sorry in advance if I missed somebody.

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

echo "<br clear=all>"
echo
echo "<h2>Honourable Mentions</h2>"
echo

N_URLS=`wc -l other-beer.txt | cut -d' ' -f1`
N_ROWS=$(($N_URLS + $N_COLS - 1))
N_ROWS=$((N_ROWS / N_COLS))

row=0
while read href; do
    [ $row -eq 0 ] && out_start

    echo "<li>$href"

    row=$(($row + 1))
    if [ $row -eq $N_ROWS ]; then
	out_end
	row=0
    fi
done <other-beer.txt

[ $row -eq 0 ] || out_end

# Trailer
cat <<EOF

<br clear=all>
<hr>
<small>Back to <a href="https://seanm.ca/">seanm.ca</a></small>
</body>
</html>
EOF
) > $COMIC_FILE
