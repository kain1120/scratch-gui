#ifndef CPD_STAT_TEST_H
#define CPD_STAT_TEST_H

#include "../stat/cpd_stat.h"

unsigned long long cpd_stat_calc_mem_size();

void cpd_stat_init_root(CPD_STAT_ROOT* root);
void cpd_stat_init_node(CPD_STAT_NODE* node, const char* name);
bool cpd_stat_add_child(CPD_STAT_HEAD* parent, CPD_STAT_HEAD* child);

CPD_STAT_HEAD* cpd_stat_del_family(CPD_STAT_HEAD* child);
CPD_STAT_HEAD* cpd_stat_del_bachelor(CPD_STAT_HEAD* self);

unsigned int cpd_stat_parse_path (const char * path, CPD_STAT_NAME names[]);
CPD_STAT_NODE* cpd_stat_follow_path (CPD_STAT* stat, const char * path, CPD_STAT_FOLLOW_PATH_OPTION option);
CPD_STAT_NODE* cpd_stat_add_path_outside_node_pool (CPD_STAT* stat, const char * path, CPD_STAT_GET_UNINIT_NODE get_node, void* extra);

CPD_STAT_TRAVEL_RESULT
cpd_stat_travel_staging (CPD_STAT* stat, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra);

CPD_STAT_TRAVEL_RESULT
cpd_stat_travel_pool (CPD_STAT* stat, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra);
#endif
