#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct _request_queue_node {
    struct _request_queue_node *next;
    int process;
} RequestQueueNode;

typedef struct _request_queue {
    RequestQueueNode *front;
    RequestQueueNode *back;
} RequestQueue;

typedef enum {REQUEST_STATE,
              ALLOC_STATE,
              CANCEL_STATE,
              DEALLOC_STATE,
              DEADLOCK_STATE} state_t;

int **requested;
int **allocated;
int *processes_visited;
int *resources_visited;
int number_processes;
int number_resources;
RequestQueue **request_queues;

RequestQueue *request_queue_constructor()
{
    RequestQueue *q = malloc(sizeof(RequestQueue));
    q->front = NULL;
    q->back = NULL;
    return q;
}

void request_queue_enqueue(RequestQueue *q, int process)
{
    assert(q);
    RequestQueueNode *new_front = malloc(sizeof(RequestQueueNode));
    new_front->process = process;
    new_front->next = NULL;
    if (q->front == NULL)
    {
        q->front = q->back = new_front;
    }
    else
    {
        q->front->next = new_front;
        q->front = new_front;
    }
}

int request_queue_dequeue(RequestQueue *q)
{
    assert(q);
    assert(q->back);
    int process = q->back->process;
    RequestQueueNode *del_node = q->back;
    if (q->back == q->front)
        q->back = q->front = NULL;
    else
        q->back = q->back->next;
    free(del_node);
    return process;
}

void request_queue_remove(RequestQueue *q, int process)
{
    assert(q);
    assert(q->back);
    if (q->back->process == process)
    {
        request_queue_dequeue(q);
        return;
    }
    RequestQueueNode *q_node = q->back;
    RequestQueueNode *trail_node = NULL;
    while (q_node)
    {
        if (q_node->process == process)
        {
            assert(trail_node);
            RequestQueueNode *del_node = q_node;
            q_node = q_node->next;
            trail_node->next = q_node;
            free(del_node);
            return;
        }
        trail_node = q_node;
        q_node = q_node->next;
    }
    assert(0);
}

int request_queue_is_empty(RequestQueue *q)
{
    assert(q);
    if (q->front)
        return 0;
    else
        return 1;
}

void request_queue_purge(RequestQueue *q)
{
    assert(q);
    while (!request_queue_is_empty(q))
        request_queue_dequeue(q);
    free(q);
}

void Rag_init(int m, int n)
{
    number_processes = m;
    number_resources = n;
    processes_visited = calloc(m, sizeof(int));
    resources_visited = calloc(n, sizeof(int));
    requested = malloc(m*sizeof(int *));
    allocated = malloc(n*sizeof(int *));
    request_queues = malloc(n*sizeof(RequestQueue *));
    for (int i = 0; i < m; i++)
        requested[i] = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++)
        allocated[i] = calloc(m, sizeof(int));
    for (int i = 0; i < n; i++)
        request_queues[i] = request_queue_constructor();
}

void Rag_deconstruct()
{
    for (int i = 0; i < number_processes; i++)
        free(requested[i]);
    free(requested);
    for (int i = 0; i < number_resources; i++)
        free(allocated[i]);
    free(allocated);
    free(processes_visited);
    free(resources_visited);
    for (int i = 0; i < number_resources; i++)
        request_queue_purge(request_queues[i]);
}

int deadlock_check_resource(int resource);

int deadlock_check_process(int process)
{
    assert(process >= 0 && process < number_processes);
    processes_visited[process] = 1;
    for (int i = 0; i < number_resources; i++)
    {
        if (requested[process][i])
        {
            if (resources_visited[i] == 1)
            {
                printf("P%d\n", process);
                return 1;
            }
            if (resources_visited[i] == 0)
            {
                int return_value = deadlock_check_resource(i);
                printf("P%d\n", process);
                return return_value;
            }
        }
    }
    processes_visited[process] = 2;
    return 0;
}

int deadlock_check_resource(int resource)
{
    assert(resource >= 0 && resource < number_resources);
    resources_visited[resource] = 1;
    for (int i = 0; i < number_processes; i++)
    {
        if (allocated[resource][i])
        {
            if (processes_visited[i] == 1)
            {
                printf("R%d\n", resource);
                return 1;
            }
            if (processes_visited[i] == 0)
            {
                int return_value = deadlock_check_process(i);
                printf("R%d\n", resource);
                return return_value;
            }
        }
    }
    resources_visited[resource] = 2;
    return 0;
}

int deadlock_check()
{
    return deadlock_check_process(0);
}

int Rag_process(int process_id, char request_type, int resource_id)
{
    state_t current_state;
    int resource_is_allocated;
    int i;
    if (process_id >= number_processes || process_id < 0)
    {
        printf("ERROR: %d is not a valid process!\n", process_id);
        exit(1);
    }
    if (resource_id >= number_resources || resource_id < 0)
    {
        printf("ERROR: %d is not a valid resource!\n", resource_id);
        exit(1);
    }
    switch (request_type)
    {
        case 'A':
            // find out if resource is allocated
            resource_is_allocated = 0;
            for (i = 0; i < number_resources; i++)
            {
                if (allocated[resource_id][i])
                {
                    resource_is_allocated = 1;
                    break;
                }
            }
            // allocate or request resource
            if (resource_is_allocated)
            {
                assert(!requested[process_id][resource_id]);
                current_state = REQUEST_STATE;
                requested[process_id][resource_id] = 1;
                request_queue_enqueue(request_queues[resource_id], process_id);
            }
            else
            {
                assert(!allocated[resource_id][process_id]);
                current_state = ALLOC_STATE;
                allocated[resource_id][process_id] = 1;
                if (requested[process_id][resource_id])
                {
                    request_queue_remove(request_queues[resource_id],
                                         process_id);
                    requested[process_id][resource_id] = 0;
                }
            }
            break;
        case 'D':
            if (requested[process_id][resource_id])
            {
                current_state = CANCEL_STATE;
                assert(!allocated[resource_id][process_id]);
                requested[process_id][resource_id] = 0;
                request_queue_remove(request_queues[resource_id],
                                     process_id);
            }
            else if (allocated[resource_id][process_id])
            {
                current_state = DEALLOC_STATE;
                allocated[resource_id][process_id] = 0;
                if (!request_queue_is_empty(request_queues[resource_id]))
                {
                    current_state = ALLOC_STATE;
                    allocated[resource_id][request_queue_dequeue(request_queues[resource_id])] = 1;
                }
            }
            else
            {
                printf("ERROR: Cannot deallocate or cancel. Process %d and resource %d are not connected to each-other by alloc or request.\n",
                       process_id, resource_id);
                exit(1);
            }
            break;
        default:
            printf("ERROR: %c is not a valid request type.\n", request_type);
            exit(1);
            break;
    }

    // TODO: detect a deadlock
    int is_deadlock = 0;
    if (deadlock_check())
        is_deadlock = 1;
    for (i = 0; i < number_processes; i++)
        processes_visited[i] = 0;
    for (i = 0; i < number_resources; i++)
        resources_visited[i] = 0;

    if (is_deadlock)
        return DEADLOCK_STATE;
    else
        return current_state;
}

int parse_line(char *line, int linesize, int *process_id, char *request_type, int *resource_id)
{
    *process_id = atoi(strtok(line, ", "));
    *request_type = strtok(NULL, ", ")[0];
    *resource_id = atoi(strtok(NULL, ", "));
}

int main(int argc, char** argv){
    int m, n = 0;

    if (argc != 3){
        printf("ERROR! NO COMPREHENDO! You must be speaking in another language...\n");
        return -1;
    }

    m = atoi(argv[1]);
    n = atoi(argv[2]);

    Rag_init(m, n);
    char* line = NULL;
    size_t size = 0;

    while (getline(&line, &size, stdin) != -1)
    {
        int process_id;
        char request_type;
        int resource_id;
        int output_result;
        if (parse_line(line, size, &process_id, &request_type, &resource_id) < 0)
        {
            printf("ERROR! WORLD WILL EXPLODE IN 4 MINUTES!\n");
            Rag_deconstruct();
            return -1;
        }
        output_result = Rag_process(process_id, request_type, resource_id);

        int is_deadlock = 0;
        int is_error = 0;
        switch (output_result)
        {
            case REQUEST_STATE:
                printf("Request...\n");
                break;
            case ALLOC_STATE:
                printf("Alloc...\n");
                break;
            case CANCEL_STATE:
                printf("Cancel...\n");
                break;
            case DEALLOC_STATE:
                printf("Dealloc...\n");
                break;
            case DEADLOCK_STATE:
                printf("ERROR! IT'S THE DEADLOCK! THE DEADLY EMBRACE! OH NO! WHY DID IT EVER COME TO THIS!\n");
                is_deadlock = 1;
                break;
            default:
                printf("ERROR! SPACETIME RIFT DETECTED, SHUTTING DOWN!\n");
                is_error = 1;
                break;
        }

        // Start Debug Print Statements
        printf("%d %c %d\n", process_id, request_type, resource_id);
        printf("%d processes. %d resources\n", number_processes, number_resources);
        printf("Requested Matrix:\n");
        for (int i = 0; i < number_processes; i++)
        {
            for (int j = 0; j < number_resources; j++)
                printf("%d ", requested[i][j]);
            printf("\n");
        }
        printf("\n");
        printf("Request Queues:\n");
        for (int i = 0; i < number_resources; i++)
        {
            RequestQueueNode *q_node = request_queues[i]->back;
            while (q_node)
            {
                printf("%d ", q_node->process);
                q_node = q_node->next;
            }
            printf("\n");
        }
        printf("\n");
        printf("Allocated Matrix:\n");
        for (int i = 0; i < number_resources; i++)
        {
            for (int j = 0; j < number_processes; j++)
                printf("%d ", allocated[i][j]);
            printf("\n");
        }
        printf("=============================================\n");
        printf("\n");
        // End Debug Print Statements
        
        if (is_deadlock)
        {
            Rag_deconstruct();
            return 0;
        }
        if (is_error)
        {
            Rag_deconstruct();
            return 1;
        }
    }
    // No deadlock
    printf("No deadlock! Yay!\n");
    
    Rag_deconstruct();
    return 0;
}
