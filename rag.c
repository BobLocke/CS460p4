#include <stdio.h>
#include <stdlib.h>

// TODO: Create Rag struct

int main(int argc, char** argv){
	int m, n = 0;

	if (argc != 2){
		printf("ERROR! NO COMPREHENDO! You must be speaking in another language...\n");
		return -1;
	}

	int m = atoi(argv[1]);
	int n = atoi(argv[2]);
 
 	Rag resource_allocation_graph;
 	Rag_init(&resource_allocation_graph, m, n);

 	while (getline(&line, &size, stdin) != -1)
 	{
 		int process_id;
 		char request_type;
 		int resource_id;
 		int output_result;
 		if (parse_line(line, &process_id, &request_type, &resource_id) < 0)
 		{
 			printf("ERROR! WORLD WILL EXPLODE IN 4 MINUTES!\n");
 			return -1;
 		}
 		output_result = Rag_process(&resource_allocation_graph, process_id, request_type, resource_id);
 		switch (output_result)
 		{
 			case 0: // REQUEST
 				printf("Request...\n");
 				break;
 			case 1: // ALLOC
 				printf("Alloc...\n");
 				break;
 			case 2: // CANCEL
 				printf("Cancel...\n");
 				break;
 			case 3: // DEALLOC
 				printf("Dealloc...\n");
 				break;
 			case 4: // DEADLOCK
 				printf("ERROR! IT'S THE DEADLOCK! THE DEADLY EMBRACE! OH NO! WHY DID IT EVER COME TO THIS!\n");
 				return 0;
 				break;
 			default:
 				printf("ERROR! SPACETIME RIFT DETECTED, SHUTTING DOWN!\n");
 				return -1;
 				break;
 		}
 	}
 	// No deadlock
 	printf("No deadlock! Yay!\n");

	return 0;
}