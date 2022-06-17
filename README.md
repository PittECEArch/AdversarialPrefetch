# Adversarial Prefetch: New Cross-Core Cache Side-Channel Attacks
## Description

This repo contains tools to perform Prefetch+Prefetch and Prefetch+Reload.  For
details of these two attacks, please refer to our [Oakland'22
paper](https://yananguo.com/files/oakland22.pdf).


## Timing Characterization

To run these two attacks, we must determine the timing threshold to distinguish
different cache events. This includes: `prefetch_hit_local_L1` and
`prefetch_hit_remote_L1` for Prefetch+Prefetch, as well as `load_hit_LLC` and
`load_hit_remote_L1` for Prefetch+Reload.

These thresholds depend on the microarchitecture details of the processor. We
provide scripts to find the correct thresholds.

First, to obtain the threshold for Prefetch+Prefetch, do:
```
cd utils
bash get_pre_miss_latency.sh
```
This script tests the latency of the prefetchw when the target data is in the
local/remote L1 cahe 50000 times (for each) and prints out the results.

When running this script, you should see something similar with the following image:
![Example Profiling Result](/figures/prefetch_latency.PNG)

In the above image, an appropriate timing threshold for Prefetch+Prefetch should be 105-115 cycles.

Similarly, to obtain to get the threshold for Prefetch+Reload, do:
```
cd utils
bash get_llc_s_latency.sh
```

Then, pick an appropriate timing theshold based on results.

Note that, the timing threshold for the attack really depends on the physcical
CPU cores the attack is running on. Thus, we should characterize the timing on
the cores used in the attacks. This can be configured in
utils/pre_miss_latency.c (for Prefetch+Prefetch) and utils/llc_s_latency.c (for
Prefetch+Reload).

In utils/pre_miss_latency.c,
```
//Test the local and remote prefetch latency between core 0 and core 1.
#define trojan_core 0;
#define spy_core 1;
```

In utils/llc_s_latency.c,
```
//Test the LLC and remote L1 load latency between core 0 and core 2. 
#define trojan_core 0;
#define spy_core 2;
#define victim_core 1; // Does not really matter.
```


## Setup and Run the Covert Channels

### Step 1: Parameter configuration

We provide PoC for the covert channels. Before building and running the channel, there are a few parameters that need to be configured accordingly. Please see covert_channels/libs/util.h.

### Step 2: Build the covert channels

To build the covert channels, do

```sh
cd $AdversarialPrefetch_HOME/covert_channels
mkdir build
cmake ..
make
```
Then you can find the sender and receiver executables in 'covert_channels/build/bin' directory.

### Step 3: Run the covert chanenls

To run a covert channel, first start the sender process

```
cd $AdversarialPrefetch_HOME/covert_channels/build/bin
./sender 

```
This sender process sends "0" and "1" alternatively.
Then start one of the receiver processes
```
cd $AdversarialPrefetch_HOME/covert_channels/build/bin
./receiver_pre_pre > raw_bits_received
```
or 

```
cd $AdversarialPrefetch_HOME/covert_channels/build/bin
./receiver_pre_relo > raw_bits_received
```

The file "raw_bits_received" will contain the bits the receiver detected.


## Side channel example
TBA.

## Contact
For any questions, please open an issue in this repo. Or contact Yanan via
email, which is contained in the paper, linked [above](#Description).
