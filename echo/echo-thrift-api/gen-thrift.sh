#!/usr/bin/env bash

# Get the absolute path where this script is.
basedir="$( cd "$( dirname "$0" )" && pwd )"

echo ""
echo ">>> Compiling thrift interfaces..."

rm -rf $basedir/gen-java
mkdir -p $basedir/gen-java
thrift --gen java -o $basedir $basedir/echo_server.thrift

echo ">>> Done."
echo ""
