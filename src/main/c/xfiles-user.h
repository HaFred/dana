// See LICENSE for license details.

#ifndef SRC_MAIN_C_XFILES_USER_H_
#define SRC_MAIN_C_XFILES_USER_H_

#include "src/main/c/xfiles.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Temporarily include supervisor data structures to support proxy
// kernel systemcalls
#include "src/main/c/xfiles-supervisor.h"

//-------------------------------------- Userland

// Request information about the specific X-FILES/DANA configuration
// and return it in an XLen sized packed representation. Optionally,
// this will print the output directly to stdout.
xlen_t xfiles_dana_id(int flag_print);

// Initiate a new Transaction for a specific NNID. The X-Files Arbiter
// will then assign and return a TID necessary for other userland
// functions. The second parameter, "num_train_outputs", when set to
// zero indicates that this is a feedforward computation. If non-zero,
// this is a learning request.
tid_type new_write_request(nnid_type nnid, learning_type_t learning_type,
                           element_type num_train_outputs);

// Function to write a specific register inside of the X-Files
// Arbiter. The value is passed as a 32-bit unsigned, but only the
// LSBs will be used if the destination register has fewer than 32
// bits.
xlen_t write_register(tid_type tid, xfiles_reg reg, uint32_t value);

// Write the contents of an input array of some size to the X-Files
// Arbiter. After completing this function, the transaction is deemed
// valid and will start executing on Dana.
xlen_t write_data(tid_type tid,
                  element_type * input_data_array,
                  size_t count);

// Writes an input array to the X-Files Arbiter, but does not write
// the last array element. This, coupled with `write_data_last` can be
// used to start transactions nearly simultaneously.
xlen_t write_data_except_last(tid_type tid,
                              element_type * input_data_array,
                              size_t count);

// Writes the last element of an input array to the X-Files Arbiter.
// This will implicitly start a transaction.
xlen_t write_data_last(tid_type tid,
                              element_type * input_data_array,
                              size_t count);

// A special write data request used for incremental training. Here,
// an input and an expected output vector are passed. The
// configuration cache is updated inside the Configuration Cache.
xlen_t write_data_train_incremental(tid_type tid,
                                    element_type * input_data_array,
                                    element_type * output_data_array,
                                    size_t count_input,
                                    size_t count_output);

// Read all the output data for a specific transaction. This throws
// the CPU into a spinlock repeatedly checking the validity of the
// X-Files response.
uint64_t read_data_spinlock(tid_type tid,
                            element_type * output_data_array,
                            size_t count);

// Forcibly kill a running transaction
xlen_t kill_transaction(tid_type tid);

//-------------------------------------- Userland Proxy Kernel Syscalls

// Set the ASID to a new value
xlen_t pk_syscall_set_asid(asid_type asid);

// Set the ASID--NNID Table Poitner (ANTP)
xlen_t pk_syscall_set_antp(ant * os_antp);

// Do a debug echo using a systemcall
xlen_t pk_syscall_debug_echo(uint32_t data);

// Print a visual organization of a specific ASID--NNIT Table
void asid_nnid_table_info(ant * table);

// Constructor and destructor for the ASID--NNID Table data structure
void asid_nnid_table_create(ant ** table, size_t num_asids,
                            size_t nn_configurations_per_asid);
void asid_nnid_table_destroy(ant **);

// Constructor and destructor for the Queue structure
void construct_queue(queue **, int);
void destroy_queue(queue **);

// Append the NN configuration contained in a binary file to the ASID
// of the specified ASID--NNID table. **NOTE** This is currently
// unsupported with the proxy kernel as it doesn't supported file
// operation system calls.
int attach_nn_configuration(ant ** table, asid_type asid,
                            const char * nn_configuration_binary_file);

// Attach an NN configuration that points to NULL. This is useful for
// testing purposes to place a specific NN configuration in a specific
// location and generate traps that will cause us to fail fast on an
// invalid read.
int attach_garbage(ant ** table, asid_type asid);

// Append the NN configuration contained in an XLen-sized (64-bit or
// 32-bit depending on RISC-V architecture) array and of a certain
// size to the ASID of a specific ASID--NNID Table.
int attach_nn_configuration_array(ant ** table, uint16_t asid,
                                  const xlen_t * nn_configuration_array,
                                  size_t size);

// Bytes of data per beat of Tilelink L2 response. This is the value
// of tlDataBeats in uncore/src/main/scala/tilelink.scala.
#define TILELINK_BYTES_PER_BEAT 16
#define TILELINK_LG_BYTES_PER_BEAT 4
#define TILELINK_L2_BYTES 64
#define TILELINK_L2_ADDR_BITS 6
// Do an allocation that is aligned on an L2 cache line
int alloc_config_aligned(xlen_t ** raw, xlen_t ** aligned, size_t size);

#endif  // SRC_MAIN_C_XFILES_USER_H_
