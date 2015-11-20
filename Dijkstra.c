#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Dijkstra.h"

#define INFINITY	2000000
#define MIN(x, y) ((x)<(y)?(x):(y))

int Dijkstra(int ** costs, int num, int src, int dest) {
	int i, j;
	int next = src, next_tmp;
	int min_cost;
	int * flag = malloc(num * sizeof(int));
	int * result = malloc(num * sizeof(int));
	int * route = malloc(num * sizeof(int));
	memset(flag, 0, num * sizeof(int));
	/* init */
	result[src] = 0;
	flag[src] = 1;
	min_cost = INFINITY;
	route[src] = -1;
	for(i = 0; i < num; i++) {
	    if(i == src)
	    	continue;
	    result[i] = costs[src][i];
	    route[i] = src;
	    if(result[i] < min_cost) {
	        min_cost = result[i];
	        next_tmp = i;
	    }
	}
	next = next_tmp;
	for(i = 1; i < num; i++) {
		if(next == dest)
			break;
		min_cost = INFINITY;
		flag[next] = 1;
		for(j = 0; j < num; j++) {
			if(!flag[j]) {
				if(result[next] + costs[next][j] < result[j]) {
					route[j] = next;    
					result[j] = result[next] + costs[next][j];
				}
			    if(result[j] < min_cost) {
			        min_cost = result[j];
					next_tmp = j;
			    }
			}
		}
		next = next_tmp;
	}
	if(min_cost != INFINITY) {
		printf("path: %d", dest);
		while(route[dest] != -1) {
			dest = route[dest];
			printf("<-%d", route[dest]);
		}
		printf("\n");
	}
	free(route);
	free(flag);
	free(result);
	return min_cost;
}
