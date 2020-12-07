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

assert 0 '{ return 0; }'
assert 42 '{ return 42; }'
assert 21 '{ return 5+20-4; }'
assert 47 '{ return 5+6*7; }' 
assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'
assert 1 '{ return 7==5+2; }'
assert 1 '{ return 7<=12; }'
assert 5 '{ a=5; return a; }'
assert 1 '{ return 30>=(7*0)+15; }'
assert 2 '{ if (0) return 3; return 2; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'


echo OK
