#ifndef BLOCK_IO_H
#define BLOCK_IO_H

typedef struct block_file block_file;
struct block_file
{
	// file descriptor to the file
	int file_descriptor;

	// size of the physical block on the disk of this file
	// this will be populated only after the first call to get_block_size_for_block_file
	size_t block_size;
};

// return values of <= 0 are errors

// return value of NULL is a failure to create block file
block_file* create_block_file(const char* filename, int additional_flags);

// return value of NULL is a failure to open block file
block_file* open_block_file(const char* filename, int additional_flags);

// size of each physical block on the disk of this file
size_t get_block_size_for_block_file(const block_file* fp);

// return of a negative value implies an error, and return of a positive value is the number of bytes read
// ensure that dest atleast has block_count & block_size number of bytes allocated
ssize_t read_blocks_from_block_file(const block_file* fp, void* dest, uint64_t block_id, uint64_t block_count);

// return of a negative value implies an error, and return of a positive value is the number of bytes written
// ensure that src atleast has block_count & block_size number of bytes allocated
ssize_t write_blocks_to_block_file(const block_file* fp, const void* src, uint64_t block_id, uint64_t block_count);

// return of a negative value implies an error, return value of 0 is a success
int flush_all_writes_to_block_file(const block_file* fp);

// return value
int close_block_file(const block_file* fp);

#endif