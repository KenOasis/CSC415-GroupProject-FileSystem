#include "mfs.h"
#include "freeSpace.h"
#include "vcb.h"
#include <stdio.h>

int main(){
	vcb* volume = initializeVCB(5000);
	printf("Mike\n");
	volume->name = malloc(sizeof(char) * 5);
	volume->name = "Mike"; //for testing only
	for (int n = 0; n < volume->free_space_ptr->size; n++) {
		printf("%d\n", volume->free_space_ptr->bitVector[n]);
	}
	return 0;
}
