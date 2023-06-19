#include<stdio.h>

#include<block_io.h>

#define ADDITIONAL_FLAGS	0
#define FILENAME			"test.db"

int main()
{
	block_file bf;
	if(!create_and_open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS) && !open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS | O_TRUNC))
	{
		printf("failed to create block file\n");
		return -1;
	}

	printf("physical block size of the file %zu\n", get_block_size_for_block_file(&bf));

	close_block_file(&bf);

	return 0;
}