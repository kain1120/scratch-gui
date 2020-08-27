#include <stdio.h>
#include <stdlib.h>
#include "../stat/cpd_stat.h"

CPD_STAT_VISIT_RESULT print_name (CPD_STAT_NODE* node, void * extra)
{
    CPD_STAT_HEAD* parent = (CPD_STAT_HEAD*)node;
    if (node)
    {
        char path[MAX_CPD_STAT_PATH_NAME_LENGTH] = {0};
        cpd_stat_get_path(node, path);
        printf("%s ref:%u\n", path, node->head.ref);
    }

    return CPD_STAT_VISIT_CONTINUE;
}

CPD_STAT_VISIT_RESULT count_node (CPD_STAT_NODE* node, void * extra)
{
    if (node)
    {
        (*((unsigned int*)extra))++;
    }

    return CPD_STAT_VISIT_CONTINUE;
}

CPD_STAT_VISIT_RESULT iterate_print_name (CPD_STAT_NODE* node, void * extra)
{
    CPD_STAT_HEAD* parent = (CPD_STAT_HEAD*)node;
    unsigned int* count = extra;
    CPD_STAT_VISIT_RESULT result = CPD_STAT_VISIT_CONTINUE;

    if (node && !cpd_stat_is_node_visited(node)
        && (!cpd_stat_has_child(node)))
    {
        char path[MAX_CPD_STAT_PATH_NAME_LENGTH] = {0};
        cpd_stat_get_path(node, path);
        printf("%s\n", path);
    }
    return result;
}

CPD_STAT_VISIT_RESULT iterate_print_name_post (CPD_STAT_NODE* node, void * extra)
{
    CPD_STAT_NODE* parent = (CPD_STAT_NODE*)node;
    unsigned int* count = extra;
    CPD_STAT_VISIT_RESULT result = CPD_STAT_VISIT_CONTINUE;

    if (node && !cpd_stat_is_node_visited(node)
        && (!cpd_stat_has_child(node)))
    {
        cpd_stat_mark_node_visited(node);

        if ((*count) > 0)
        {
            --(*count);
        }

        if ((*count) == 0)
        {
            result = CPD_STAT_VISIT_STOP;
        }
    }
    return result;
}

CPD_STAT_VISIT_RESULT iterate_remove_node (CPD_STAT_NODE* node, void * extra)
{
    CPD_STAT_HEAD* head = (CPD_STAT_HEAD*)node;
    CPD_STAT_HEAD* removed = NULL_STAT_HEAD;
	CPD_STAT_ROOT* pool = (CPD_STAT_ROOT*)extra;
    if (head && pool)
    {
        char path[MAX_CPD_STAT_PATH_NAME_LENGTH] = {0};
        cpd_stat_get_path(node, path);
        printf("%s ref:%u\n", path, node->head.ref);
		removed = cpd_stat_del_bachelor(head);
		if (removed)
		{
        	printf("remove successful\n");
			cpd_stat_add_child((CPD_STAT_HEAD*)pool, removed);
		}
		else
		{
        	printf("remove fail\n");
		}
    }

    //return CPD_STAT_VISIT_STOP;
    return CPD_STAT_VISIT_CONTINUE;
}

CPD_STAT_NODE* get_node_from_pool(void* extra)
{
    CPD_STAT_ROOT* root = (CPD_STAT_ROOT*) extra;
    CPD_STAT_HEAD* head = NULL_STAT_HEAD;

    if (root && root->head.child_head)
    {
        head = cpd_stat_del_bachelor(root->head.child_head);
    }

    return ((CPD_STAT_NODE*)head);
}

#define CPD_STAT_TEST_NODE_POOL_SIZE 1024
CPD_STAT_NODE node_array[CPD_STAT_TEST_NODE_POOL_SIZE];
CPD_STAT_ROOT node_pool; 

CPD_STAT_ROOT* cpd_stat_test_prepare_node_pool()
{
    cpd_stat_init_root(&node_pool);
    for (unsigned int i = 0; i < CPD_STAT_TEST_NODE_POOL_SIZE; ++i)
    {
        cpd_stat_init_node(&node_array[i], "");
        cpd_stat_add_child((CPD_STAT_HEAD*)&node_pool,
                (CPD_STAT_HEAD*)&node_array[i]);
    }

    return &node_pool;
}

#define CPD_STAT_TEST_SAMPLE_NUMBER 16
CPD_STAT_ROOT sample_array[CPD_STAT_TEST_SAMPLE_NUMBER];
unsigned int sample_number = 0;

void cpd_stat_test_prepare_sample_array()
{
    for (unsigned int i = 0; i < CPD_STAT_TEST_SAMPLE_NUMBER; ++i)
    {
        cpd_stat_init_root(&sample_array[i]);
    }

    printf ("prepare %uth sample: empty\n", ++sample_number);
    {
        cpd_stat_init_root(&sample_array[sample_number - 1]);
    }

    printf ("prepare %uth sample: 1 node\n", ++sample_number);
    {
        CPD_STAT_NODE* node = NULL_STAT_NODE;

        node = get_node_from_pool((void*)&node_pool);
        if (node)
        {
            cpd_stat_init_node(node, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node);
        }
    }

    printf ("prepare %uth sample: 2 brother node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node2);
        }
    }

    printf ("prepare %uth sample: 3 brother node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;
        CPD_STAT_NODE* node3 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node2);
        }

        node3 = get_node_from_pool((void*)&node_pool);
        if (node3)
        {
            cpd_stat_init_node(node3, "f1_3");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node3);
        }
    }

    printf ("prepare %uth sample: 1 node + 1 child node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f2_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
        }
    }

    printf ("prepare %uth sample: 1 node + 2 child node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;
        CPD_STAT_NODE* node3 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f2_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
        }

        node3 = get_node_from_pool((void*)&node_pool);
        if (node3)
        {
            cpd_stat_init_node(node3, "f2_1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node3);
        }
    }

    printf ("prepare %uth sample: 1 node + 3 child node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;
        CPD_STAT_NODE* node3 = NULL_STAT_NODE;
        CPD_STAT_NODE* node4 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f2_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
        }

        node3 = get_node_from_pool((void*)&node_pool);
        if (node3)
        {
            cpd_stat_init_node(node3, "f2_1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node3);
        }

        node4 = get_node_from_pool((void*)&node_pool);
        if (node4)
        {
            cpd_stat_init_node(node4, "f2_1_3");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node4);
        }
    }

    printf ("prepare %uth sample: 1 node + 3 child node + 1 brother node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;
        CPD_STAT_NODE* node3 = NULL_STAT_NODE;
        CPD_STAT_NODE* node4 = NULL_STAT_NODE;
        CPD_STAT_NODE* node5 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f2_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
        }

        node3 = get_node_from_pool((void*)&node_pool);
        if (node3)
        {
            cpd_stat_init_node(node3, "f2_1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node3);
        }

        node4 = get_node_from_pool((void*)&node_pool);
        if (node4)
        {
            cpd_stat_init_node(node4, "f2_1_3");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node4);
        }

        node5 = get_node_from_pool((void*)&node_pool);
        if (node5)
        {
            cpd_stat_init_node(node5, "f1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node5);
        }
    }

    printf ("prepare %uth sample: 1 node + 3 child node + 1 brother node + 1 brother's child node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;
        CPD_STAT_NODE* node3 = NULL_STAT_NODE;
        CPD_STAT_NODE* node4 = NULL_STAT_NODE;
        CPD_STAT_NODE* node5 = NULL_STAT_NODE;
        CPD_STAT_NODE* node6 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f2_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
        }

        node3 = get_node_from_pool((void*)&node_pool);
        if (node3)
        {
            cpd_stat_init_node(node3, "f2_1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node3);
        }

        node4 = get_node_from_pool((void*)&node_pool);
        if (node4)
        {
            cpd_stat_init_node(node4, "f2_1_3");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node4);
        }

        node5 = get_node_from_pool((void*)&node_pool);
        if (node5)
        {
            cpd_stat_init_node(node5, "f1_2");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node5);
        }

        node6 = get_node_from_pool((void*)&node_pool);
        if (node6)
        {
            cpd_stat_init_node(node6, "f2_2_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node5, (CPD_STAT_HEAD*)node6);
        }
    }

    printf ("prepare %uth sample: 3 nodes who has 3 child nodes\n", ++sample_number);
    {
        for (unsigned int i = 1; i <= 3; ++i)
        {
            CPD_STAT_NODE* node1 = NULL_STAT_NODE;
            char name1[16];
            node1 = get_node_from_pool((void*)&node_pool);
            if (node1)
            {
                sprintf(name1, "f1_%u", i);
                cpd_stat_init_node(node1, name1);
                cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);

                for (unsigned int j = 1; j <= 3; ++j)
                {
                    char name2[16];
                    sprintf(name2, "f2_%u_%u", i, j);
                    CPD_STAT_NODE* node2 = get_node_from_pool((void*)&node_pool);
                    if (node2)
                    {
                        cpd_stat_init_node(node2, name2);
                        cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
                    }
                }
            }
        }
    }

    printf ("prepare %uth sample: 1 node + 1 child node + 1 grandchild node\n", ++sample_number);
    {
        CPD_STAT_NODE* node1 = NULL_STAT_NODE;
        CPD_STAT_NODE* node2 = NULL_STAT_NODE;
        CPD_STAT_NODE* node3 = NULL_STAT_NODE;

        node1 = get_node_from_pool((void*)&node_pool);
        if (node1)
        {
            cpd_stat_init_node(node1, "f1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);
        }

        node2 = get_node_from_pool((void*)&node_pool);
        if (node2)
        {
            cpd_stat_init_node(node2, "f2_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);
        }

        node3 = get_node_from_pool((void*)&node_pool);
        if (node3)
        {
            cpd_stat_init_node(node3, "f2_1_1_1");
            cpd_stat_add_child((CPD_STAT_HEAD*)node2, (CPD_STAT_HEAD*)node3);
        }
    }

    printf ("prepare %uth sample: 3 nodes who has 3 child nodes who has 3 child nodes\n", ++sample_number);
    {
        for (unsigned int i = 1; i <= 3; ++i)
        {
            CPD_STAT_NODE* node1 = NULL_STAT_NODE;
            char name1[16];
            node1 = get_node_from_pool((void*)&node_pool);
            if (node1)
            {
                sprintf(name1, "f1_%u", i);
                cpd_stat_init_node(node1, name1);
                cpd_stat_add_child((CPD_STAT_HEAD*)&sample_array[sample_number - 1], (CPD_STAT_HEAD*)node1);

                for (unsigned int j = 1; j <= 3; ++j)
                {
                    char name2[16];
                    CPD_STAT_NODE* node2 = get_node_from_pool((void*)&node_pool);
                    if (node2)
                    {
                        sprintf(name2, "f2_%u_%u", i, j);
                        cpd_stat_init_node(node2, name2);
                        cpd_stat_add_child((CPD_STAT_HEAD*)node1, (CPD_STAT_HEAD*)node2);

                        for (unsigned int k = 1; k <= 3; ++k)
                        {
                            char name3[16];
                            CPD_STAT_NODE* node3 = get_node_from_pool((void*)&node_pool);
                            if (node3)
                            {
                                sprintf(name3, "f3_%u_%u_%u", i, j, k);
                                cpd_stat_init_node(node3, name3);
                                cpd_stat_add_child((CPD_STAT_HEAD*)node2, (CPD_STAT_HEAD*)node3);
                            }
                        }
                    }
                }
            }
        }
    }
}

CPD_STAT_ROOT dyn_sample;

void cpd_stat_test_prepare_dyn_sample()
{
	cpd_stat_init_root(&dyn_sample);
}

CPD_STAT_ROOT stage_node_pool;

void cpd_stat_test_prepare_stage_node_pool()
{
	cpd_stat_init_root(&stage_node_pool);
}

int main(int argc, char *argv[])
{
    printf ("================================\n");
    printf ("prepare node pool\n");
    cpd_stat_test_prepare_node_pool();
    printf ("travel all node pool: count number\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        unsigned int count = 0;
        cpd_stat_travel(&node_pool, option, (CPD_STAT_VISIT)count_node, (CPD_STAT_VISIT)0, (void*)&count);
        printf ("%u nodes are prepared in node pool\n", count);
    }

    printf ("================================\n");
    printf ("prepare sample arrays\n");
    cpd_stat_test_prepare_sample_array();
    printf ("travel all node pool: count number\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        unsigned int count = 0;
        cpd_stat_travel(&node_pool, option, (CPD_STAT_VISIT)count_node, (CPD_STAT_VISIT)0, (void*)&count);
        printf ("%u nodes left in node pool\n", count);
    }

    printf ("================================\n");
    printf ("travel all sample arrays: pre-order print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        for (unsigned int i = 0; i < sample_number; ++i)
        {
            printf("-----------%uth sample------------\n", (i + 1));
            cpd_stat_travel(&sample_array[i], option, print_name, (CPD_STAT_VISIT)0, (void*)0);
        }
    }

    printf ("================================\n");
    printf ("travel all sample arrays: post-order print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        for (unsigned int i = 0; i < sample_number; ++i)
        {
            printf("-----------%uth sample------------\n", (i + 1));
            cpd_stat_travel(&sample_array[i], option, (CPD_STAT_VISIT)0, print_name, (void*)0);
        }
    }
    printf ("================================\n");
    printf ("iterately travel all sample arrays: print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        option.flag = 0;
        option.flag |= (1 << CPD_STAT_TRAVEL_FLAG_VISITED_CHECK_BIT);

        for (unsigned int i = 0; i < sample_number; ++i)
        {
            unsigned int round = 1;
            CPD_STAT_TRAVEL_RESULT result = CPD_STAT_TRAVEL_MIDWAY;
            printf("-----------%uth sample------------\n", (i + 1));
            do
            {
                unsigned int nodes_per_round = 1;
                printf ("round %u:\n", round++);
                result = cpd_stat_travel(&sample_array[i], option, 
                    iterate_print_name, iterate_print_name_post, (void*)&nodes_per_round);
            } while (result != CPD_STAT_TRAVEL_END);
        }
    }
    printf ("================================\n");
    printf ("iterately travel all sample arrays 2nd time without unmark visited: print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        option.flag = 0;
        option.flag |= (1 << CPD_STAT_TRAVEL_FLAG_VISITED_CHECK_BIT);

        for (unsigned int i = 0; i < sample_number; ++i)
        {
            unsigned int round = 1;
            CPD_STAT_TRAVEL_RESULT result = CPD_STAT_TRAVEL_MIDWAY;
            printf("-----------%uth sample------------\n", (i + 1));
            do
            {
                unsigned int nodes_per_round = 1;
                printf ("round %u:\n", round++);
                result = cpd_stat_travel(&sample_array[i], option, 
                    iterate_print_name, iterate_print_name_post, (void*)&nodes_per_round);
            } while (result != CPD_STAT_TRAVEL_END);
        }
    }

    printf ("================================\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        for (unsigned int i = 0; i < sample_number; ++i)
        {
            printf("-----------unmark visited for %uth sample------------\n", (i + 1));
            cpd_stat_unmark_visited(&sample_array[i]);
        }
    }

    printf ("================================\n");
    printf ("iterately travel all sample arrays 3rd time with unmark visited: print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        option.flag = 0;
        option.flag |= (1 << CPD_STAT_TRAVEL_FLAG_VISITED_CHECK_BIT);

        for (unsigned int i = 0; i < sample_number; ++i)
        {
            unsigned int round = 1;
            CPD_STAT_TRAVEL_RESULT result = CPD_STAT_TRAVEL_MIDWAY;
            printf("-----------%uth sample------------\n", (i + 1));
            do
            {
                unsigned int nodes_per_round = 1;
                printf ("round %u:\n", round++);
                result = cpd_stat_travel(&sample_array[i], option, 
                    iterate_print_name, iterate_print_name_post, (void*)&nodes_per_round);
            } while (result != CPD_STAT_TRAVEL_END);
        }
    }

    printf ("================================\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        for (unsigned int i = 0; i < sample_number; ++i)
        {
            printf("-----------unmark visited for %uth sample------------\n", (i + 1));
            cpd_stat_unmark_visited(&sample_array[i]);
        }
    }

    printf ("================================\n");
	printf ("prepare dynamic sample\n");
    {
        CPD_STAT_NODE * node = NULL_STAT_NODE;

        for (unsigned int i = 1; i <= 3; ++i)
		{
        	for (unsigned int j = 1; j <= 3; ++j)
			{
            	for (unsigned int k = 1; k <= 3; ++k)
				{
            		char name[256];
            		sprintf(name, "f1_%u.f2_%u_%u.f3_%u_%u_%u", i, i, j, i, j, k);
        			node = cpd_stat_add_path(&dyn_sample, name, get_node_from_pool, (void*)&node_pool);
        			if (!node)
        			{
        				printf("fail to add %s\n", name);
        			}
				}
			}
		}
	}

    printf ("================================\n");
    printf ("travel all node pool: count number\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        unsigned int count = 0;
        cpd_stat_travel(&node_pool, option, (CPD_STAT_VISIT)count_node, (CPD_STAT_VISIT)0, (void*)&count);
        printf ("%u nodes left in node pool\n", count);
    }

    printf ("================================\n");
    printf ("travel dynamic sample: pre-order print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        cpd_stat_travel(&dyn_sample, option, print_name, (CPD_STAT_VISIT)0, (void*)0);
    }

    printf ("================================\n");
    printf ("prepare stage pool\n");
    {
	    cpd_stat_test_prepare_stage_node_pool();
	}

    printf ("================================\n");
    printf ("delete child_head node of dynamic sample, put into stage pool\n");
    {
		CPD_STAT_HEAD * del = NULL_STAT_HEAD;
		del = cpd_stat_del_family(dyn_sample.head.child_head);
		if (del)
		{
		    cpd_stat_add_child((CPD_STAT_HEAD*)&(stage_node_pool), del);
		}
	}

    printf ("================================\n");
    printf ("travel stage pool: pre-order print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        cpd_stat_travel(&stage_node_pool, option, print_name, (CPD_STAT_VISIT)0, (void*)0);
    }

    printf ("================================\n");
    printf ("travel dynamic sample: pre-order print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        cpd_stat_travel(&dyn_sample, option, print_name, (CPD_STAT_VISIT)0, (void*)0);
    }

    printf ("================================\n");
    printf ("return stage pool's node to node pool\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
		
        unsigned int round = 1;
        CPD_STAT_TRAVEL_RESULT result = CPD_STAT_TRAVEL_MIDWAY;
        do
        {
            printf ("round %u:\n", round++);
        	result = cpd_stat_travel(&stage_node_pool, option, 
				(CPD_STAT_VISIT)0, iterate_remove_node, (void*)&node_pool);
        } while (result != CPD_STAT_TRAVEL_END);
    }

    printf ("================================\n");
    printf ("travel stage pool: pre-order print name\n");
    {
        CPD_STAT_TRAVEL_OPTION option;
        cpd_stat_travel(&stage_node_pool, option, print_name, (CPD_STAT_VISIT)0, (void*)0);
    }

    return 0;
}

#if 0
int main(int argc, char *argv[])
{
    printf("test name parser\n");
    {
        CPD_STAT_NAME names[16];
        unsigned int count = 0;

        count = cpd_stat_parse_path("", names);
        for (unsigned int i = 0; i < count; ++i)
        {
            printf("%s\n", names[i].str);
        }
        printf("==============\n");
    }
    {
        CPD_STAT_NAME names[16];
        unsigned int count = 0;

        count = cpd_stat_parse_path("1st", names);
        for (unsigned int i = 0; i < count; ++i)
        {
            printf("%s\n", names[i].str);
        }
        printf("==============\n");
    }
    {
        CPD_STAT_NAME names[16];
        unsigned int count = 0;

        count = cpd_stat_parse_path("1st.2nd", names);
        for (unsigned int i = 0; i < count; ++i)
        {
            printf("%s\n", names[i].str);
        }
        printf("==============\n");
    }

    {
        CPD_STAT_NAME names[16];
        unsigned int count = 0;

        count = cpd_stat_parse_path("1st.2nd.3rd", names);
        for (unsigned int i = 0; i < count; ++i)
        {
            printf("%s\n", names[i].str);
        }
        printf("==============\n");
    }

    {
        CPD_STAT_NAME names[16];
        unsigned int count = 0;

        count = cpd_stat_parse_path("1st.2nd.3rd.4.5.6.7.8.9.10abcdefghijklmnopqrstuvwxyz.11.12.13.14.15.16.17.18", names);
        for (unsigned int i = 0; i < count; ++i)
        {
            printf("%s\n", names[i].str);
        }
        printf("==============\n");
    }

    return 0;
}
#endif
