#include<block_io.h>

#include<stdlib.h>

block_file* create_block_file(const char* filename, int additional_flags);

block_file* open_block_file(const char* filename, int additional_flags);

size_t get_block_size_for_block_file(const block_file* fp);

ssize_t read_blocks_from_block_file(const block_file* fp, void* dest, off_t block_id, size_t block_count);

ssize_t write_blocks_to_block_file(const block_file* fp, const void* src, off_t block_id, size_t block_count);

int flush_all_writes_to_block_file(const block_file* fp);

int close_block_file(block_file* fp)
{
	int error = close(fp->file_descriptor);
	free(fp);
	return error;
}