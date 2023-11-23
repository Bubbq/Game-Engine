#!/bin/bash

set -xe

gcc test.c -o test -lraylib -lm -Werror -Wall -std=c11 -pedantic
