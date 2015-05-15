// Copyright 2015 CoreOS, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Finds the size of an ext4 filesystem located at the beginning of a given device.

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "ext4.h"

#define SUPER_BLOCK_OFFSET	0x400
#define SUPER_BLOCK_SIZE	0x400
#define EXT4_MAGIC		0xEF53

int read_ext4_super_block(const char *device_name, struct ext4_super_block *super_block) {
	int dev = -1;
	int retc = 1;

	if ((dev = open(device_name, O_RDONLY)) == -1) {
		goto err;
	}

	if (lseek(dev, SUPER_BLOCK_OFFSET, SEEK_SET) != SUPER_BLOCK_OFFSET) {
		goto err;
	}

	if (read(dev, super_block, SUPER_BLOCK_SIZE) != SUPER_BLOCK_SIZE) {
		goto err;
	}

	if (super_block->s_magic != EXT4_MAGIC) {
		goto err;
	}

	retc = 0;

err:
	if (dev != -1) {
		close(dev);
	}

	return retc;
}

uint64_t calc_ext4fs_size(struct ext4_super_block *super_block) {
	uint64_t total_blocks_count = 0;
	uint32_t blocks_count_lo = 0;
	uint32_t blocks_count_hi = 0;
	unsigned int block_size = 0;

	block_size = 1 << (10 + super_block->s_log_block_size);
	blocks_count_lo = super_block->s_blocks_count_lo;

	if (super_block->s_feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) {
		blocks_count_hi = super_block->s_blocks_count_hi;
	}

	total_blocks_count = (((uint64_t)blocks_count_hi) << 32) | blocks_count_lo;

	return total_blocks_count * block_size;
}

static void usage() {
	printf("Usage: ext4sz <device>\n");
}

int main(int argc, char *argv[]) {
	struct ext4_super_block *super_block = malloc(SUPER_BLOCK_SIZE);
	uint64_t fs_size = 0;

	if (argc != 2) {
		usage();
		goto out;
	}

	if (read_ext4_super_block(argv[1], super_block) != 0) {
		printf("%s\n", strerror (errno) );
		goto out;
	}

	fs_size = calc_ext4fs_size(super_block);
	printf("%" PRIu64 "\n", fs_size);

out:
	free(super_block);
	return 0;
}
