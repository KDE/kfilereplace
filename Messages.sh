#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/kfilereplace.pot
