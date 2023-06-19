#include<block_io.h>

#include<stdlib.h>

int create_and_open_block_file(block_file* fp, const char* filename, int additional_flags)
{
}

int open_block_file(block_file* fp, const char* filename, int additional_flags)
{
}

size_t get_block_size_for_block_file(const block_file* fp);

ssize_t read_blocks_from_block_file(const block_file* fp, void* dest, off_t block_id, size_t block_count)
{
	off_t start_offset = block_id * get_block_size_for_block_file(fp);
	size_t bytes_count = block_count * get_block_size_for_block_file(fp);

	ssize_t bytes_read = 0;
	while(bytes_count > bytes_read)
	{
		ssize_t result = pread(fp->file_descriptor, dest + bytes_read, bytes_count - bytes_read, start_offset + bytes_read);
		if(result < 0)
			return result;
		bytes_read += result;
	}

	return bytes_read;
}

ssize_t write_blocks_to_block_file(const block_file* fp, const void* src, off_t block_id, size_t block_count)
{
	off_t start_offset = block_id * get_block_size_for_block_file(fp);
	size_t bytes_count = block_count * get_block_size_for_block_file(fp);

	ssize_t bytes_written = 0;
	while(bytes_count > bytes_written)
	{
		ssize_t result = pwrite(fp->file_descriptor, src + bytes_written, bytes_count - bytes_written, start_offset + bytes_written);
		if(result < 0)
			return result;
		bytes_written += result;
	}

	return bytes_written;
}

int flush_all_writes_to_block_file(const block_file* fp)
{
	return fsync(fp->file_descriptor);
}

int close_block_file(const block_file* fp)
{
	return close(fp->file_descriptor);
}