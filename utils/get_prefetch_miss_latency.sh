#!/bin/bash
gcc -mprefetchwt1 -pthread pre_miss_latency.c -o pre_miss
./pre_miss > out_pre_miss
python process_pp.py out_pre_miss
