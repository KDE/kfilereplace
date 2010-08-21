#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTET *.cpp *.h -o $podir/kfilereplace.pot
