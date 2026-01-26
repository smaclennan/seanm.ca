#!/bin/sh -e

# We need the no-header to keep it to two pages
enscript --no-header -presume.ps resume.txt
ps2pdf resume.ps resume.pdf
rm resume.ps
