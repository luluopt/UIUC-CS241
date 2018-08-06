/**
 * Parallel Make
 * CS 241 - Fall 2017
 */


#include "format.h"
#include "graph.h"
#include "dictionary.h"
#include "queue.h"
#include "parmake.h"
#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#define RS_INIT  0x0
#define RS_PASS  0x1
#define RS_FAIL  0x2 
#define RS_DROP  0x3
#define RS_BUILD 0x4


// Bookkeeping struct for Tarjan algorithm
typedef struct tarjan_fields_st
{
    int index;
    int lowlink;
    int is_on_stack;

    int color;      // initiallly 0 - White, 1 - Grey, 2 - Black
} tarjan_fields_st;

typedef struct agent_args_st
{
    graph  *dep_graph;
    vector *sorted;
    vector *build_targets;
    vector *cycles;
    pthread_mutex_t *mt_resource; // mutex for all resources, since the build_queue and graph share the same data
    pthread_cond_t  *cv_build;    // use this to signal any changes to the nodes
} agent_args_st;

int min(int a, int b)
{
    return a <= b ? a : b;
}

// Implementation of the Tarjan's algorithm
void tarjan_strongconnect(void *v, rule_t *rule, graph *g, vector *stack, int *index, vector *cycles)
{
    tarjan_fields_st *tracker = malloc(sizeof(tarjan_fields_st));
    memset(tracker, 0, sizeof(tarjan_fields_st));
  
    // Associate the Tarjan's fields with the vertex itself
    rule->data = tracker;

    tracker->index   = *index;
    tracker->lowlink = *index;
    (*index)++;
    vector_push_back(stack, v);
    tracker->is_on_stack = 1;
   
    // Look at the children of v 
    vector *children     = graph_neighbors(g, v);
    size_t  num_children = vector_size(children); 
    char   *item   = NULL;
    rule_t *w_rule = NULL;
    tarjan_fields_st *w_tracker = NULL;
    
    if (num_children > 0) {
        VECTOR_FOR_EACH(children, child, {
            // Check if the child has been visited yet
            item = (char*)child;
            w_rule = (rule_t*)graph_get_vertex_value(g, item);
            if (w_rule->data == NULL) {
                tarjan_strongconnect(child, w_rule, g, stack, index, cycles);
                w_tracker = (tarjan_fields_st*)(w_rule->data);
                tracker->lowlink = min(tracker->lowlink, w_tracker->lowlink);
            } else {  // If the child has been visited already, check if it's still on stack
                w_tracker = (tarjan_fields_st*)(w_rule->data);
                if (w_tracker->is_on_stack) {
                    tracker->lowlink = min(tracker->lowlink, w_tracker->lowlink);
                }
            } 
        });
    }

    // if v is a root node, generate a vector that describes the cycle
    void *popped = NULL;

    if (tracker->lowlink == tracker->index) {
        vector *scc = vector_create(NULL, NULL, NULL);
        do {
            
            popped = *(vector_back(stack));
            vector_pop_back(stack);
            w_rule = (rule_t*)graph_get_vertex_value(g, popped);
            w_tracker = (tarjan_fields_st*)(w_rule->data); 
            w_tracker->is_on_stack = 0;

            vector_push_back(scc, popped);
        } while (popped != v);

        if (vector_size(scc) == 1) {
            // an scc size of 1 means the node is not in a cycle
            vector_destroy(scc);
        } else {
            // Add the current cycle to the collection of cycles detected
            vector_push_back(cycles, scc); 
        }
    }

}

// Implement Tarjan's algorithm to detect cycles in an graph
vector* detect_cycles(graph *g)
{
    vector    *vertices = graph_vertices(g);
    rule_t    *rule     = NULL;
    vector    *stack    = vector_create(NULL, NULL, NULL);

    vector    *cycles   = vector_create(NULL, NULL, NULL);    

    int   index = 0;
    char *item  = NULL;

    VECTOR_FOR_EACH(vertices, vertex, {
        item = (char*)vertex;        
        rule = (rule_t*)graph_get_vertex_value(g, item);
        if (rule->data == NULL) {
            tarjan_strongconnect(vertex, rule, g, stack, &index, cycles);
        }
    }); 

    vector_destroy(stack);

    return cycles;
}

bool is_vertex_in_cycle(void *v, vector* cycles)
{
    // Check the provided node to see if it's in any of the cycles
    if (vector_size(cycles) > 0) {
        VECTOR_FOR_EACH(cycles, cycle, {
            VECTOR_FOR_EACH((vector*)cycle, node, {
                if (node == v) {
                    return true;
                }
            });
        });
    }

    return false;
}

// Check if the node, or any of its children are in a cycle
bool is_vertex_depend_on_cycle(graph *g, void *v, vector* cycles)
{
    // Do not check for vector size, since this function is only called
    // when number of cycles > 0
    vector *children = graph_neighbors(g, v);
    int    num_children = vector_size(children);

    if (num_children == 0) {
        // If the node is a leaf node, ie. no dependencies
        return is_vertex_in_cycle(v, cycles);
    } else {
        // Check the node itself first
        if (is_vertex_in_cycle(v, cycles)) {
            return true;
        }

        // Recursively check all children
        VECTOR_FOR_EACH(children, child, {
            if (is_vertex_depend_on_cycle(g, child, cycles)) {
                return true;
            }
        });

        // Should not reach here
        return false;
    }
}

rule_t *graph_get_rule(graph *g, void *v)
{
    return (rule_t*)graph_get_vertex_value(g, v);
}

tarjan_fields_st *graph_get_vertex_stats(graph *g, void *v)
{
    rule_t *rule  = graph_get_rule(g, v);
    return (tarjan_fields_st*)(rule->data);
}

int graph_get_vertex_color(graph *g, void *v)
{
    return graph_get_vertex_stats(g, v)->color;
}

char *graph_find_unmarked(graph *g)
{
    vector *vertices = graph_vertices(g);
    if (vector_size(vertices) == 0) {
        return NULL;
    }
    
    VECTOR_FOR_EACH(vertices, vertex, {
        if (graph_get_vertex_color(g, vertex) == 0) { // unmarked
            return (char*)vertex;
        }
    });

    return NULL;
}

int graph_has_unmarked(graph *g) 
{
    vector *vertices = graph_vertices(g);
    if (vector_size(vertices) == 0) {
        return 0;
    }
    
    VECTOR_FOR_EACH(vertices, vertex, {
        if (graph_get_vertex_color(g, vertex) == 0) { // unmarked
            return 1;
        }
    });

    return 0;
}

void graph_visit(graph *g, void *node, vector *sorted)
{
    tarjan_fields_st *stats = graph_get_vertex_stats(g, node); 
    if (stats->color == 2) {
        return;
    } else if (stats->color == 1) {
        return;
    }

    // mark as GREY
    stats->color = 1;

    vector *children     = graph_neighbors(g, node);
    size_t  num_children = vector_size(children); 
    if (num_children > 0) {
        VECTOR_FOR_EACH(children, child, {
            graph_visit(g, child, sorted);
        });
    }
    
    // mark as BLACK
    stats->color = 2;
    
    vector_push_back(sorted, node);
}

// Implement DFS as a topographical sort on a graph
vector* graph_topo_sort(graph *g)
{
    // Allocate space for a vector that stores the sorted list
    vector *sorted = vector_create(NULL, NULL, NULL);

    char *node = NULL;
    while (graph_has_unmarked(g)) {
        node = graph_find_unmarked(g);
        graph_visit(g, node, sorted);
    }   

    return sorted;
}

bool need_to_build_file(struct stat *target_fstat, vector *children)
{
    if (vector_size(children) == 0) {
        // if target is a file, but has no dependencies, then the build rule 
        // is always satisfied, we should never call its recipe 
        return false;
    } else {
        // Check if each child is a file, and they have an older timestamp than
        // target
        struct stat c_fstat;
        int         c_status;

        VECTOR_FOR_EACH(children, child, {
            c_status = stat((char*)child, &c_fstat);
            
            // if any child is not a file, build target
            if (c_status != 0) {
                return true;
            }
            
            // if any child is newer than the target, build target
            if (difftime(c_fstat.st_mtime, target_fstat->st_mtime) > 0) {
                return true;
            }
        });

        // Doesn't need to build target if we reached here
        return false;
    }
}

int can_build_target(graph *dep_graph, char *target)
{
    vector *children     = graph_neighbors(dep_graph, target);
    size_t  num_children = vector_size(children);

    rule_t *c_rule = NULL;

    if (num_children > 0) {
        bool children_drop = false;
        bool children_fail = false;
        bool children_pass = true;
        VECTOR_FOR_EACH(children, child, {
            c_rule = graph_get_rule(dep_graph, child);
            if (c_rule->state != RS_PASS) {
                children_pass   = false;
                
                if (c_rule->state == RS_FAIL) {
                    children_fail = true;
                } else if (c_rule->state == RS_DROP) {
                    children_drop = true;
                }
            }
        });

        // Only continue if all children dependencies have PASS
        // otherwise, return to caller, caller will decide what to do
        if (!children_pass) {
            rule_t *target_rule = graph_get_rule(dep_graph, target);
            if (children_drop) {
                target_rule->state = RS_DROP;
                return RS_DROP;
            } else if (children_fail) {
                target_rule->state = RS_FAIL;
                return RS_FAIL;
            } else {
                target_rule->state = RS_INIT;
                return RS_INIT;
            }
        }
    }

    // Should only be here if there is no child, or all of them have been built
    
    // Check if target is a file on the filesystem
    struct stat fs_buffer;
    int         fs_status;
    fs_status = stat(target, &fs_buffer);
    
    if (fs_status == 0) {  // target is a file
        // target is satisfied, doesn't need to rebuild
        if (!need_to_build_file(&fs_buffer, children)) {
            rule_t *target_rule = graph_get_rule(dep_graph, target);
            target_rule->state = RS_PASS;
            return RS_PASS;
        }
    }

    // If we reached here, we need to execute the recipe
    return RS_BUILD;
}

int execute_recipe(rule_t *rule)
{
    vector *cmds = rule->commands;
    if (vector_size(cmds) <= 0) {
        return RS_PASS;
    }
   
    // Run the commands sequentially
    int rc;
    VECTOR_FOR_EACH(cmds, cmd, {
        rc = system((char*)cmd);
        if (rc != 0) {
            return RS_FAIL;
        }
    });

    return RS_PASS;
}

void char_destroy(void *ptr)
{
    char *temp = (char*)ptr;
    free(temp);
}

// Create a vector that stores all the build targets passed in by command line
vector* store_targets(char **targets, graph *graph)
{
    vector *build_targets = vector_create(NULL, NULL, NULL);
    vector *vertices      = graph_vertices(graph);

    // If no targets are specified, then we stop at the sentinel node
    if (*targets == NULL) {
        VECTOR_FOR_EACH(vertices, v, {
            if (strcmp("", (char*)v) == 0) {
                vector *children = graph_neighbors(graph, v);
                char   *first    = (char*)*(vector_front(children));
                vector_push_back(build_targets, first);
            }
        }); 
        return build_targets;
    }
    
    // Mark all target node
    while(*targets != NULL) {
        VECTOR_FOR_EACH(vertices, v, {
            if (strcmp(*targets, (char*)v) == 0) {
                vector_push_back(build_targets, v);
            }
        }); 
        targets++;
    }

    return build_targets;
}

// Check against all intended targets to see if we complete the build
bool build_complete(vector *targets, char *target)
{
    int num_targets = vector_size(targets);
    char *temp = NULL;    
    int idx = -1;

    // Search the vector to see if we completed one of the build targets
    for (int i=0; i<num_targets; i++) {
        temp = (char*)(*(vector_at(targets, i)));
        if (strcmp(temp, target) == 0) {
            idx = i;
        }
    }

    // Remove from build targets if we completed it
    if (idx >= 0) {
        vector_erase(targets, idx);
    }

    if (vector_size(targets) == 0) {
        vector_destroy(targets);
        return true;
    } else {
        return false;
    }
}

// Check build targets to cycle dependencies, and print them if they depend on cycles
void annouce_drop_cycle_targets(graph *g, vector *targets, vector *cycles)
{
    int num_targets = vector_size(targets);
    char *temp = NULL;

    for (int i=0; i<num_targets; i++) {
        temp = (char*)(*(vector_at(targets, i)));
        if (is_vertex_depend_on_cycle(g, temp, cycles)) {
            print_cycle_failure(temp);
        }
    }
}

// Return the index within the sorted vector, if a rule can either be marked as fail, or ready to start
int find_rule_to_work_on(graph *g, vector *sorted, vector* cycles, int *next_state)
{
    unsigned int i = 0;
    char *node = NULL;
    for (i=0; i<vector_size(sorted); i++) {
        node = (char*)(*(vector_at(sorted, i)));
        if (is_vertex_in_cycle(node, cycles)) {
            // Part of a cycle, drop the rule
            *next_state = RS_DROP;
            graph_get_rule(g, node)->state = RS_DROP;
            return i;
        } else {
            *next_state = can_build_target(g, node); 
            if (*next_state != RS_INIT) {
                // RS_INIT means we have to wait for dependencies to build
                // check the rest of the vector
                return i;
            }
        } 
    }

    return -1;
}

// Functions for parallelization
void* start_build_agent(void *args)
{
    // save some typing
    agent_args_st *agent_args     = (agent_args_st*)args;
    graph         *dep_graph      = agent_args->dep_graph;
    vector        *sorted         = agent_args->sorted;
    vector        *build_targets  = agent_args->build_targets;
    vector        *cycles         = agent_args->cycles;
    pthread_mutex_t *mt_resource  = agent_args->mt_resource;
    pthread_cond_t  *cv_build     = agent_args->cv_build;
    
    char *rule_name  = NULL;
    int   next_state = -1;

    int   node_idx   = -1;

    while (true) {
        pthread_mutex_lock(mt_resource);
        if (vector_size(sorted) > 0) {
            // see if there is any nodes we can work on, or drop 
            node_idx = find_rule_to_work_on(dep_graph, sorted, cycles, &next_state);

            if (node_idx == -1) {
                // RS_INIT means some children has not completed yet
                
                // This means we need to wait until some other agents completes
                // to avoid busy waiting, we use a condition variable here
                pthread_cond_wait(cv_build, mt_resource);
                pthread_mutex_unlock(mt_resource);
            } else {
                rule_name = (char*)(*vector_at(sorted, node_idx));

                // pop the front element
                vector_erase(sorted, node_idx);

                // Check if we completed all the build targets
                // We have to do this before actual execution of the recipe because we
                // don't want the next thread to build un-wanted rules
                if (build_complete(build_targets, rule_name)) {
                    vector_clear(sorted);
                }
            
                pthread_mutex_unlock(mt_resource);
                // signal other threads there were changes to the tree
                pthread_cond_broadcast(cv_build);
                
                if (next_state == RS_BUILD) {
                    rule_t *rule = graph_get_rule(dep_graph, rule_name);
                    int     rc   = execute_recipe(rule);
                    
                    // Update the state of the node, need to obtain a lock here to avoid
                    // race condition where another thread just happen to check the node
                    // before this gets set
                    pthread_mutex_lock(mt_resource);
                    rule->state  = rc;
                    pthread_mutex_unlock(mt_resource);
                } 

                // changes to the tree happened, we signal other threads they should check
                // again
                pthread_cond_broadcast(cv_build);
            }
        } else {
            // Nothing to build any more, the thread can exit
            pthread_mutex_unlock(mt_resource);
            return NULL;
        }
    }
}

int parmake(char *makefile, size_t num_threads, char **targets) 
{
    // SINGLE THREAD CODE
    // Parse graph    
    graph  *dep_graph     = parser_parse_makefile(makefile, targets);
    vector *build_targets = store_targets(targets, dep_graph);
    
    vector *cycles        = detect_cycles(dep_graph);

    // establish build-order by doing a topographical sort
    // use a vector because we need to be able to peek instead of pop
    vector *sorted    = graph_topo_sort(dep_graph);

    if (vector_size(cycles) > 0) {
        annouce_drop_cycle_targets(dep_graph, build_targets, cycles);
    }

    // END SINGLE THREAD CODE
    
    
    // MULTI-THREAD CODE
    agent_args_st *agent_args = malloc(num_threads * sizeof(agent_args_st));
    memset(agent_args, 0, num_threads* sizeof(agent_args_st)); 

    pthread_t       *th_agents   = malloc(num_threads * sizeof(pthread_t));
    pthread_mutex_t *mt_resource = malloc(sizeof(pthread_mutex_t));
    pthread_cond_t  *cv_build    = malloc(sizeof(pthread_cond_t)); 

    pthread_mutex_init(mt_resource, NULL);
    pthread_cond_init(cv_build, NULL);

    unsigned int i = 0;
    for (i=0; i<num_threads; i++) {
        agent_args[i].dep_graph     = dep_graph;
        agent_args[i].sorted        = sorted;
        agent_args[i].build_targets = build_targets; 
        agent_args[i].cycles        = cycles;
        agent_args[i].mt_resource   = mt_resource;
        agent_args[i].cv_build      = cv_build;
    }

    for (i=0; i<num_threads; i++) {
        pthread_create(&(th_agents[i]), NULL, start_build_agent, (void*)&(agent_args[i]));
    }

    for (i=0; i<num_threads; i++) {
        pthread_join(th_agents[i], NULL);
    }

    free(agent_args);
    pthread_mutex_destroy(mt_resource);
    pthread_cond_destroy(cv_build);

    return 0;
}
