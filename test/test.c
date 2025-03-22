#include<block_io.h>

#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>

#define ADDITIONAL_FLAGS	0 //| O_DIRECT | O_SYNC
#define FILENAME			"test.db"

#define FORMAT "This is block no %" PRId64 " - %" PRId64 "\n"

#define BLOCK_COUNT 4

#define BLOCK_COUNTS_TO_ACCESS 2048

//#define TEST_TEMP_FILE

int main()
{
	block_file bf;
	int write_first = 0;
#ifndef TEST_TEMP_FILE
	// write first only if you created
	if(!(write_first = create_and_open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS)) && !open_block_file(&bf, FILENAME, ADDITIONAL_FLAGS | O_TRUNC))
	{
		printf("failed to create/open block file\n");
		return -1;
	}
	printf("\n\nREGULAR FILE\n\n");
#else
	write_first = 1;
	if(!temp_block_file(&bf, ".", ADDITIONAL_FLAGS))
	{
		printf("failed to open temp block file\n");
		return -1;
	}
	printf("\n\nTEMPORARY FILE\n\n");
#endif

	printf("physical block size of the file %zu\n", get_block_size_for_block_file(&bf));

	size_t blocks_size = get_block_size_for_block_file(&bf) * BLOCK_COUNT;
	char* blocks = aligned_alloc(4096, blocks_size);

	if(write_first)
	{
		printf("\n\nWRITING\n\n");
		for(off_t i = 0; i < BLOCK_COUNTS_TO_ACCESS; i += BLOCK_COUNT)
		{
			off_t first = i;
			off_t last = i + BLOCK_COUNT - 1;
			snprintf(blocks, blocks_size, FORMAT, first, last);
			if(!write_blocks_to_block_file(&bf, blocks, first, BLOCK_COUNT))
				printf("error writing to blocks %"PRId64" - %" PRId64"\n", first, last);
		}

		if(!flush_all_writes_to_block_file(&bf))
			printf("error flushing written blocks\n");
	}

	printf("\n\nREADING\n\n");
	for(off_t i = 0; i < BLOCK_COUNTS_TO_ACCESS; i += BLOCK_COUNT)
	{
		off_t first = i;
		off_t last = i + BLOCK_COUNT - 1;
		if(!read_blocks_from_block_file(&bf, blocks, first, BLOCK_COUNT))
			printf("error reading from blocks %" PRId64 " - %" PRId64 "\n", first, last);
		printf("%" PRId64 " - %" PRId64 " block read -> %s\n", first, last, blocks);
	}

	free(blocks);

	printf("total_size = %"PRId64"\n", get_total_size_for_block_file(&bf));

	printf("making a hole in the file from block_id 17 to 37, both inclusive\n");
	if(!punch_hole_in_block_file(&bf, 17, (38-17)))
		printf("failed to punch a hole\n");

	printf("total_size = %"PRId64"\n", get_total_size_for_block_file(&bf));

	off_t holes_to_check[][2] = {
		{150, 200},
		{15, 20},
		{36, 42},
		{15, 42},
		{20, 30},
	};

	for(int i = 0; i < sizeof(holes_to_check)/sizeof(holes_to_check[0]); i++)
	{
		off_t hole_start = 0;
		off_t hole_last = 0;
		if(!get_hole_in_block_file(&bf, &hole_start, &hole_last, holes_to_check[i][0], holes_to_check[i][1] - holes_to_check[i][0] + 1))
			printf("failed to find a hole\n");
		printf("%ld-%ld first hole at %ld-%ld\n\n", holes_to_check[i][0], holes_to_check[i][1], hole_start, hole_last);
	}

	close_block_file(&bf);

	return 0;
}