#!/bin/bash

echo -e "N_TXs\tN_SUB_TXs\tPERCENT_DIST\tN_Aborts\tN_DPUs\tN_Threads\tN_Tasklets\tBatch_Size\tTotal_Time\tCPU_Time\tDPU_Time\tCopy_Time\tN_Rounds" > results.txt

BATCHES="500 1000 2500 5000"

# for b in $BATCHES; do
# 	make clean
# 	make BATCH_SIZE=$b

# 	for (( i = 0; i < 3; i++ )); do
# 		./bin/tpcc -t 30 -d 2000 >> results.txt
# 	done
# done

THREADS="1 2 4 8 16 24 31"

# for t in $THREADS; do
# 	make clean
# 	make BATCH_SIZE=100

# 	for (( i = 0; i < 3; i++ )); do
# 		./bin/tpcc -t $t -d 2048 >> results.txt
# 	done
# done

TASKLETS="1 2 3 4 5 6 7 8 9 10 11"

# for t in $TASKLETS; do
# 	make clean
# 	make NR_TASKLETS=$t

# 	for (( i = 0; i < 1; i++ )); do
# 		./bin/tpcc -t 2 -d 1 >> results.txt
# 	done
# done

DPUS="512 1024 1536 2048"

# for d in $DPUS; do
# 	make clean
# 	make NR_TASKLETS=1

# 	for (( i = 0; i < 3; i++ )); do
# 		./bin/tpcc -t 33 -d $d >> results.txt
# 	done
# done

DIST="0 15 45 75"

# for d in $DIST; do
# 	make clean
# 	make NR_TASKLETS=11 DIST_PAYMENT=$d

# 	for (( i = 0; i < 3; i++ )); do
# 	    ./bin/tpcc -t 33 -d 2048 >> results.txt
# 	done
# done

for d in $DPUS; do
    for p in $DIST; do
	make clean
	make NR_TASKLETS=2 DIST_PAYMENT=$p

		for (( i = 0; i < 2; i++ )); do
			./bin/tpcc -t 33 -d $d >> results.txt
		done
    done
done
