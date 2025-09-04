# PIM TPCC

This version aims to create a TM system that works across multiple DPUs of the UPMEM hardware
* Transactions do not require data from remote DPUs
* Transactions are generated in the DPUs
* Transactions can be executed in 2 modes:
    * Deterministic
    	* Used for transactions that span across multiple DPUs
    * Non-Deterministic
    	* Used for single partition transaction (transactions that are executed on a single DPU)
