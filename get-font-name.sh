#!/bin/bash

fc-scan $1 | \
    grep fullname: | \
    sed -e 's/^[ \t]*fullname:\ \"//g' | \
    sed -e 's/\"(s)//g'

