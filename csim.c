#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <assert.h>
#include "cachelab.h"

//Selmane Tabet - 210004050

typedef unsigned long long int addr64; //Memory address format, 64-bit integer.

typedef struct cacheLine {
 char valid;
 addr64 tag;
 unsigned LRU;
} cacheLine;

unsigned counter = 0; //global vars, to be incremented through the execution of the program.

unsigned traceCount = 0;

unsigned hits = 0;
unsigned misses = 0;
unsigned evicts = 0;

void trial(addr64 address, cacheLine** lines, unsigned E, unsigned s, unsigned b, unsigned t){
//extract params from address.
//if hit, update hit and LRU.
//if miss, update miss and fill in data in an empty line, otherwise, it's eviction.
//if evict, replace the line with the lowest LRU value with new tag and LRU counter.
	assert(t == 64 - s - b);
	
	addr64 setID = (address << t) >> (t + b);
	addr64 tagID = (address >> (s + b)) & ~(0x8000000000000000 >> (b + s - 1)); //-1 to adjust for the set MSB
	printf("Set ID: %llu \n", setID);
	printf("Tag ID: %llu \n", tagID);
	printf("Address: %llu \n", address);

	printf("Hit check ongoing...\n");
	for(unsigned k = 0; k < E; k++){ //Linear search for hit.
		counter++;
		if ((lines[setID][k].valid == 1) && (lines[setID][k].tag == tagID)){ //Hit
			lines[setID][k].LRU = counter;
			hits++;
			printf("Confirmed Hit\n");
			return;
		}
	}

	unsigned m = 0;
	printf("Miss check ongoing...\n");
	while (m < E){ //scan for empty lines;
		counter++;
		if (lines[setID][m].valid == 0){ //confirmed miss, fill any empty line.
			assert(lines[setID][m].tag == 0xFFFFFFFFFFFFFFFF);
			assert(lines[setID][m].LRU == 0);
			misses++;
			printf("Confirmed Miss\n");
			lines[setID][m].valid = 1;
			lines[setID][m].tag = tagID;
			lines[setID][m].LRU = counter;
			printf("Hole filled.\n");
			return;
		}
		m++;
	}
	misses++; //Confirmed miss on full cache set, proceed to eviction.
	assert(m == E); //at this point, we are sure that all lines are occupied.
	//Therefore, an eviction will take place.
	unsigned lowest = counter; //store current time state for the first LRU comparison
	unsigned lowestIndex;
	printf("Eviction ongoing...\n");
	for (unsigned n = 0; n < E; n++){
		assert(lines[setID][n].valid == 1);
		assert(~(lines[setID][n].tag)); //defaults at all set, zero means false.
		counter++;
		if (lines[setID][n].LRU < lowest){ //scanning for lowest LRU
			lowest = lines[setID][n].LRU; //lowest LRU so far
			lowestIndex = n; //Way number where lowest LRU was found
		}
	}
	lines[setID][lowestIndex].tag = tagID; //replace lowest LRU line with a new line.
	lines[setID][lowestIndex].LRU = counter; //update LRU field
	evicts++;
	printf("Eviction complete.\n");
	return;
}

int main(int argc, char **argv)
{
	unsigned s; //number of bits for sets.
	unsigned b; //number of bits for blocks.
	unsigned E; //number of ways in the cache.
	unsigned S; //Total number of sets to be stored here.
	//int B;
	unsigned t; //number of bits for tags.
	//int tmp;

	//bool verbose = false; //Verbose mode defaults at Not verbose.

	FILE *traceRead; //for pointing and scanning file content
	char *traceFile; //for holding name of file
	addr64 address; //for parsing instruction address from trace.
	char instructionLine; //for parsing instruction type from trace
	unsigned size; //for parsing size field from trace
	char c; //for parsing args

    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1) //parsing args
	{
        switch(c)
		{
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            traceFile = optarg;
            break;
        case 'v':
            //verbose = true;
            break;
        case 'h':
            exit(0);
        default:
            exit(1);
        }
        printf("Parsed.\n");
    }

    if (s == 0 || E == 0 || b == 0 || traceFile == NULL) //Case of invalid inputs.
	{
        printf("%s: Missing required command line argument\n", argv[0]);
        
        exit(1);
    }

 	S = pow(2.0, s); //Set number = 2^s
	//B = pow(2.0, b); //Block size = 2^b
	t = 64 - s - b; //Number of tag bits.
	printf("S and t are calculated.\n");

//Single pointer (**line) pointing to an array of pointers (*line) that point to cacheLine arrays (lines).

	cacheLine **lines = (cacheLine**)malloc(sizeof(cacheLine*) * S);
	for (unsigned i = 0; i < S; i++) {
	    lines[i] = (cacheLine*)malloc(sizeof(struct cacheLine) * E);
		for(unsigned j = 0; j < E; j++){
			lines[i][j].valid = 0; //i is the set ID, j is the way number.
			lines[i][j].tag = 0xFFFFFFFFFFFFFFFF; //to avoid wrong tag detection.
			lines[i][j].LRU = 0;
		}
	}
	printf("Cache created and initialized.\n");

	traceRead  = fopen(traceFile, "r"); //Take the file name and open it in read mode.
	if (traceRead != NULL) { //Now use scan to parse traces
		while (fscanf(traceRead, " %c %llx,%d", &instructionLine, &address, &size) == 3) { //Format of traces, one can use regex too
			switch(instructionLine) {
				case 'I':
					break;
				case 'L':
					printf("Attempting L with address %llx and size %d\n", address, size);
					trial(address, lines, E, s, b, t);
					traceCount++;
					break;
				case 'S':
					printf("Attempting S with address %llx and size %d\n", address, size);
					trial(address, lines, E, s, b, t);
					traceCount++;
					break;
				case 'M':
					printf("Attempting M-Load with address %llx and size %d\n", address, size);
					trial(address, lines, E, s, b, t);
					printf("Attempting M-Store with address %llx and size %d\n", address, size);
					trial(address, lines, E, s, b, t);
					traceCount++;
					break;
				default:
					break;
			}
		}
	}
	fclose(traceRead);
    printSummary(hits, misses, evicts);
    printf("Traces done: %d \n", traceCount);
    for (int k = 0; k < S; k++) free(lines[k]);
	free(lines);
	printf("Memory freed from Heap.\n");
	printf("Done.\n");
    return 0;
    
}
