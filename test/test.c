#include<stdio.h>
#include<stdlib.h>

#include<block_io.h>

#define ADDITIONAL_FLAGS	0
#define FILENAME			"test.db"

#define FORMAT "This is block no %d\n"

int main()
{
	block_file bf;
	if(!create_and_open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS) && !open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS | O_TRUNC))
	{
		printf("failed to create block file\n");
		return -1;
	}

	printf("physical block size of the file %zu\n", get_block_size_for_block_file(&bf));

	char* block = malloc(get_block_size_for_block_file(&bf));

	for(int i = 0; i < 32; i++)
	{
		snprintf(block, get_block_size_for_block_file(&bf), FORMAT, i);
		if(!write_blocks_to_block_file(&bf, block, i, 1))
			printf("error writing to block %d\n", i);
	}

	if(!flush_all_writes_to_block_file(&bf))
		printf("error flushing written blocks\n");

	for(int i = 0; i < 32; i++)
	{
		if(!read_blocks_from_block_file(&bf, block, i, 1))
			printf("error reading from block %d\n", i);
		printf("%d block read -> %s\n", i, block);
	}

	free(block);

	close_block_file(&bf);

	return 0;
}