#ifndef CPD_STAT_H
#define CPD_STAT_H

#define MAX_CPD_STAT_NAME_LENGTH 16
#define MAX_CPD_STAT_PATH_NAME_LENGTH 256
#define MAX_CPD_STAT_PATH_NAME_DEPTH 16

#define MAX_CPD_STAT_NODE_NUMBER 1024
#define MAX_CPD_STAT_COUNTER_NUMBER 512 
#define MAX_CPD_STAT_GAUGE_NUMBER 512 

typedef enum { false, true } bool;
#define NULL_STAT_HEAD ((CPD_STAT_HEAD*) 0)
#define NULL_STAT_NODE ((CPD_STAT_NODE*) 0)

typedef enum _CPD_STAT_TYPE
{
    CPD_STAT_TYPE_NONE    = 0,
    CPD_STAT_TYPE_COUNTER = 1,
    CPD_STAT_TYPE_GAUGE   = 2,
} CPD_STAT_TYPE;

typedef enum _CPD_STAT_HEAD_TYPE
{
    CPD_STAT_HEAD_TYPE_ROOT    = 0,
    CPD_STAT_HEAD_TYPE_NODE    = 1,
} CPD_STAT_HEAD_TYPE;

typedef struct _CPD_STAT_HEAD
{
    struct _CPD_STAT_HEAD* parent;
    struct _CPD_STAT_HEAD* child_head;
    struct _CPD_STAT_HEAD* child_tail;
    struct _CPD_STAT_HEAD* next;
    struct _CPD_STAT_HEAD* prev;
    CPD_STAT_HEAD_TYPE type;
	unsigned int       ref;
} CPD_STAT_HEAD;

typedef struct _CPD_STAT_NAME
{
    char str[MAX_CPD_STAT_NAME_LENGTH];
} CPD_STAT_NAME;

typedef struct _CPD_STAT_STORE CPD_STAT_STORE;
typedef struct _CPD_STAT CPD_STAT;

typedef bool (*CPD_STAT_PUT_STORE)(CPD_STAT_STORE* store, void* extra);
typedef CPD_STAT_STORE* (*CPD_STAT_GET_STORE)(CPD_STAT* stat);
typedef struct _CPD_STAT_STORE
{
	union
	{
    	CPD_STAT_HEAD* parent;
		struct _CPD_STAT_STORE* next;
	};
	unsigned int   ref;
	CPD_STAT_PUT_STORE put;
} CPD_STAT_STORE;

#define CPD_STAT_NODE_FLAG_VISITED  0x00000001
#define CPD_STAT_NODE_FLAG_OBSOLETE 0x00000002
#define CPD_STAT_NODE_FLAG_VISITED_BIT   (0) 
#define CPD_STAT_NODE_FLAG_OBSOLETE_BIT  (1) 
typedef unsigned int CPD_STAT_NODE_FLAG;

typedef struct _CPD_STAT_COMMON
{
    CPD_STAT_TYPE type;
    CPD_STAT_NAME name;
    CPD_STAT_NODE_FLAG flag;
	CPD_STAT_STORE *store;
} CPD_STAT_COMMON;

typedef struct _CPD_STAT_NODE
{
    CPD_STAT_HEAD head;
    CPD_STAT_COMMON stat;
} CPD_STAT_NODE;

typedef struct _CPD_STAT CPD_STAT;

typedef struct _CPD_STAT_ROOT
{
    CPD_STAT_HEAD head;
	CPD_STAT*     owner;
} CPD_STAT_ROOT;

typedef struct _CPD_STAT
{
    CPD_STAT_ROOT root;
	CPD_STAT_ROOT node_pool;
	CPD_STAT_ROOT staging_node_pool;
	CPD_STAT_STORE counter_pool;
	CPD_STAT_STORE gauge_pool;
	void* mem;
} CPD_STAT;


typedef struct _CPD_STAT_COUNTER
{
	CPD_STAT_STORE meta; 
    unsigned long value;
} CPD_STAT_COUNTER;

typedef struct _CPD_STAT_GAUGE
{
	CPD_STAT_STORE meta; 
    long value;
} CPD_STAT_GAUGE;

#define CPD_STAT_FOLLOW_PATH_FLAG_ADD_UNFOUND    0x00000001
#define CPD_STAT_FOLLOW_PATH_FLAG_ADD_UNFOUND_BIT    (0) 
typedef unsigned int CPD_STAT_FOLLOW_PATH_FLAG;
typedef CPD_STAT_NODE* (*CPD_STAT_GET_UNINIT_NODE)(void* extra);

typedef struct _CPD_STAT_FOLLOW_PATH_OPTION
{
    CPD_STAT_FOLLOW_PATH_FLAG flag;
    CPD_STAT_GET_UNINIT_NODE  get_node;
    void* extra;
} CPD_STAT_FOLLOW_PATH_OPTION;

#define CPD_STAT_TRAVEL_FLAG_VISITED_CHECK  0x00000001
#define CPD_STAT_TRAVEL_FLAG_VISITED_CHECK_BIT  (0) 
typedef unsigned int CPD_STAT_TRAVEL_FLAG;

typedef struct _CPD_STAT_TRAVEL_OPTION
{
    CPD_STAT_TRAVEL_FLAG flag;
} CPD_STAT_TRAVEL_OPTION;

typedef enum _CPD_STAT_VISIT_RESULT
{
    CPD_STAT_VISIT_STOP     = 0,
    CPD_STAT_VISIT_CONTINUE = 1,
    CPD_STAT_VISIT_BYPASS   = 2,
} CPD_STAT_VISIT_RESULT;

typedef enum _CPD_STAT_TRAVEL_RESULT
{
    CPD_STAT_TRAVEL_MIDWAY     = 0,
    CPD_STAT_TRAVEL_END        = 1,
} CPD_STAT_TRAVEL_RESULT;

typedef CPD_STAT_VISIT_RESULT (*CPD_STAT_VISIT)(CPD_STAT_NODE* node, void* extra);

bool cpd_stat_is_root(CPD_STAT_HEAD* head);

bool cpd_stat_check_node(CPD_STAT_NODE* node);
void cpd_stat_mark_node_visited(CPD_STAT_NODE* node);
void cpd_stat_unmark_node_visited(CPD_STAT_NODE* node);
bool cpd_stat_is_node_visited(CPD_STAT_NODE* node);
void cpd_stat_mark_node_obsolete(CPD_STAT_NODE* node);
void cpd_stat_unmark_node_obsolete(CPD_STAT_NODE* node);
bool cpd_stat_is_node_obsolete(CPD_STAT_NODE* node);
bool cpd_stat_has_child(CPD_STAT_NODE* node);
bool cpd_stat_recycle_node (CPD_STAT_NODE* node);

typedef void* (*CPD_STAT_ALLOC)(unsigned long long size);
typedef void (*CPD_STAT_FREE)(void* mem, unsigned long long size);
bool cpd_stat_init(CPD_STAT* stat, CPD_STAT_ALLOC alloc);
void cpd_stat_release(CPD_STAT* stat, CPD_STAT_FREE free);

bool cpd_stat_verify_path (CPD_STAT_NODE* node, const char * path);
void cpd_stat_get_path(CPD_STAT_NODE* node, char path[]);
void cpd_stat_get_full_name(CPD_STAT_STORE* store, char path[]);

CPD_STAT_NODE* cpd_stat_search_path (CPD_STAT* stat, const char * path);
CPD_STAT_NODE* cpd_stat_add_path (CPD_STAT* stat, const char * path);
bool cpd_stat_trunc_path (CPD_STAT* stat, const char * path);
bool cpd_stat_reclaim (CPD_STAT* stat, unsigned int count);

CPD_STAT_TRAVEL_RESULT 
cpd_stat_travel (CPD_STAT* stat, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra);

CPD_STAT_TRAVEL_RESULT
cpd_stat_unmark_visited(CPD_STAT* stat);

CPD_STAT_STORE* cpd_stat_get_counter(CPD_STAT* stat);
CPD_STAT_STORE* cpd_stat_get_gauge(CPD_STAT* stat);

void cpd_stat_init_counter(CPD_STAT_COUNTER* counter);
void cpd_stat_init_gauge(CPD_STAT_GAUGE* gauge);

// return NULL means not successful
CPD_STAT_COUNTER* cpd_stat_attach_counter(CPD_STAT* stat, const char * path);
CPD_STAT_GAUGE* cpd_stat_attach_gauge(CPD_STAT* stat, const char * path);
bool cpd_stat_detach_store(CPD_STAT_STORE* store);

void cpd_stat_increase_counter(CPD_STAT_COUNTER* counter, unsigned int number);
void cpd_stat_increase_gauge(CPD_STAT_GAUGE* gauge, unsigned int number);
void cpd_stat_decrease_gauge(CPD_STAT_GAUGE* gauge, unsigned int number);

#endif

