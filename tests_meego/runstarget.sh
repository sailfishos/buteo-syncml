#!/bin/sh

if [ $# -ne 1 ]; then
    echo "You need to pass test executable as an argument!"
    exit 1
fi

echo "running ${1}..."

FILE=${1##*/}

cd /opt/tests/buteo-syncml

exec ${1} -maxwarnings 0
