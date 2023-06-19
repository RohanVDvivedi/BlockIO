#include<block_io.h>

#include<stdlib.h>

#define OPEN_WITH_READ_WRITE_PERMISSION 				(O_RDWR)
#define CREATE_OR_FAIL_CREATE_IF_FILE_EXISTS 			(O_CREAT | O_EXCL)
#define DO_NOT_UPDATE_ACCESS_TIME_ON_READ_CALLS 		(O_NOATIME)

int create_and_open_block_file(block_file* fp, const char* filename, int additional_flags)
{
	fp->block_size = 0;
	fp->file_descriptor = open(filename,
								OPEN_WITH_READ_WRITE_PERMISSION |
								CREATE_OR_FAIL_CREATE_IF_FILE_EXISTS |
								DO_NOT_UPDATE_ACCESS_TIME_ON_READ_CALLS |
								(additional_flags & ALLOWED_ADDITIONAL_FLAGS),
							S_IRUSR | S_IWUSR);
	return fp->file_descriptor;
}

int open_block_file(block_file* fp, const char* filename, int additional_flags)
{
	fp->block_size = 0;
	fp->file_descriptor = open(filename,
								OPEN_WITH_READ_WRITE_PERMISSION |
								DO_NOT_UPDATE_ACCESS_TIME_ON_READ_CALLS |
								(additional_flags & ALLOWED_ADDITIONAL_FLAGS));
	return fp->file_descriptor;
}

// must be non zero
#define DEFAULT_BLOCK_SIZE 512

size_t get_block_size_for_block_file(const block_file* fp)
{
	if(fp->block_size)
		return fp->block_size;

	// TODO get fp->block_size for the device of this file
	char device_path[256];
	if(find_device(dbfile_p, device_path) == -1)
	{
		printf("Device does not seem to be found for file provided\n");
		goto END;
	}

	printf("Given file is on device %s\n", device_path);
	int device_fd = open(device_path, O_RDONLY);
	if(device_fd == -1)
	{
		printf("Could not open device %s\n", device_path);
		goto END;
	}

	int physical_block_size = -1;
	int err_return = ioctl(device_fd, BLKSSZGET, &physical_block_size);
	close(device_fd);
	if(err_return == -1)
	{
		printf("Getting physical block size returns %d, errnum %d\n", err_return, errno);
		goto END;
	}

	printf("getting physical block size as %d\n", physical_block_size);
	dbfile_p->block_size = physical_block_size;

	END:;

	if(fp->block_size == 0)
		fp->block_size = DEFAULT_BLOCK_SIZE;

	return fp->block_size;
}

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