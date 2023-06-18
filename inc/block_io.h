#ifndef BLOCK_IO_H
#define BLOCK_IO_H

#include<stddef.h>
#include<unistd.h>

typedef struct block_file block_file;
struct block_file
{
	// file descriptor to the file
	int file_descriptor;

	// size of the physical block on the disk of this file
	// this will be populated only after the first call to get_block_size_for_block_file
	// it's initial value will be 0 -> an invalid value
	size_t block_size;
};

// file sizes and file offsets on any posix system can only be stored/worked with in a off_t (a signed integral) type integer

// maximum value of off_t data type
#define OFF_T_MAX  ((off_t) (~( ((off_t)(1)) << ((sizeof(off_t) * CHAR_BIT) - 1) )))

// maximum blocks counts physically possible to be addressable by an off_t variable
#define MAX_BLOCK_COUNT(BLOCK_SIZE)	(OFF_T_MAX / BLOCK_SIZE)

// this is the maximum possible value of block_id that can be held by a off_t
#define MAX_BLOCK_ID(BLOCK_SIZE)	(MAX_BLOCK_COUNT(BLOCK_SIZE) - 1)

// this is the maximum block counts that you can submit for an IO, in the read_blocks and write_blocks functions calls
#define MAX_BLOCK_COUNT_FOR_IO(BLOCK_SIZE) (SSIZE_MAX / BLOCK_SIZE)

// you must flush (flush_all_writes_to_disk) intermittently to ensure that the data has reached the non-volatile memory of the disk
// for surety of the data being flushed after every write call, you must open/create the file with O_DIRECT | O_SYNC flags

// return values of <= 0 are errors

// return value of NULL is a failure to create block file
// additional flags are the once in addition to the default flags
block_file* create_block_file(const char* filename, int additional_flags);

// return value of NULL is a failure to open block file
// additional flags are the once in addition to the default flags
block_file* open_block_file(const char* filename, int additional_flags);

// size of each physical block on the disk of this file
size_t get_block_size_for_block_file(const block_file* fp);

// return of a negative value implies an error, and return of a positive value is the number of bytes read
// ensure that dest atleast has block_count & block_size number of bytes allocated
ssize_t read_blocks_from_block_file(const block_file* fp, void* dest, off_t block_id, size_t block_count);

// return of a negative value implies an error, and return of a positive value is the number of bytes written
// ensure that src atleast has block_count & block_size number of bytes allocated
// a write call may or may not flush the contents to non-volatile disk
ssize_t write_blocks_to_block_file(const block_file* fp, const void* src, off_t block_id, size_t block_count);

// return of a negative value implies an error, return value of 0 is a success
int flush_all_writes_to_block_file(const block_file* fp);

// return value
int close_block_file(const block_file* fp);

#endif