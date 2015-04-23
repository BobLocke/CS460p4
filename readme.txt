Ryan "Bob" Dean and Richard Barella
CS 460 - Program 4 - Resource Allocation Graph

This program simulates resource allocation, based in an input file piped in from the command line with instructions. If the program detects a deadlock, it will abort and output the problem processes/resources.

IMPORTANT: The input file must have spaces after each comma, or the parser will NOT work.

To run: ./scheduler m n < inputfile

m: Number of processes.
n: Number of resources.
inputfile: inputfile with allocation and deallocation instructions.

    Intructions are read line by line, in the format below:
    process_id, instruction, resource_id

    process_id: Id# of the process requesting action. Must be less than m.
    instruction: 
        A: request allocation of resourse to process
        D: request deallocation of resource to process
    resource_id: id# of the resource requested. Must be less than n.

    IMPORTANT: The input file must have spaces after each comma, or the parser will NOT work.

