#include<stdio.h>
#include<stdlib.h>

#include<block_io.h>

#define ADDITIONAL_FLAGS	0
#define FILENAME			"test.db"

#define FORMAT "This is block no %d - %d\n"

#define BLOCK_COUNT 4

int main()
{
	block_file bf;
	if(!create_and_open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS) && !open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS | O_TRUNC))
	{
		printf("failed to create block file\n");
		return -1;
	}

	printf("physical block size of the file %zu\n", get_block_size_for_block_file(&bf));

	size_t blocks_size = get_block_size_for_block_file(&bf) * BLOCK_COUNT;
	char* blocks = aligned_alloc(4096, blocks_size);

	for(int i = 0; i < 32; i += BLOCK_COUNT)
	{
		int first = i;
		int last = i + BLOCK_COUNT - 1;
		snprintf(blocks, blocks_size, FORMAT, first, last);
		if(!write_blocks_to_block_file(&bf, blocks, first, BLOCK_COUNT))
			printf("error writing to blocks %d - %d\n", first, last);
	}

	if(!flush_all_writes_to_block_file(&bf))
		printf("error flushing written blocks\n");

	for(int i = 0; i < 32; i += BLOCK_COUNT)
	{
		int first = i;
		int last = i + BLOCK_COUNT - 1;
		if(!read_blocks_from_block_file(&bf, blocks, first, BLOCK_COUNT))
			printf("error reading from blocks %d - %d\n", first, last);
		printf("%d - %d block read -> %s\n", first, last, blocks);
	}

	free(blocks);

	close_block_file(&bf);

	return 0;
}