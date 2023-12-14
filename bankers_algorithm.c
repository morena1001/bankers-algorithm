#include <stdio.h>             
#include <stdlib.h>            
#include <string.h> 
#include <stdbool.h>

struct process
{
    bool suspended;
};

struct state 
{
    int *resources;
    int *available;
    int *claim;
    int *alloc;
};

void assign_resource_units (struct state *state_data, int resources);
void assign_claimed_units (struct state *state_data, int resources, int processes);
void resource_release_algorithm (struct state *state_data, int data[3], int p_count, int r_count);
void resource_allocation_algorithm (struct state *state_data, int data[3], int p_count, int r_count);
bool safe (struct state state_data, int m, int n);



int main (int argc, char **argv)
{
    struct state state_data;
    int r = 0;
    int p = 0;

    bool resource_number = false;
    bool process_number = false;

    // Get arguments from command line 
    while (*argv != NULL) 
    {
        if (**argv == '-') 
        {
            switch ((*argv) [1]) 
            {
                case 'h':
                    fprintf (stderr, "usage: bankers_algorithm [-h] -r <number of resources> -p <number of processes>\n");
                    break;

                case 'r':
                    resource_number = true;
                    break;

                case 'p':
                    process_number = true;
                    break;

                default:
                fprintf (stderr, "Error: invalid option. Use -h for help.\n");
            }
        } 

        else 
        {
            if (resource_number && !process_number)
                r = atoi (*argv);
            else if (process_number && resource_number)
                p = atoi (*argv);
        }
        argv++;
    }
    // system("cls");

    // Check if a correct input was given for the number of resources and processes
    if (r < 1)
    {
        fprintf (stderr, "Error: missing the number of resources. Try again.\n");
        return 0;
    }
    else if (p < 1)
    {
        fprintf (stderr, "Error: missing the numbers of processes. Try again\n");
        return 0;
    }

    // Allocate memory for the matrices and vectors
    state_data.resources = malloc (sizeof (int) * r);
    state_data.available = malloc (sizeof (int) * r);
    state_data.claim     = malloc (sizeof (int) * (p * r));
    state_data.alloc     = malloc (sizeof (int) * (p * r));

    // Assign initial resource and claim units
    assign_resource_units (&state_data, r);
    // system("cls");
    assign_claimed_units (&state_data, r, p);
    // system("cls");

    fprintf (stdout, "All resources and process claims have been assigned. Now you can request and release resources.\n");
    fprintf (stdout, "If you need help requesting and releasing resources, simply type 'h'.\n");
    fprintf (stdout, "If you want to stop requesting and releasing, simply type '.'.\n\n");

    char input [30];
    int i;
    bool request;
    bool release;
    int process;
    int resource;
    int units;
    
    // While loop for requests and releases
    while (fgets(input, sizeof(input), stdin))
    {
        if (*input == '.')
            break;
            
        else if (!strcmp (input, "-h\n"))
        {
            fprintf (stdout, "To request units of a resource, type 'request(<process number>, <resource number>, <units being requested>)'\n");
            fprintf (stdout, "To release units of a resource, type 'release(<process number>, <resource number>, <units being released>)\n\n");
        }

        else
        {
            char *token = strtok (input, " (,)\n");
            i = process =  resource = units = 0;
            request = false;
            release = false;

            // Parse input and check if request or release results in a safe state
            while (token != NULL)
            {   
                switch (i)
                {
                case 0:
                    if (!strcmp (token, "request"))
                        request = true;
                    else if (!strcmp (token, "release"))   
                        release = true;
                    break;
                
                case 1:
                    process = atoi (token);
                    break;
                
                case 2:
                    resource = atoi (token);
                    break;

                case 3:
                    units = atoi (token);
                    break;
                
                default:
                    fprintf (stderr, "Error: input is not valid. Try again. %s\n", token);
                    break;
                }

                i++;
                token = strtok (NULL, " (,)\n");
            }

            int data [3] = { process, resource, units };
            if (request)
                resource_allocation_algorithm (&state_data, data, p, r);
            else if (release)
                resource_release_algorithm (&state_data, data, p, r);
        }
    }
}



void assign_resource_units (struct state *state_data, int resources)
{
    char input [5];

    fprintf (stdout, "Please specify the number of units each resource will have:\n");
    for (int m = 0; m < resources; m++)
    {
        fprintf (stdout, "Units for resource r%d: ", m);
        fgets (input, sizeof (input), stdin);
        
        state_data->resources [m] = atoi (input);
        state_data->available [m] = atoi (input);
    }
}



void assign_claimed_units (struct state *state_data, int resources, int processes)
{
    char input [5];

    fprintf (stdout, "Please specify the number of units of each resource each process will request:\n");
    for (int n = 0; n < processes; n++)
    {
        for (int m = 0; m < resources; m++)
        {
            fprintf (stdout, "Units of resource r%d for process p%d: ", m, n);
            fgets (input, sizeof (input), stdin);

            while (atoi (input) > state_data->resources [m])
            {
                fprintf(stdout, "Process p%d is trying to claim more than resource r%d has (r%d only has %d units). Try again: ", n, m, m, state_data->resources [m]);
                fgets (input, sizeof (input), stdin);
            }
            
            state_data->claim [n * resources + m] = atoi (input);
            state_data->alloc [n * resources + m] = 0;
        }
    }
}



void resource_allocation_algorithm (struct state *state_data, int data[3], int p_count, int r_count)
{
    int process_number = data [0];
    int resource_number = data [1];
    int units = data [2];
    struct state new_state_data;

    // Make sure the requested and already allocated units of resource_number to process_number
    // Does not exceed the total claimed units    
    if (state_data->alloc [process_number * r_count + resource_number] + units > 
        state_data->claim [process_number * r_count + resource_number])    
    {
        // system("cls");
        fprintf (stderr, "Error: process %d requesting more than maximum claim for resource %d\n\n", process_number, resource_number);
       return;
    }

    // Make sure the requested units of resource_number do not exceed the units available for that resource
    if (units > state_data->available [resource_number])
    {
        // system("cls");
        fprintf (stderr, "Not granted, not enough units of resource %d for process %d\n\n", resource_number, process_number);
       return;
    }

    // Get the new state
    else
    {
        // Allocate memory for the new state
        new_state_data.resources = malloc (sizeof (int) * r_count);
        new_state_data.available = malloc (sizeof (int) * r_count);
        new_state_data.claim     = malloc (sizeof (int) * (p_count * r_count));
        new_state_data.alloc     = malloc (sizeof (int) * (p_count * r_count));

        // Copy the original state into the new state
        for (int i = 0; i < r_count; i++)
            new_state_data.resources [i] = state_data->resources [i];
        for (int i = 0; i < r_count; i++)
            new_state_data.available [i] = state_data->available [i]; 
        for (int j = 0; j < p_count; j++)
            for (int i = 0; i < r_count; i++)
                new_state_data.claim [j * r_count + i] = state_data->claim [j * r_count + i];
        for (int j = 0; j < p_count; j++)
            for (int i = 0; i < r_count; i++)
                new_state_data.alloc [j * r_count + i] = state_data->alloc [j * r_count + i];

        // Prepare the new state with the new info
        new_state_data.alloc [process_number * r_count + resource_number] += units;
        new_state_data.available [resource_number] -= units;
    }

    // Check if new state is safe state
    if (safe (new_state_data, r_count, p_count))
    {
        state_data->alloc [process_number * r_count + resource_number] += units;
        state_data->available [resource_number] -= units;
        // system("cls");
        fprintf (stdout, "Request granted\n\n");
    }
    else
    {
        // system("cls");
        fprintf (stderr, "Request denied\n\n");
    }
}


bool safe (struct state state_data, int m, int n)
{
    int *current_available = malloc (sizeof (int) * m);
    int *rest = malloc (sizeof (int) * n);
    for (int i = 0; i < m; i++)
        current_available [i] = state_data.available[i];
    for (int i = 0; i < n; i++)
        rest [i] = i;

    bool possible = true;
    bool found = false;
    int i = 0;
    int j = 0;
    int row_found;
    int column_found;

    while (possible)
    {
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < m; j++)
            {
                if (state_data.claim [i * m + j] - state_data.alloc [i * m + j] > current_available [j] || rest [i] == -1)
                    break;
                if (j == m - 1)
                {
                    for (int k = 0; k < n; k++)
                    {
                        if (rest [k] == i)
                        {
                            found = true;
                            row_found = i;
                            column_found = j;
                            goto skip_loops;
                        } 
                    }
                }
            }
        }
       skip_loops:
        if (found)
        {
            for (int a = 0; a < m; a++)
                current_available[a] += state_data.alloc [row_found * m + a];
            for (int a = 0; a < n; a++)
                if (rest [a] == row_found)
                    rest [a] = -1;
            found = false;
        }
        else
            possible = false;
    }
    free(current_available);
    for (int a = 0; a < n; a++)
        if (rest [a] != -1)
            return false;
    
    return true;
}

void resource_release_algorithm (struct state *state_data, int data[3], int p_count, int r_count)
{
    int process_number = data [0];
    int resource_number = data [1];
    int units = data [2];

    // if units being released is greates than allocated, deny request
    if (units > state_data->alloc [process_number * r_count + resource_number])
    {
        // system("cls");
        fprintf (stderr, "Error: process %d releasing more than allocated units for resource %d\n", process_number, resource_number);
        return;
    }
    else
    {
        state_data->alloc [process_number * r_count + resource_number] -= units;
        state_data->available [resource_number] += units;
        // system("cls");
        fprintf (stdout, "Release granted\n");
    }
}
