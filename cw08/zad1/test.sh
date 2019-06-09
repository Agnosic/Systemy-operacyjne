#!/bin/bash

test_thread(){
    echo "   "
    echo "$2 threads"
    echo "$3 size of filter"
    ./filtr "$2" "$1" images/marcie.ascii.pgm "$3" out.pgm
}

test_method(){
    echo "  "
    echo "$1 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    test_thread "$1" 1 "$2"
    test_thread "$1" 2 "$2"
    test_thread "$1" 4 "$2"
    test_thread "$1" 8 "$2"
}

test_method "block" filters/4
test_method "interleaved" filters/4
test_method "block" filters/30
test_method "interleaved" filters/30
test_method "block" filters/65
test_method "interleaved" filters/65