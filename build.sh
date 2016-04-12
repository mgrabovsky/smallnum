#!/bin/bash
(cd _build ; cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./sn_test)
# valgrind --trace-children=yes --leak-check=full ./sn_test

