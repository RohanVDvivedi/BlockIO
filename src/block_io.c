#include<blockio/block_io.h>

#include<stdlib.h>
#include<errno.h>

#define OPEN_WITH_READ_WRITE_PERMISSION 				(O_RDWR)
#define CREATE_OR_FAIL_CREATE_IF_FILE_EXISTS 			(O_CREAT | O_EXCL)
#define TEMP_FILE_CREATION								(O_TMPFILE | O_EXCL) // O_EXCL suggests that this fill will always remain a temp file (i.e. destroyed on closing)
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
	return fp->file_descriptor != -1;
}

int open_block_file(block_file* fp, const char* filename, int additional_flags)
{
	fp->block_size = 0;
	fp->file_descriptor = open(filename,
								OPEN_WITH_READ_WRITE_PERMISSION |
								DO_NOT_UPDATE_ACCESS_TIME_ON_READ_CALLS |
								(additional_flags & ALLOWED_ADDITIONAL_FLAGS));
	return fp->file_descriptor != -1;
}

int temp_block_file(block_file* fp, const char* directory, int additional_flags)
{
	fp->block_size = 0;
	fp->file_descriptor = open(directory,
								TEMP_FILE_CREATION |
								OPEN_WITH_READ_WRITE_PERMISSION |
								(additional_flags & ALLOWED_ADDITIONAL_FLAGS),
							S_IRUSR | S_IWUSR);
	return fp->file_descriptor != -1;
}

int read_blocks_from_block_file(block_file* fp, void* dest, off_t block_id, size_t block_count)
{
	off_t start_offset = block_id * get_block_size_for_block_file(fp);
	size_t bytes_count = block_count * get_block_size_for_block_file(fp);

	ssize_t bytes_read = 0;
	while(bytes_count > bytes_read)
	{
		ssize_t result = pread(fp->file_descriptor, dest + bytes_read, bytes_count - bytes_read, start_offset + bytes_read);
		if(result == -1 || result == 0)
			return 0;
		bytes_read += result;
	}

	return 1;
}

int write_blocks_to_block_file(block_file* fp, const void* src, off_t block_id, size_t block_count)
{
	off_t start_offset = block_id * get_block_size_for_block_file(fp);
	size_t bytes_count = block_count * get_block_size_for_block_file(fp);

	ssize_t bytes_written = 0;
	while(bytes_count > bytes_written)
	{
		ssize_t result = pwrite(fp->file_descriptor, src + bytes_written, bytes_count - bytes_written, start_offset + bytes_written);
		if(result == -1)
			return 0;
		bytes_written += result;
	}

	return 1;
}

int truncate_block_file(block_file* fp, size_t block_count)
{
	off_t bytes_count = block_count * get_block_size_for_block_file(fp);

	int result = ftruncate(fp->file_descriptor, bytes_count);
	if(result == -1)
		return 0;

	return 1;
}

int flush_all_writes_to_block_file(const block_file* fp)
{
	return fsync(fp->file_descriptor) != -1;
}

int close_block_file(const block_file* fp)
{
	return close(fp->file_descriptor) != -1;
}

//--------------------------------------------
// all of the linux specific code is below
//--------------------------------------------

#include<string.h>
#include<stdio.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<sys/sysmacros.h>
#include<linux/fs.h>

static int find_device(const block_file* fp, char device[256])
{
	struct stat fp_stat;
	if(-1 == fstat(fp->file_descriptor, &fp_stat))
		return -1;
	
	char major_minor[12];
	sprintf(major_minor, "%u:%u ", major(fp_stat.st_dev), minor(fp_stat.st_dev));

	FILE* f = fopen("/proc/self/mountinfo", "r");

	char sline[512];
	int device_found = -1;
	while(fgets(sline, 512, f))
	{
		char* suffix = strstr(sline, " - ");
		char* where = strstr(sline, major_minor);
		if(where && where <= suffix)
		{
			char* token = strtok(suffix, " -:");
			token = strtok(NULL, " -:");
			strcpy(device, token);
			device_found = 0;
			break;
		}
	}

	fclose(f);
	return device_found;
}

// must be non zero
#define DEFAULT_BLOCK_SIZE 512

size_t get_block_size_for_block_file(block_file* fp)
{
	if(fp->block_size)
		return fp->block_size;

	// TODO get fp->block_size for the device of this file
	char device_path[256];
	if(find_device(fp, device_path) == -1)
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
	fp->block_size = physical_block_size;

	END:;

	if(fp->block_size == 0)
		fp->block_size = DEFAULT_BLOCK_SIZE;

	return fp->block_size;
}

off_t get_total_size_for_block_file(block_file* fp)
{
	struct stat file_status;
	if(fstat(fp->file_descriptor, &file_status) < 0)
		return -1;

	return file_status.st_size;
}

int punch_hole_in_block_file(block_file* fp, off_t block_id, size_t block_count)
{
	off_t start_offset = block_id * get_block_size_for_block_file(fp);
	size_t bytes_count = block_count * get_block_size_for_block_file(fp);

	if(fallocate(fp->file_descriptor, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, start_offset, bytes_count) < 0)
		return 0;

	return 1;
}

int get_hole_in_block_file(block_file* fp, off_t* first_hole_start_block_id, off_t* first_hole_last_block_id, off_t block_id, size_t block_count)
{
	const off_t start_offset = block_id * get_block_size_for_block_file(fp);
	const size_t bytes_count = block_count * get_block_size_for_block_file(fp);
	const off_t last_offset = start_offset + bytes_count - 1;

	const off_t start_hole_offset = lseek(fp->file_descriptor, start_offset, SEEK_HOLE);
	if(start_hole_offset < 0) // failure
		return 0;
	if(!(start_offset <= start_hole_offset && start_hole_offset <= last_offset)) // hole is not within range
	{
		(*first_hole_start_block_id) = -1;
		return 1;
	}
	(*first_hole_start_block_id) = UINT_ALIGN_DOWN(start_hole_offset, get_block_size_for_block_file(fp)) / get_block_size_for_block_file(fp);

	const off_t end_hole_offset = lseek(fp->file_descriptor, start_hole_offset, SEEK_DATA);
	if(end_hole_offset < 0) // failure
		return 0;
	if(!(start_offset <= end_hole_offset && end_hole_offset <= last_offset)) // data is not within range
	{
		(*first_hole_last_block_id) = block_id + block_count - 1;
		return 1;
	}
	(*first_hole_last_block_id) = (UINT_ALIGN_UP(end_hole_offset, get_block_size_for_block_file(fp)) / get_block_size_for_block_file(fp)) - 1;

	return 1;
}