#include "cpd_stat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

CPD_STAT_COUNTER* get_counter(void)
{
    return (CPD_STAT_COUNTER*)0;
}

CPD_STAT_GAUGE* get_gauge(void)
{
    return (CPD_STAT_GAUGE*)0;
}

void cpd_stat_init_root(CPD_STAT_ROOT* root)
{
    assert(root);
    root->head.parent = NULL_STAT_HEAD;
    root->head.child_head = NULL_STAT_HEAD;
    root->head.child_tail = NULL_STAT_HEAD;
    root->head.next = NULL_STAT_HEAD;
    root->head.prev = NULL_STAT_HEAD;
    root->head.type = CPD_STAT_HEAD_TYPE_ROOT;
    root->head.ref  = 0;
	root->owner = NULL;
}

bool cpd_stat_is_root(CPD_STAT_HEAD* head)
{
    return ((head->type) == CPD_STAT_HEAD_TYPE_ROOT);
}

void cpd_stat_init_node(CPD_STAT_NODE* node, const char* name)
{
    assert(node && name);
    node->head.parent = NULL_STAT_HEAD;
    node->head.child_head = NULL_STAT_HEAD;
    node->head.child_tail = NULL_STAT_HEAD;
    node->head.next = NULL_STAT_HEAD;
    node->head.prev = NULL_STAT_HEAD;
    node->head.type = CPD_STAT_HEAD_TYPE_NODE;
    node->head.ref  = 0;
    strncpy(node->stat.name.str, name, MAX_CPD_STAT_NAME_LENGTH);
    node->stat.flag = 0;
}

bool cpd_stat_check_node(CPD_STAT_NODE* node)
{
    assert(node);
    if (!strlen(node->stat.name.str))
    {
        return false;
    }
    return true;
}

void cpd_stat_mark_node_visited(CPD_STAT_NODE* node)
{
    assert(node);
    node->stat.flag |= CPD_STAT_NODE_FLAG_VISITED;
}

void cpd_stat_unmark_node_visited(CPD_STAT_NODE* node)
{
    assert(node);
    node->stat.flag &= ~CPD_STAT_NODE_FLAG_VISITED;
}

bool cpd_stat_is_node_visited(CPD_STAT_NODE* node)
{
    assert(node);
    return (node->stat.flag & CPD_STAT_NODE_FLAG_VISITED);
}

void cpd_stat_mark_node_obsolete(CPD_STAT_NODE* node)
{
    assert(node);
    node->stat.flag |= CPD_STAT_NODE_FLAG_OBSOLETE;
}

void cpd_stat_unmark_node_obsolete(CPD_STAT_NODE* node)
{
    assert(node);
    node->stat.flag &= ~CPD_STAT_NODE_FLAG_OBSOLETE;
}

bool cpd_stat_is_node_obsolete(CPD_STAT_NODE* node)
{
    assert(node);
    return (node->stat.flag & CPD_STAT_NODE_FLAG_OBSOLETE);
}

bool cpd_stat_has_child(CPD_STAT_NODE* node)
{
    assert(node);
    return (node->head.child_head != 0);
}

bool cpd_stat_add_child(CPD_STAT_HEAD* parent, CPD_STAT_HEAD* child)
{
    assert(parent && child);
    assert(!child->parent 
        && !child->next && !child->prev);
    assert((parent->child_head && parent->child_tail)
        ||(!parent->child_head && !parent->child_tail));
    assert((child->child_head && child->child_tail)
        ||(!child->child_head && !child->child_tail));

    if (!parent->child_head && !parent->child_tail)
    {
        child->prev = child;
        child->next = child;

        parent->child_head = child;
        parent->child_tail = child;
    }
    else
    {
        assert(parent->child_head->prev 
            == parent->child_tail);
        assert(parent->child_tail->next
            == parent->child_head);

        child->prev = parent->child_tail;
        child->next = parent->child_head;

        parent->child_tail->next = child;
        parent->child_head->prev = child;

        parent->child_tail = child;
    }

    child->parent = parent;
	parent->ref++;

    return true;
}

CPD_STAT_HEAD* cpd_stat_del_family(CPD_STAT_HEAD* child)
{
    assert(child);
    CPD_STAT_HEAD* parent = child->parent;
    
    if (!parent)
    {
        return NULL_STAT_HEAD;
    }

    if ((parent->child_head == child)
        && (parent->child_tail == child))
    {
        assert(child->next == child);
        assert(child->prev == child);
        parent->child_tail = parent->child_head = NULL_STAT_HEAD;
    }
    else
    {
        if (parent->child_head == child)
        {
            assert(child->next != child);
            assert(child->prev == parent->child_tail);
            parent->child_head = child->next;
        }
        else if (parent->child_tail == child)
        {
            assert(child->prev != child);
            assert(child->next == parent->child_head);
            parent->child_tail = child->prev;
        }
        else
        {
            assert(child->next != child);
            assert(child->prev != child);
        }

        child->next->prev = child->prev;
        child->prev->next = child->next;
    }

    child->next = NULL_STAT_HEAD;
    child->prev = NULL_STAT_HEAD;

    child->parent = NULL_STAT_HEAD;
	if (parent->ref)
	{
		parent->ref--;
	}

    return child;
}

CPD_STAT_HEAD* cpd_stat_del_bachelor(CPD_STAT_HEAD* self)
{
    assert(self);
    CPD_STAT_HEAD* parent = self->parent;
    
    if (!parent || self->ref)
    {
        return NULL_STAT_HEAD;
    }

    return cpd_stat_del_family(self);
}

unsigned long long cpd_stat_calc_mem_size()
{
    unsigned long long size = 0;
	size = MAX_CPD_STAT_NODE_NUMBER * sizeof(CPD_STAT_NODE);
	size += MAX_CPD_STAT_COUNTER_NUMBER * sizeof(CPD_STAT_COUNTER);
	size += MAX_CPD_STAT_GAUGE_NUMBER * sizeof(CPD_STAT_GAUGE);
	size += 64;

	return size;
}

CPD_STAT* cpd_stat_get_node_owner(CPD_STAT_NODE* node)
{
    CPD_STAT_HEAD* parent = (CPD_STAT_HEAD*)node;
	CPD_STAT* owner = NULL;

    assert(node);
    while (!cpd_stat_is_root(parent))
    {
        parent = parent->parent;
    }

	if (parent && cpd_stat_is_root(parent))
	{
		owner = ((CPD_STAT_ROOT*)parent)->owner;
	}

	return owner;
}

CPD_STAT* cpd_stat_get_store_owner(CPD_STAT_STORE* store)
{
    CPD_STAT_HEAD* parent = NULL_STAT_HEAD;

	assert(store);
	parent = store->parent;
	assert(parent);

	return cpd_stat_get_node_owner((CPD_STAT_NODE*)parent);
}

void cpd_stat_init_store(CPD_STAT_STORE* store)
{
    assert(store);
	store->parent = NULL_STAT_HEAD;
	store->ref = 0;
}

void cpd_stat_push_store(CPD_STAT_STORE* stack, CPD_STAT_STORE* store)
{
    assert(stack && store);
	
	store->next = NULL;
	store->ref = 0;

	if (stack->next)
	{
		store->next = stack->next;
	}

	stack->next = store;
	stack->ref++;
}

CPD_STAT_STORE* cpd_stat_pop_store(CPD_STAT_STORE* stack)
{
	CPD_STAT_STORE* store = NULL;
    assert(stack);

	if (stack->next)
	{
		store = stack->next;
		stack->next = store->next;
		store->next = NULL;

		if (stack->ref)
		{
			stack->ref--;
		}
	}

	return store;
}

CPD_STAT_STORE* cpd_stat_get_counter(CPD_STAT* stat)
{
	CPD_STAT_STORE* store;
    assert(stat);

	store = cpd_stat_pop_store(&(stat->counter_pool));

	if (store)
	{
		cpd_stat_init_counter((CPD_STAT_COUNTER*)store);
	}

	return store;
}

CPD_STAT_STORE* cpd_stat_get_gauge(CPD_STAT* stat)
{
	CPD_STAT_STORE* store;
    assert(stat);

	store = cpd_stat_pop_store(&(stat->gauge_pool));

	if (store)
	{
		cpd_stat_init_counter((CPD_STAT_COUNTER*)store);
	}


	return store;
}

bool cpd_stat_put_store(CPD_STAT_STORE* store)
{
	assert(store);

	if (store->ref)
	{
	    return false;
	}

	return true;
}

bool cpd_stat_put_counter(CPD_STAT_STORE* store, void* extra)
{
	bool result = false;
	CPD_STAT* stat = (CPD_STAT*)extra;
	CPD_STAT_COUNTER* counter = (CPD_STAT_COUNTER*)store;

	assert(counter && stat);

    result = cpd_stat_put_store(&(counter->meta));
	if (result)
	{
	    cpd_stat_push_store(&(stat->counter_pool), &(counter->meta));
		result = true;
	}

	return result;
}

void cpd_stat_init_counter(CPD_STAT_COUNTER* counter)
{
    assert(counter);
	cpd_stat_init_store(&counter->meta);
	counter->meta.put = cpd_stat_put_counter;
	counter->value = 0;
}

bool cpd_stat_put_gauge(CPD_STAT_STORE* store, void* extra)
{
	bool result = false;
	CPD_STAT* stat = (CPD_STAT*)extra;
	CPD_STAT_GAUGE* gauge = (CPD_STAT_GAUGE*)store;

	assert(gauge && stat);

    result = cpd_stat_put_store(&(gauge->meta));
	if (result)
	{
	    cpd_stat_push_store(&(stat->gauge_pool), &(gauge->meta));
		result = true;
	}

	return result;
}

void cpd_stat_init_gauge(CPD_STAT_GAUGE* gauge)
{
    assert(gauge);
	cpd_stat_init_store(&gauge->meta);
	gauge->meta.put = cpd_stat_put_gauge;
	gauge->value = 0;
}


CPD_STAT_STORE* cpd_stat_direct_attach_store(CPD_STAT* stat, const char * path, CPD_STAT_STORE* store)
{
	CPD_STAT_NODE* node = NULL_STAT_NODE;

	assert(stat && path && store);

	node = cpd_stat_search_path(stat, path);

	if (node && !cpd_stat_has_child(node)) 
	{
		if (!node->stat.store)
		{
			node->stat.store = store; 
			store->ref++;

			store->parent = &(node->head);
			node->head.ref++;
			return store;
		}
		else
		{
			assert(node->stat.store->parent == &(node->head));
			if (node->stat.store->put == store->put)
			{
				node->stat.store->ref++;
				return node->stat.store;
			}
		}
	}

	return (CPD_STAT_STORE*)0;
}

CPD_STAT_STORE* cpd_stat_attach_store (CPD_STAT* stat, const char * path,
			   CPD_STAT_GET_STORE get, CPD_STAT_PUT_STORE put)
{
	CPD_STAT_STORE* store = NULL;
	CPD_STAT_STORE* store2 = NULL;
	bool result = false;
    assert(stat && path && get && put);

	store = (CPD_STAT_STORE*)get(stat);

	if (store)
	{
	    store2 = cpd_stat_direct_attach_store(stat, path, store);
	}

	if (!store2 || store2 != store)
	{
	    result = put(store, (void*)stat);
		assert(result);
	}

	return store2; 
}

CPD_STAT_COUNTER* cpd_stat_attach_counter(CPD_STAT* stat, const char * path)
{
    CPD_STAT_STORE* store = NULL;	
	store = cpd_stat_attach_store(stat, path, cpd_stat_get_counter, cpd_stat_put_counter);
	return (CPD_STAT_COUNTER*)store; 
}

CPD_STAT_GAUGE* cpd_stat_attach_gauge(CPD_STAT* stat, const char * path)
{
    CPD_STAT_STORE* store = NULL;	
	store = cpd_stat_attach_store(stat, path, cpd_stat_get_gauge, cpd_stat_put_gauge);
	return (CPD_STAT_GAUGE*)store; 
}

CPD_STAT_STORE* cpd_stat_detach_store_from_node (CPD_STAT_STORE* store)
{
	CPD_STAT_HEAD* parent = NULL_STAT_HEAD;
	assert(store);

	if (store->ref)
	{
	    store->ref--;
	}

	if (store->ref)
	{
		return (CPD_STAT_STORE*)NULL;
	}

	parent = store->parent;

	if (parent)
	{
        CPD_STAT_NODE* node = (CPD_STAT_NODE*)parent;	
		assert(node->stat.store == store);
		assert(parent->ref);

		node->stat.store = (CPD_STAT_STORE*)0;
		parent->ref--;

	}

	store->parent = NULL_STAT_HEAD;

	return store;
}

bool cpd_stat_detach_store(CPD_STAT_STORE* store)
{
	bool result = false;
	CPD_STAT* owner = NULL;
	CPD_STAT_STORE* store2 = NULL;

	assert(store);
	owner = cpd_stat_get_store_owner(store);
	assert(owner);

	store2 = cpd_stat_detach_store_from_node(store);

	if (store2 && store2->put)
	{
		result = store2->put(store2, (void*)owner);
	}

	return result;
}

void cpd_stat_increase_counter(CPD_STAT_COUNTER* counter, unsigned int number)
{
    assert(counter);
	counter->value += number;
}

void cpd_stat_increase_gauge(CPD_STAT_GAUGE* gauge, unsigned int number)
{
    assert(gauge);
	gauge->value += number;
}

void cpd_stat_decrease_gauge(CPD_STAT_GAUGE* gauge, unsigned int number)
{
    assert(gauge);
	if (gauge->value >= number)
	{
	    gauge->value -= number;
	}
	else
	{
	    gauge->value = 0;
	}
}


bool cpd_stat_init(CPD_STAT* stat, CPD_STAT_ALLOC alloc)
{
    void * mem = NULL;
	unsigned long long size = cpd_stat_calc_mem_size();
	assert(stat);

	cpd_stat_init_root(&(stat->root));
	cpd_stat_init_root(&(stat->node_pool));
	cpd_stat_init_root(&(stat->staging_node_pool));

	stat->root.owner = stat;
	stat->node_pool.owner = stat;
	stat->staging_node_pool.owner = stat;

	cpd_stat_init_store(&(stat->counter_pool));
	cpd_stat_init_store(&(stat->gauge_pool));

	if (alloc)
	{
		mem = (void*)alloc(size);
	}

	if (mem)
	{
		stat->mem = mem;
		for (unsigned int i = 0; i < MAX_CPD_STAT_NODE_NUMBER; ++i)
		{
		    cpd_stat_init_node((CPD_STAT_NODE*)mem, "");
			cpd_stat_add_child(&(stat->node_pool.head), (CPD_STAT_HEAD*)mem);
			mem += sizeof(CPD_STAT_NODE);
		}

		for (unsigned int i = 0; i < MAX_CPD_STAT_COUNTER_NUMBER; ++i)
		{
		    cpd_stat_init_counter((CPD_STAT_COUNTER*)mem);
			cpd_stat_push_store(&(stat->counter_pool), (CPD_STAT_STORE*)mem);
			mem += sizeof(CPD_STAT_COUNTER);
		}

		for (unsigned int i = 0; i < MAX_CPD_STAT_GAUGE_NUMBER; ++i)
		{
		    cpd_stat_init_gauge((CPD_STAT_GAUGE*)mem);
			cpd_stat_push_store(&(stat->gauge_pool), (CPD_STAT_STORE*)mem);
			mem += sizeof(CPD_STAT_GAUGE);
		}
	}
	else
	{
		stat->mem = NULL;
	}

	return true;
}

void cpd_stat_release(CPD_STAT* stat, CPD_STAT_FREE free)
{
	unsigned long long size = 0;
    assert(stat);

	if (stat->mem && free)
	{
	    size = cpd_stat_calc_mem_size();
		free(stat->mem, size);

		cpd_stat_init_root(&(stat->root));
		cpd_stat_init_root(&(stat->node_pool));
		cpd_stat_init_root(&(stat->staging_node_pool));
	}
}

unsigned int cpd_stat_parse_path (const char * path, CPD_STAT_NAME names[])
{
    unsigned int char_index = 0;
    unsigned int name_index = 0;
    const char* p = path;
    assert(path);

    if ((*p)== '\0')
    {
        return 0;
    }

    while ((*p) 
        && (name_index < MAX_CPD_STAT_PATH_NAME_DEPTH))
    {
        if ((*p) == '.')
        {
            names[name_index].str[char_index++] = '\0'; 
            ++name_index;
            char_index = 0;
        }
        else
        {
            if ((char_index + 1) < MAX_CPD_STAT_NAME_LENGTH)
            {
                names[name_index].str[char_index++] = *p;
            }
        }
        ++p;
    }

    names[name_index].str[char_index++] = '\0'; 
    return (name_index + 1);
}

bool cpd_stat_match_name (CPD_STAT_NODE* node, CPD_STAT_NAME*  name)
{
    assert (node && name && name->str[0]);
    if (!strcmp(node->stat.name.str, name->str))
    {
        return true;
    }
    return false;
}

void cpd_stat_get_path(CPD_STAT_NODE* node, char path[])
{
    CPD_STAT_HEAD* parent = (CPD_STAT_HEAD*)node;
    CPD_STAT_HEAD* nodes[MAX_CPD_STAT_PATH_NAME_DEPTH] = {0};
    unsigned int i = 0;
    unsigned int j = 0;

    assert(node);
    while (!cpd_stat_is_root(parent) && (i < MAX_CPD_STAT_PATH_NAME_DEPTH))
    {
        //printf("%s.", ((CPD_STAT_NODE*)parent)->stat.name.str);
        nodes[i++] = parent;
        parent = parent->parent;
    }

    for (j = i - 1; j > 0; --j)
    {
        strcat(path, ((CPD_STAT_NODE*)nodes[j])->stat.name.str);
        strcat(path, ".");
    }

    strcat(path, ((CPD_STAT_NODE*)nodes[j])->stat.name.str);
}

void cpd_stat_get_full_name(CPD_STAT_STORE* store, char path[])
{
	CPD_STAT_HEAD* parent = NULL_STAT_HEAD;

	assert(store);
	parent = store->parent;

	if (parent)
	{
		cpd_stat_get_path((CPD_STAT_NODE*)parent, path);
	}
}

CPD_STAT_NODE* cpd_stat_follow_path (CPD_STAT* stat, const char * path, CPD_STAT_FOLLOW_PATH_OPTION option)
{
    CPD_STAT_NAME names[MAX_CPD_STAT_PATH_NAME_DEPTH];
    CPD_STAT_HEAD* floor = NULL_STAT_HEAD;
    CPD_STAT_HEAD* match = NULL_STAT_HEAD; 
    CPD_STAT_HEAD* last_match = NULL_STAT_HEAD; 
    unsigned int name_num = 0;
    unsigned int name_ind = 0;

    assert(stat&& path);

    last_match = (CPD_STAT_HEAD*)&(stat->root); 

    floor = stat->root.head.child_head;
    if (!(name_num = cpd_stat_parse_path(path, names)) 
        || name_num > MAX_CPD_STAT_PATH_NAME_DEPTH)
    {
        return NULL_STAT_NODE;
    }

    for (; name_ind < name_num && floor; ++name_ind)
    {
        CPD_STAT_HEAD* next = floor; 

        assert(names[name_ind].str[0]);
        match = NULL_STAT_HEAD;

        do
        {
            CPD_STAT_NODE* node = (CPD_STAT_NODE *) next;
            if (cpd_stat_match_name(node, &(names[name_ind])))
            {
                match = next;
                last_match = match;
                break;
            }
            next = next->next;
        } while (next != floor);

        floor = match ? match->child_head : NULL_STAT_HEAD;

		if (!match)
		{
			break;
		}
    }

    if (option.flag & CPD_STAT_FOLLOW_PATH_FLAG_ADD_UNFOUND)
    {
        for (; name_ind < name_num; ++name_ind)
        {
            CPD_STAT_NODE* node = NULL_STAT_NODE;
            if (option.get_node)
            {
                node = option.get_node(option.extra);
            }

            if (node)
            {
                cpd_stat_init_node(node, names[name_ind].str);
                if(cpd_stat_add_child(last_match, (CPD_STAT_HEAD*)node))
                {
                    last_match = (CPD_STAT_HEAD*)node;
                }
            }
			else
			{ 
				break;
			}
        }
    }

    if (name_ind != name_num)
    {
        last_match = NULL_STAT_HEAD;
    }

    return ((CPD_STAT_NODE*) last_match);
}

CPD_STAT_NODE* cpd_stat_search_path (CPD_STAT* stat, const char * path)
{
	CPD_STAT_FOLLOW_PATH_OPTION option;
    option.flag = 0;
    option.get_node = NULL;
    option.extra = (void*)0;

	return cpd_stat_follow_path(stat, path, option);
}

CPD_STAT_NODE* cpd_stat_add_path_outside_node_pool (CPD_STAT* stat, const char * path, CPD_STAT_GET_UNINIT_NODE get_node, void* extra)
{
	CPD_STAT_FOLLOW_PATH_OPTION option;

    option.flag = 0;
    option.get_node = get_node;
    option.extra = extra;
    option.flag |= CPD_STAT_FOLLOW_PATH_FLAG_ADD_UNFOUND;

	return cpd_stat_follow_path(stat, path, option);
}

CPD_STAT_NODE* get_node_inside (void* extra)
{
    CPD_STAT* stat = (CPD_STAT*) extra;
    CPD_STAT_HEAD* head = NULL_STAT_HEAD;

    if (stat && stat->node_pool.head.child_head)
    {
        head = cpd_stat_del_bachelor(stat->node_pool.head.child_head);
    }

    return ((CPD_STAT_NODE*)head);
}

void put_node_inside (CPD_STAT* stat, CPD_STAT_NODE* node)
{
    if (stat && node)
    {
	    cpd_stat_add_child((CPD_STAT_HEAD*)&(stat->node_pool),
		     (CPD_STAT_HEAD*)node);
    }
}

CPD_STAT_NODE* cpd_stat_add_path (CPD_STAT* stat, const char * path)
{
	CPD_STAT_FOLLOW_PATH_OPTION option;

    option.get_node = get_node_inside;
    option.extra = (void*)stat;
    option.flag = 0;
    option.flag |= CPD_STAT_FOLLOW_PATH_FLAG_ADD_UNFOUND;

	return cpd_stat_follow_path(stat, path, option);
}

bool cpd_stat_recycle_node (CPD_STAT_NODE* node)
{
	CPD_STAT* owner = NULL;

	assert(node);
	owner = cpd_stat_get_node_owner(node);
	assert(owner);
	
	if (owner)
	{
		CPD_STAT_HEAD* head = NULL_STAT_HEAD;
		bool is_obsolete = cpd_stat_is_node_obsolete(node);

		if (is_obsolete)
		{
    		head = cpd_stat_del_family((CPD_STAT_HEAD*)node);
		}
		else
		{
    		head = cpd_stat_del_bachelor((CPD_STAT_HEAD*)node);
		}
		
		if (head)
		{
	    	cpd_stat_add_child((CPD_STAT_HEAD*)&(owner->staging_node_pool),
		    	head);
			return true;
		}
	}

	return false;
}

bool cpd_stat_trunc_path (CPD_STAT* stat, const char * path)
{
	CPD_STAT_NODE* node = NULL_STAT_NODE;

	assert(stat);
	
	node = cpd_stat_search_path (stat, path);

	if (node)
	{
		CPD_STAT_HEAD* head = NULL_STAT_HEAD;
    	head = cpd_stat_del_family((CPD_STAT_HEAD*)node);
		
		if (head)
		{
	    	cpd_stat_add_child((CPD_STAT_HEAD*)&(stat->staging_node_pool),
		    	head);
			return true;
		}
	}

	return false;
}

bool cpd_stat_verify_path (CPD_STAT_NODE* node, const char * path)
{
    CPD_STAT_NAME names[MAX_CPD_STAT_PATH_NAME_DEPTH];
    unsigned int name_num = 0;
    unsigned int name_ind = 0;
    CPD_STAT_NODE* floor = node;

    assert(node && path);

    name_num = cpd_stat_parse_path(path, names);

    for (name_ind = name_num; name_ind > 0; --name_ind)
    {
        if (!floor || !cpd_stat_match_name(floor, &(names[name_ind])))
        {
            return false;
        }
        floor = (CPD_STAT_NODE*)floor->head.parent;
    }

    if (name_ind > 0)
    {
        return false;
    }

    return true;
}

bool cpd_stat_check_visit_result(bool visit, bool post_visit, bool next_floor)
{
    return (visit !=CPD_STAT_VISIT_STOP && 
            post_visit != CPD_STAT_VISIT_STOP &&
            next_floor == CPD_STAT_TRAVEL_END);
}

CPD_STAT_TRAVEL_RESULT 
cpd_stat_travel_internal (CPD_STAT_HEAD* floor, 
        CPD_STAT_TRAVEL_FLAG flag, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra)
{
    CPD_STAT_HEAD* next = floor;
    CPD_STAT_VISIT_RESULT  visit_result     = CPD_STAT_VISIT_CONTINUE;
    CPD_STAT_VISIT_RESULT  post_visit_result= CPD_STAT_VISIT_CONTINUE;
    CPD_STAT_TRAVEL_RESULT travel_result    = CPD_STAT_TRAVEL_MIDWAY;
    CPD_STAT_TRAVEL_RESULT next_floor_result= CPD_STAT_TRAVEL_END;
    bool check_visited = false;

    if (flag & CPD_STAT_TRAVEL_FLAG_VISITED_CHECK)
    {
        check_visited = true;
    }

    if (!next)
    {
        return CPD_STAT_TRAVEL_END;
    }

    do
    {
        visit_result = CPD_STAT_VISIT_CONTINUE;
        post_visit_result = CPD_STAT_VISIT_CONTINUE;
        next_floor_result = CPD_STAT_TRAVEL_END;

        if (check_visited && cpd_stat_is_node_visited((CPD_STAT_NODE*)next))
        {
            // by pass
            visit_result = CPD_STAT_VISIT_BYPASS;
            post_visit_result = CPD_STAT_VISIT_BYPASS;
            //printf("by pass %s\n", ((CPD_STAT_NODE*)next)->stat.name.str);
            next = next->next;
        }
        else
        {
			CPD_STAT_HEAD* parent = next->parent;
            if (visit)
            {
                visit_result = visit((CPD_STAT_NODE*)next, extra);
            }

            if (visit_result == CPD_STAT_VISIT_CONTINUE)
            {
                next_floor_result = cpd_stat_travel_internal(next->child_head, flag, visit, post_visit, extra);
            }

            if (post_visit && next_floor_result == CPD_STAT_TRAVEL_END)
            {
                post_visit_result = post_visit((CPD_STAT_NODE*)next, extra);
            }

			// parent changed, we can say the 'next' has been deleted, stop travel!
			if (next->parent != parent)
			{
				//printf("del itself detected!\n");
			    visit_result = CPD_STAT_VISIT_STOP;
			    post_visit_result = CPD_STAT_VISIT_STOP;
			}

            if (cpd_stat_check_visit_result(visit_result, post_visit_result, next_floor_result))
            {
                next = next->next;
            }
        }
    } while ((next != floor) && 
            cpd_stat_check_visit_result(visit_result, post_visit_result, next_floor_result));

    if (next == floor && 
        cpd_stat_check_visit_result(visit_result, post_visit_result, next_floor_result))
    {
        travel_result = CPD_STAT_TRAVEL_END;
    }

    return travel_result;
}

CPD_STAT_TRAVEL_RESULT
cpd_stat_travel (CPD_STAT* stat, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra)
{
    assert(stat); 
    assert(visit || post_visit); 

    if (stat->root.head.child_head)
    {
        return cpd_stat_travel_internal(stat->root.head.child_head, option.flag, 
                visit, post_visit, extra);
    }
    return CPD_STAT_TRAVEL_END;
}

CPD_STAT_TRAVEL_RESULT
cpd_stat_travel_staging (CPD_STAT* stat, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra)
{
    assert(stat); 
    assert(visit || post_visit); 

    if (stat->staging_node_pool.head.child_head)
    {
        return cpd_stat_travel_internal(stat->staging_node_pool.head.child_head, option.flag, 
                visit, post_visit, extra);
    }
    return CPD_STAT_TRAVEL_END;
}

CPD_STAT_TRAVEL_RESULT
cpd_stat_travel_pool (CPD_STAT* stat, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra)
{
    assert(stat); 
    assert(visit || post_visit); 

    if (stat->node_pool.head.child_head)
    {
        return cpd_stat_travel_internal(stat->node_pool.head.child_head, option.flag, 
                visit, post_visit, extra);
    }
    return CPD_STAT_TRAVEL_END;
}

CPD_STAT_VISIT_RESULT 
cpd_stat_visit_unmark_visited (CPD_STAT_NODE* node, void * extra)
{
    CPD_STAT_NODE* parent = node;
    if (node /*&& !node->head.child_head*/)
    {
        cpd_stat_unmark_node_visited(node);
    }
    return CPD_STAT_VISIT_CONTINUE;
}

CPD_STAT_TRAVEL_RESULT 
cpd_stat_unmark_visited(CPD_STAT* stat)
{
    CPD_STAT_TRAVEL_OPTION option;
    option.flag = 0;
    return cpd_stat_travel(stat, option, cpd_stat_visit_unmark_visited, (CPD_STAT_VISIT)0, (void*) 0);
}

CPD_STAT_VISIT_RESULT
cpd_stat_remove_bachelor_iterate (CPD_STAT_NODE* node, void * extra)
{
    CPD_STAT_HEAD* head = (CPD_STAT_HEAD*)node;
    CPD_STAT_HEAD* removed = NULL_STAT_HEAD;
	CPD_STAT* stat = (CPD_STAT*)extra;
    if (head && stat)
    {
		removed = cpd_stat_del_bachelor(head);
		if (removed)
		{
			cpd_stat_init_node((CPD_STAT_NODE*)removed, "");
			cpd_stat_add_child((CPD_STAT_HEAD*)&(stat->node_pool), removed);
		}
    }

    //return CPD_STAT_VISIT_STOP;
    return CPD_STAT_VISIT_CONTINUE;
}

bool cpd_stat_reclaim (CPD_STAT* stat, unsigned int count)
{
    CPD_STAT_TRAVEL_OPTION option;
    CPD_STAT_TRAVEL_RESULT result = CPD_STAT_TRAVEL_MIDWAY;
	assert(stat);

	do
	{
    	result = cpd_stat_travel_staging(stat, option, 
        	(CPD_STAT_VISIT)0, cpd_stat_remove_bachelor_iterate, (void*)stat);
		count--;
	} while (result != CPD_STAT_TRAVEL_END && count);

	if (result == CPD_STAT_TRAVEL_END)
	{
        return true;
	}

    return false;
}

