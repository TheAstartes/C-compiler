#!/bin/bash
assert() {
  expected="$1"
  input="$2"

 ./compiler  "$input" > tmp.s || exit
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 47 '5+6*7;' 
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 1 '7==5+2;'
assert 1 '7<=12;'
assert 5 'a=5; a;'
assert 1 '30>=(7*0)+15;'

echo OK
