/*------------------------------------------------------------------------------------------
 *
 * William Silberstein
 * wsilberstein@wustl.edu
 * csim.c - Takes in a Valgorand Trance File and simulates the instructions through
 * an arbitrary associative level. Prints out the amount of cache hits, misses, evictions,
 * dirty evictions, active dirty bits, and double hit references.
 *
 *----------------------------------------------------------------------------------------*/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define CACHELENGTH 64

// Struct to represent a Cache Line
struct CacheLine {
  int bytes;
  int set;
  int tag;
  unsigned v:1;
  unsigned d:1;
};

// Struct to represent Cache
struct Cache {
  int sets;
  int bps;
  int bSize;

  struct CacheLine** cache;
};

// Methods
void initArgs(int argc, char *argv[]);
void readFile(FILE* file);
void initCache();
int extractBits(int num, int bits, int pos);
void shiftCache(int set, int start);
int hasFreeSpace(int set);
void removeFromCache(int set, int index);
int isInCache(int set, struct CacheLine cacheLine);
void printCache();
void freeCache();

struct Cache Cache;
struct CacheLine CacheLine;

int hits = 0;
int misses = 0;
int evicts = 0;
int devicts = 0;
int dactive = 0;
int drefs = 0;

int main(int argc, char *argv[])
{
 
  initArgs(argc, argv);
  
  // Read in file from last argument
  FILE* file = fopen(argv[argc-1], "r");
  
  // Size of cache = (2^sets) * (size of pointers)
  int cacheSize = (1 << Cache.sets) * sizeof(int**);
  Cache.cache = malloc(cacheSize);

  initCache();

  readFile(file);
  
  // Close file
  fclose(file);

  // Dirty active bits = total dirty bytes - evicted dirty bytes
  dactive*= 1 << Cache.bSize;
  dactive-=devicts;
	 
  freeCache();
  printSummary(hits, misses, evicts, devicts, dactive, drefs);
    return 0;
}

// Shifts all of the cachelines in a set starting at position start
void shiftCache(int set, int start) {
  int i;
  for(i = start; i > 0; i--) {
    Cache.cache[set][i] = Cache.cache[set][i-1];
  }
}

// Prints the cache in a readable format
void printCache() {
  int sets = 1 << Cache.sets;
  for(int i = 0; i < sets; i++) {
    for(int j = 0; j < Cache.bps; j++) {
      printf(" %x ", Cache.cache[i][j].d);
    }
    printf("\n");
  }
}

// returns the bits from pos to pos + bits given a number
int extractBits(int num, int bits, int pos) {
   return (((1 << bits) - 1) & (num >> (pos - 1)));
}

//Initializes Cache - Allocates enough memory
void initCache() {
  int i;
  int cacheWidth = sizeof(struct CacheLine) * Cache.bps;
  int sets = 1 << Cache.sets;
 
  // Cacheline memory size = (Size of cacheline struct) * (blocks per set)
  for(i = 0; i < sets; i++) {
    Cache.cache[i] = malloc(cacheWidth);
  }
}

//Returns 1 if cache has a free spot available
int hasFreeSpace(int set) {
	int i;
	for(i = 0; i < Cache.bSize; i++) {
		if(Cache.cache[set][i].d == 0) {
			return 1;
		}
	}
	return 0;
}

//Returns the position in cache where the cache line is located - Returns -1 if not in cache
int isInCache(int set, struct CacheLine cacheLine) {
	int i;
	for(i = 0; i < Cache.bps; i++) {
		if(Cache.cache[set][i].v == 1 && Cache.cache[set][i].tag == cacheLine.tag) {
			return i;
		}
	}
	return -1;
}

// Frees the memory used by the Cache simulator
void freeCache() {
  int i;

  for(i = 0; i < Cache.sets; i++) {
    free(Cache.cache[i]);
  }
  free(Cache.cache);
}

// Reads instructions from File and simulates the instructions going though cache
void readFile(FILE* file) {
  char op;
  int address;
  int size;
  char comma;
  int index;
  int setIndex = Cache.bSize+1;
  int tagIndex = setIndex + Cache.sets;
  int tagSize = CACHELENGTH - Cache.sets - Cache.bSize;
  
  // Loop though all of the lines in the file
  while(fscanf(file, " %c %x %c %d", &op, &address, &comma, &size) == 4) {
	
	// Skip all instructions that have I
	if(op == 'I') {
		continue;
	}

	//Fill byte information from instruction - Initialize valid bit to 1 and dirty bit to 0
        CacheLine.bytes = extractBits(address, Cache.bSize, 1);
    	CacheLine.set = extractBits(address, Cache.sets, setIndex);
    	CacheLine.tag = extractBits(address, tagSize, tagIndex);
	CacheLine.v = 1;
	CacheLine.d = 0;		

	//Get index of line in cache if it exists
	index = isInCache(CacheLine.set, CacheLine);

	// Cache hit
	if(index != -1) {
		hits++;

		// Set dirty bit to dirty bit of found cache line
		CacheLine.d = Cache.cache[CacheLine.set][index].d;
		
		// Double reference if index of cache is 0
		if(index == 0) {
			drefs++;
		}
		
		// Determine hits, double references, and dirty active bits based off instruction type
		if(op == 'M') {
			hits++;
			drefs++;

			// if previous dirty bit was not set add 1 to the dirty bit counter 
			if(CacheLine.d != 1) {
				dactive++;
			}
			CacheLine.d = 1;	
		} else if(op == 'S') {
			
			// Increment active dirty bits if dirty bit is not currently high
			if(CacheLine.d != 1) {
                                dactive++;
                        }
			CacheLine.d = 1;
		}

		// Shift all cache lines before current cache line and set the first cacheline to the current cacheline
		shiftCache(CacheLine.set, index);
	        Cache.cache[CacheLine.set][0] = CacheLine;	
	
	// Cache Miss
	} else {
		misses++;

		// Determine hits, double references, and dirty active bits based off instruction type
	        if(op == 'M') {
                        hits++;
                        drefs++;
			CacheLine.d = 1; 
			dactive++;
                } else if(op == 'S') {
                        CacheLine.d = 1;
			dactive++;
                }
		
		// If last used cache line has dirty bit set then add to dirty eviction counter
		if(Cache.cache[CacheLine.set][Cache.bps-1].d == 1 && Cache.cache[CacheLine.set][Cache.bps-1].v == 1) {
			devicts += 1 << Cache.bSize;
			evicts++;
		
		// Normal Evicts if dirty bit not set
		} else if(Cache.cache[CacheLine.set][Cache.bps-1].v == 1){
			evicts++;		
		}
		
		// Shift all lines in cache by one and put cache line in first index
		shiftCache(CacheLine.set, Cache.bps-1);
                Cache.cache[CacheLine.set][0] = CacheLine;		
	}
   } 
}

// Reads in initial cache argument sizes from command line arguments
void initArgs(int argc, char* argv[]) {
  int opt; 
  while((opt = getopt(argc, argv, "s:E:b:tvh")) != -1) {
    switch(opt) {
      case 's':
        Cache.sets = atoi(optarg);
        break;
      case 'E':
        Cache.bps = atoi(optarg);
        break;
      case 'b':
         
        Cache.bSize = atoi(optarg);
	
        break;
      case '?':
        printf("unknown option: %c\n", opt);
        break;
      default :
        break;
    }
  }
}
