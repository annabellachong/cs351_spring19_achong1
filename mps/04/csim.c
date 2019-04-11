#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

typedef unsigned long long memadd_t;

typedef struct {
	int valid;
 	memadd_t tag;
	int timestamp;
} line;

typedef struct {
	line *lines;
} set;

typedef struct {
	set *sets;
} cache_t;

void read_input(int argc, char **argv);
int evict_index(int idx);
int empty_index(int idx);
void sim_cmd(memadd_t addr);
cache_t build_cache(int S, int E, int B);
void clear_cache(cache_t cache_sim);
void usage(void);

//define parameters

int verbose=0;
int s=0, E=0, b=0,S=0, B=0;
int hit_count=0, miss_count=0, eviction_count=0;

// Tracefile
char *t;

int time;    // cycle time


// the actual cache simulation
cache_t cache_sim;

int main(int argc, char **argv) {
	read_input(argc, argv);   // read command
  cache_sim = build_cache(S, E, B); 	// build the cache
	hit_count = 0;
	miss_count = 0;
	eviction_count = 0;
	time = 0;

	FILE *tracefile;      // read trace file
	char command;
	memadd_t addr;
	int size;
	tracefile = fopen(t, "r");   //open file
	if(tracefile != NULL) {
		while(fscanf(tracefile, " %c %llx,%d", &command, &addr, &size) == 3) {
			switch(command) {        // determine input
				case 'I':
					break;
				case 'L':
					if(verbose) printf("%c %llx,%d ", command, addr, size);
					sim_cmd(addr);
					if (verbose) printf("\n");
					break;
				case 'S':
					if(verbose) printf("%c %llx,%d ", command, addr, size);
					sim_cmd(addr);
					if (verbose) printf("\n");
					break;
				case 'M':
					if(verbose) printf("%c %llx,%d ", command, addr, size);
					sim_cmd(addr);

					hit_count += 1; if(verbose) printf("hit\n"); //increase hit count
					break;
			}

		}
	}

	printSummary(hit_count, miss_count, eviction_count); // PRINT SUMMARY

	return 0;
}

void sim_cmd(memadd_t addr) {
	memadd_t mem_tag = addr >> (s + b);
	int idx = (unsigned int) ((addr << (64 - (s + b))) >> (64 - s)); //64 bit long
	int full = 1;
	set set_curr = cache_sim.sets[idx];
	int curr_hits = hit_count;

	int li;        	//check address
	for(li = 0; li < E; li++) {
		line line_curr = set_curr.lines[li];
		if(line_curr.valid && line_curr.tag == mem_tag) {
			set_curr.lines[li].timestamp = ++time;
			hit_count++;
			if(verbose) printf("hit ");
		} else if(!(line_curr.valid)) {
			full = 0;
		}
	}

	if(hit_count > curr_hits)    	//check if hit
		return;
	else {
		miss_count ++;              // no hit, increment miss count
		if(verbose) printf("miss ");
	}


	if(full == 1) {

		eviction_count++;
		if(verbose) printf("eviction ");
		int lru_idx = evict_index(idx);
		set_curr.lines[lru_idx].tag = mem_tag;
		set_curr.lines[lru_idx].timestamp = ++time;
	} else {

		int emt_idx = empty_index(idx);
		set_curr.lines[emt_idx].tag = mem_tag;
		set_curr.lines[emt_idx].valid = 1;
		set_curr.lines[emt_idx].timestamp = ++time;
	}
}

int evict_index(int idx) {
	int min_idx = 0;
	int li;
	for(li = 0; li < E; li++) {

		if(cache_sim.sets[idx].lines[li].timestamp < cache_sim.sets[idx].lines[min_idx].timestamp) {
			min_idx = li;
		}
	}
	return min_idx;
}

int empty_index(int idx) {
	int li;
	for(li = 0; li < E; li++) {
		if(!cache_sim.sets[idx].lines[li].valid)
			return li;
	}
	return -1;
}

cache_t build_cache(int S, int E, int B) {
	cache_t cache_sim;
	set cache_set;
	line cache_line;

	int si, li;
	cache_sim.sets = (set *) calloc(S, sizeof(set));  //allocate sets

	for(si = 0; si < S; si++) {
		cache_set.lines = (line *) calloc(E, sizeof(line)); 		// allocate lines for each set
		for(li = 0; li < E; li++) {
			cache_line.tag = 0;
			cache_line.valid = 0;
			cache_line.timestamp = 0;
			cache_set.lines[li] = cache_line;
		}
		cache_sim.sets[si] = cache_set;
	}
	return cache_sim;
}

void clear_cache(cache_t cache_sim) {
	int si;
	for(si = 0; s < S; si++) {
		free(cache_sim.sets[si].lines); 		// free lines in set
	}
	free(cache_sim.sets);                 	// free all sets in simulated cache
}

void read_input(int argc, char **argv) {
	char flag;
	while((flag = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		switch(flag) {
			case 's':
				s = atoi(optarg);
				S = (int)pow(2, s);    //size of sets (2^s)
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				B = (int)pow(2, b);    //size of block (2^b)
				break;
			case 't':
				t = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
				usage();
				exit(0);
			default:
				usage();
				exit(1);
		}
	}
}

void usage(void){
    printf("Usage:  ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h         Prints help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("   linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("   linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
    exit(0);
}
