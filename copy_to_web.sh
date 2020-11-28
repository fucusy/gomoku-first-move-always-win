#!/usr/bin/env bash
cp ./gomoku/script/web/gomoku.html ../fucusy.github.io/docs/
sed -i '' -e 's/\"\/\"/"http:\/\/81.70.152.141:8080"/g' ../fucusy.github.io/docs/gomoku.html

