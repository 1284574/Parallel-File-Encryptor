# Parallel File Encryptor/Decryptor

## Table of Contents

* [Overview](#overview)
* [Features](#features)
* [Architecture](#architecture)
* [Concurrency Model](#concurrency-model)
* [Directory Structure](#directory-structure)
* [Components](#components)
* [Build Instructions](#build-instructions)
* [Usage](#usage)
* [Example](#example)
* [Environment Configuration](#environment-configuration)
* [Encryption Algorithm](#encryption-algorithm)
* [Error Handling](#error-handling)
* [Future Enhancements](#future-enhancements)

## Overview

This C++ project implements a **parallel**, **concurrently synchronized** file encryption and decryption utility. It uses a combination of POSIX shared memory, named semaphores, and `fork()`-based child processes to safely distribute work across multiple processes.

## Features

* Byte-level Caesar cipher encryption and decryption.
* **Interprocess** producer–consumer queue with a fixed capacity (1000 tasks).
* Synchronization via named POSIX semaphores (`sem_open`).
* Shared memory buffer (`shm_open` + `mmap`) for task descriptors.
* Custom `IO` RAII wrapper for safe file handling.
* Task abstraction for serializing and deserializing work units.
* Environment-based key configuration via `.env` file.

## Architecture

1. **Main Driver (`encrypt_decrypt`)**

   * Parses command-line arguments or test inputs.
   * Calls `ProcessManagement::submitToQueue()` to enqueue each task.
   * Each call to `submitToQueue` spawns a child process immediately.

2. **ProcessManagement**

   * On construction:

     * Creates two named semaphores:

       * **emptySlotsSemaphore**: Tracks available buffer slots (initialized to 1000).
       * **itemsSemaphore**: Tracks number of enqueued tasks (initialized to 0).
     * Allocates a shared memory segment for a circular task queue.
   * `submitToQueue(std::unique_ptr<Task>)`:

     1. `sem_wait(emptySlotsSemaphore)` to reserve a slot.
     2. Locks an in-process `std::mutex` to guard pointer arithmetic.
     3. Writes the serialized task string into the shared-memory ring.
     4. Updates `rear` index and atomically increments `size`.
     5. Unlocks and `sem_post(itemsSemaphore)`.
     6. `fork()`s a worker process:

        * Child calls `executeTask()` and exits.
        * Parent returns to main flow.

3. **executeTask()** (run in child):

   1. `sem_wait(itemsSemaphore)` to wait for available work.
   2. Locks the `std::mutex` and reads from the `front` of the ring.
   3. Updates `front` index and atomically decrements `size`.
   4. Unlocks and `sem_post(emptySlotsSemaphore)` to free a slot.
   5. Calls `executeCryption()` on the deserialized task.

## Concurrency Model

* **Bounded Buffer**: Fixed-size array of 1000 task slots in shared memory.
* **Semaphores**:

  * Producers block on **emptySlotsSemaphore** when the buffer is full.
  * Consumers (child processes) block on **itemsSemaphore** when the buffer is empty.
* **Mutex**:

  * A `std::mutex queueLock` guards index updates and buffer writes/reads within each process. **Note:** `std::mutex` is thread-level; for truly process-shared locking, a `pthread_mutex_t` with `PTHREAD_PROCESS_SHARED` would be required.
* **Process Pool**: Each submission forks a new process that immediately consumes one task. This design maximizes parallel CPU utilization but may spawn many short-lived processes.

## Directory Structure

```text
Parallel-File-Encryptor/
├── makedirs.py             # Utility script to create directory structure automatically
├── Makefile

├── Makefile
├── main.cpp
├── src/
│   └── app/
│       ├── encryptDecrypt/
│       │   ├── Cryption.hpp
│       │   ├── Cryption.cpp
│       │   └── CryptionMain.cpp
│       ├── fileHandling/
│       │   ├── IO.hpp
│       │   ├── IO.cpp
│       │   ├── ReadEnv.hpp
│       │   └── ReadEnv.cpp
│       └── processes/
│           ├── Task.hpp
│           ├── ProcessManagement.hpp
│           └── ProcessManagement.cpp
└── .env
```

## Components

* **IO**: RAII class opening/closing `std::fstream`.
* **ReadEnv**: Loads the Caesar cipher key from `.env`.
* **Task**: Encapsulates file path, stream, and `Action` (ENCRYPT/DECRYPT); supports `toString()`/`fromString()`.
* **ProcessManagement**: Manages the interprocess queue, semaphores, and shared memory; spawns worker processes.
* **Cryption**: Implements in-place, byte-by-byte Caesar cipher on an open file.

## Build Instructions

1. Ensure you have a C++17 compiler and POSIX support.
2. Populate `.env` in project root with a single integer (the cipher shift):

   ```
   42
   ```
3. Run:

   ```bash
   make
   ```

   This produces two executables:

   * `encrypt_decrypt` — parallel driver.
   * `cryption`        — standalone file processor.

## Usage

```bash
# Parallel encrypt:
./encrypt_decrypt <file> ENCRYPT

# Parallel decrypt:
./encrypt_decrypt <file> DECRYPT

# Standalone mode:
./cryption <file> ENCRYPT
./cryption <file> DECRYPT
```

## Example 1

```bash
# Encrypt test.txt in parallel:
./encrypt_decrypt test.txt ENCRYPT

# Decrypt the same file:
./encrypt_decrypt test.txt DECRYPT
```

## Example 2
```bash
# Make test directory of random test files
python3 makeDirs.py
# Encrypt ./test in parallel:
./encrypt_decrypt test ENCRYPT
# Decrypt the same folder:
./encrypt_decrypt test DECRYPT
```


## Environment Configuration

* `.env` must contain exactly one integer (e.g., `7`).
* The integer defines the Caesar cipher shift amount.

## Encryption Algorithm

A simple additive cipher per byte:

```cpp
unsigned char u = static_cast<unsigned char>(ch);
// encrypt:
unsigned char out = (u + key) % 256;
// decrypt:
unsigned char out = (u + 256 - key) % 256;
```

## Error Handling

* **Semaphore failures** and **shared memory errors** print to stderr and exit.
* **File open errors** throw `std::runtime_error` in `Task::fromString()`.
* **Invalid `.env`** contents cause `std::stoi()` exceptions.

## Future Enhancements

* **True shared mutex** (`pthread_mutex_t` with `PTHREAD_PROCESS_SHARED`).
* **Thread pool** instead of per-call `fork()`.
* **Configurable buffer size** and dynamic worker pool.
* **Stronger encryption** (AES) alongside the Caesar cipher.
* **Progress reporting** and structured logging.
