#!/bin/sh

kill -9 $(pidof m11_server)
kill -9 $(pidof theme1.exe)

echo "All killed !!!"
