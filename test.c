#include <stdio.h>
#include <stdlib.h>

#define DEADLOCK_STATE 10 // arbitrary number to define the deadlock state. Probably should use “enum” instead

// TODO: Create Rag struct
struct _ragProcess;

typedef struct _ragResource {
    int id
    struct _ragResource *next;
    struct _ragProcess *allocated;
} RagResource;

typedef struct _ragResourceRequest {
    struct _ragResource *request;
    struct _ragResourceRequest *next;
} RagResourceRequest;

typedef struct _ragProcess {
    int id
    struct _ragProcess *next;
    struct _ragResourceRequest *requests;
} RagProcess;

typedef struct Rag {
    struct _ragProcess *processes;
    struct _ragResource *resources;
}

void Rag_init(Rag *rag, int m, int n)
{
    rag->processes = NULL;
    rag->resources = NULL;
    RagProcess *process = NULL;
    RagResource *resource = NULL;
    for (int i = 0; i < n - 1; i++){
        process = rag->processes;
        if (process == NULL){
            process = calloc(1, sizeof(RagProcess));
            continue;
        }
        while (process->next){
            process = process->next;
        }
        process->next = calloc(1, sizeof(RagProcess));
        process->next->id = i + 1;
    }

    for (int i = 0; i < m - 1; i++){
        resource = rag->resource;
        if (resource == NULL){
            resource = calloc(1, sizeof(RagResource));
            continue;
        }
        while (resource->next){
            resource = resource->next;
        }
        resource->next = calloc(1, sizeof(RagResource));
        resource->next->id = i + 1;
    }
}

int Rag_process(Rag *rag, int process_id, char request_type, int resource_id)
{
    RagProcess *process = rag->processes;
    while (process && process->id != process_id)
        process = process->next;
    if (!process)
    {
        printf("ERROR: %d is not a valid process!\n", process_id);
        exit(1);
    }
    RagResource *resource = rag->resources;
    while (resource && resource->id != resource_id)
        resource = resource->next;
    if (!resource)
    {
        printf("ERROR: %d is not a valid resource!\n", resource_id);
        exit(1);
    }
    switch (request_type)
    {
        case ‘A’:
            if (resource->allocated)
            {
                if (!process->requests)
                {
                    process->requests = malloc(sizeof(RagResourceRequest));
                    process->requests->next = NULL;
                    process->requests->request = resource;
                }
                else
                {
                    RagResourceRequest *requests = process->requests;
                    while (requests->next)
                        requests = requests->next;
                    requests->next = malloc(sizeof(RagResourceRequest));
                    requests = requests->next;
                    requests->next = NULL;
                    requests->request = resource;
                }
            }
            else
                resource->allocated = process;
            break;
        case ‘D’:
            if (resource->allocated != process)
            {
                printf("ERROR: Process %d cannot deallocate resource %d which is allocated to it.\n",
                       process_id, resource_id);
                exit(1);
            }
            resource->allocated = NULL;
            process = rag->processes;
            bool found_request = false;
            while (process)
            {
                RagResourceRequest *trailing_request = NULL;
                RagResourceRequest *process_requests = process->requests;
                while (process_requests)
                {
                    if (process_requests->request == resource)
                    {
                        if (trailing_request == process_requests)
                        {
                            process->requests = NULL;
                            free(process_requests);
                        }
                        else
                        {
                            trailing_request->next = process_requests->next;
                            free(process_requests);
                        }
                        found_request = true;
                        break;
                    }
                    trailing_request = process_requests;
                    process_requests = process_requests->next;
                }
                if (found_request)
                    break;
                process = process->next;
            }
            resource->allocated = process;
            break;
        default:
            printf("ERROR: %c is not a valid request type.\n", request_type);
            exit(1);
            break;
    }

    // TODO: detect a deadlock

    if (deadlock)
        return DEADLOCK_STATE;

    return state;
}

int parse_line(char *line, int linesize, int *process_id, char *request_type, int *resource_id)
{
    *process_id = atoi(strtok(line, ','));
    request_type = strtok(NULL, ',');
    *resource_id = atoi(strtok(NULL, ','));
}

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
