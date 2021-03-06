#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<cassert>
#include<string>
#include<vector>

#include <stdint.h>
#include "bam.h"
#include "sam.h"
#include "sam_utils.h"

#include "utils.h"
#include "my_assert.h"

using namespace std;

int nThreads;
string cqname;
samfile_t *in, *out;
bam1_t *b;
vector<bam1_t*> arr;
bool unaligned;

void output() {
	if (unaligned || arr.size() == 0) return;
	bool isPaired = bam_is_paired(arr[0]);
	if ((isPaired && arr.size() != 2) || (!isPaired && arr.size() != 1)) return;
	for (size_t i = 0; i < arr.size(); ++i) samwrite(out, arr[i]);
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		printf("Usage: rsem-get-unique number_of_threads unsorted_transcript_bam_input bam_output\n");
		exit(-1);
	}

        nThreads = atoi(argv[1]);
	in = samopen(argv[2], "r", NULL);
	assert(in != 0);
	out = samopen(argv[3], "wb", in->header);
	assert(out != 0);
        if (nThreads > 1) general_assert(samthreads(out, nThreads, 256) == 0, "Fail to create threads for writing the BAM file!");

	HIT_INT_TYPE cnt = 0;

	cqname = "";
	arr.clear();
	b = bam_init1();
	unaligned = false;

	while (samread(in, b) >= 0) {
		if (cqname != bam_get_qname(b)) {
			output();
			cqname = bam_get_qname(b);
			for (size_t i = 0; i < arr.size(); ++i) bam_destroy1(arr[i]);
			arr.clear();
			unaligned = false;
		}

		unaligned = unaligned || bam_is_unmapped(b);
		arr.push_back(bam_dup1(b));

		++cnt;
		if (cnt % 1000000 == 0) { printf("."); fflush(stdout); }
	}

	if (cnt >= 1000000) printf("\n");

	output();

	bam_destroy1(b);
	samclose(in);
	samclose(out);

	printf("done!\n");

	return 0;
}
