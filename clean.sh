#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

rm $SCRIPT_DIR/arduino
ant -f $SCRIPT_DIR/build/build.xml clean
