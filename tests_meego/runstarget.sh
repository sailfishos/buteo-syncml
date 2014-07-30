#!/bin/sh

if [ $# -lt 1 ]; then
    echo "You need to pass test executable as an argument!"
    exit 1
fi

cd "$(dirname "${0}")"

PATH=".:${PATH}"
exec "${@}" -maxwarnings 0
