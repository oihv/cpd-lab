#!/bin/bash

just build

# Or test all
for i in {1..5}; do
    echo -n "test/nfa$i.txt: "
    ./main "test/nfa$i.txt" "test/dfa$i.txt"
done
