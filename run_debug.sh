#!/bin/bash
valgrind --track-origins=yes --log-file=/tmp/val_log.txt --leak-check=full --show-leak-kinds=definite ./bin/nclyr $@
