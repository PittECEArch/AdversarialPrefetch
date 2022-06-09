#!/bin/bash
gcc -mprefetchwt1 -pthread llc_s_latency.c -o llc_s
./llc_s > out_llc_s
python process_pr.py out_llc_s
