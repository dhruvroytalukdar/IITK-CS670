
# [CS670] Assignment 2 submission by Dhruv Roy Talukdar (251110027)

  

This repository implements a **Distributed Point Function (DPF)** scheme in **C++**.

A DPF allows two parties to share keys that, when evaluated independently and XORed together, yield a function that is **non-zero at exactly one target index** and **zero elsewhere** — without revealing the target index or value to either party.

  
---

  

## 📘 Overview

  

This implementation demonstrates the **generation**, **evaluation**, and **verification** of DPF keys using a **pseudorandom generator (PRG)**.

  

The code consists of the following main components:

- Random number generation using 64-bit arithmetic.

- Seed expansion through a length-doubling PRG.

- Layer-wise correction word computation for two DPF trees.

- Full evaluation of each DPF key.

- Correctness checking by verifying the XOR of outputs.

  

---

  

## 🧩 Core Idea

  

Given a **domain of size `N`**, a **target index `t`**, and a **target value `v`**,
the DPF construction produces **two keys**  `k₀` and `k₁`.

Each key can be evaluated locally to produce a pseudorandom vector, and the XOR of both parties' outputs yields the desired point function.

---

  

## ⚙️ How It Works

  

### 1. Random Number Generation

```cpp

inline int64_t random_uint(int seed=0)

```

  

Generates a random 64-bit integer in the range [1, PRIME) using the Mersenne Twister engine.

If a seed is given, it re-seeds the generator for deterministic randomness.

  

### 2. Length-Doubling PRG

```cpp

vector<int64_t> length_doubling_PRG(int64_t seed)

```

  

Expands one 64-bit seed into two new pseudorandom 64-bit seeds.

This is used to simulate a pseudorandom generator (PRG) for expanding seeds in a binary tree.

  

### 3. Seed Expansion

```cpp

void expand_layer(vector<int64_t> &seeds, vector<uint8_t> &flags)

```

  

Each seed and flag in the current layer is expanded into two new seeds and flags for the next layer using the PRG.

  

### 4. DPF Key Generation

```cpp

vector<dpf_key_type> generateDPF(int64_t domain_size, int64_t target_index, int64_t target_value)

```

- Generates two DPF keys for the given parameters.

- Ensures that the domain size is rounded up to the next power of 2.

- Builds two identical PRG trees (one per party) with randomized seeds and flags.

- At each level, computes correction words (cw) and flag correction words (fcw0, fcw1) to ensure both parties’ trees remain consistent except at the target path.

- The final correction word (final_cw) encodes the target_value.

  

### 5. Key Evaluation

```cpp

vector<int64_t> EvalFull(int64_t domain_size, dpf_key_type dpf_key, int64_t target_index)

```

Fully expands a DPF key into its corresponding output vector over the given domain by re-applying the seed expansion and correction process.

  

### 6. Correctness Check

```cpp

bool check_dpf_correctness(int64_t domain_size, int64_t target_index, int64_t target_value, vector<dpf_key_type> dpf_keys)

```

  

Verifies the DPF property:

  

XOR of both evaluations equals target_value at target_index

XOR equals 0 at all other indices

  

## User defined data structures

  

**struct dpf_key_type**: Holds the components of a DPF key:

  

```cpp

struct dpf_key_type {

int64_t root; // Root seed of the DPF tree

uint8_t flag; // Root flag (either 0 or 1)

vector<int64_t> cw; // Correction words for each layer

vector<uint8_t> fcw0; // Flag correction for non-target nodes

vector<uint8_t> fcw1; // Flag correction for target-path nodes

int64_t final_cw; // Final correction word for the last layer

};

```

  

## Program Flow

Compile using
```bash
g++ gen_queries.cpp -o dpf

```


Run using
```bash

./dpf.exe <domain_size> <num_dpfs>

```

  

- **domain_size**: Length of the final vector.

- **num_dpfs**: The number of DPF instances to generate and test.

  

For each DPF:

  

- Randomly select a target_index and target_value.

- Generate two DPF keys using generateDPF.

- Evaluate both keys using EvalFull.

- Check correctness using check_dpf_correctness.

  
  

Print the result as:

  

```yaml

DPF 1: PASSED

DPF 2: PASSED

...

```

  

🧮 Example Execution

Compilation

```bash

g++ gen_queries.cpp -o dpf

```

Example Run

```bash

./dpf 8 3

```

Example Output

```yaml
DPF 1: PASSED
DPF 2: PASSED
DPF 3: PASSED
```