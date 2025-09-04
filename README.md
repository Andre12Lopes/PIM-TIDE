# PIM-TIDE

PIM-TIDE is a transaction execution platform designed specifically for PIM architectures. PIM-TIDE provides a lightweight software-based coordination mechanism that enables transactions to span multiple DPUs while ensuring consistency, atomicity, and isolation.

* Transactions do not require data from remote DPUs
* Transactions are generated in the DPUs
* Transactions can be executed in 2 modes:
    * Deterministic
    	* Used for transactions that span across multiple DPUs
    * Non-Deterministic
    	* Used for single partition transaction (transactions that are executed on a single DPU)
