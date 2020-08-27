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

CPD_STAT_NODE* cpd_stat_follow_path (CPD_STAT_ROOT* root, const char * path, CPD_STAT_FOLLOW_PATH_OPTION option)
{
    CPD_STAT_NAME names[MAX_CPD_STAT_PATH_NAME_DEPTH];
    CPD_STAT_HEAD* floor = NULL_STAT_HEAD;
    CPD_STAT_HEAD* match = NULL_STAT_HEAD; 
    CPD_STAT_HEAD* last_match = (CPD_STAT_HEAD*)root; 
    unsigned int name_num = 0;
    unsigned int name_ind = 0;

    assert(root && path);

    floor = root->head.child_head;
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
                    continue;
                }
            }
        }
    }

    if (name_ind != name_num)
    {
        last_match = NULL_STAT_HEAD;
    }

    return ((CPD_STAT_NODE*) last_match);
}

CPD_STAT_NODE* cpd_stat_search_path (CPD_STAT_ROOT* root, const char * path)
{
	CPD_STAT_FOLLOW_PATH_OPTION option;
    option.flag = 0;
    option.get_node = NULL;
    option.extra = (void*)0;

	return cpd_stat_follow_path(root, path, option);
}

CPD_STAT_NODE* cpd_stat_add_path (CPD_STAT_ROOT* root, const char * path, CPD_STAT_GET_UNINIT_NODE get_node, void* extra)
{
	CPD_STAT_FOLLOW_PATH_OPTION option;

    option.flag = 0;
    option.get_node = get_node;
    option.extra = extra;
    option.flag |= CPD_STAT_FOLLOW_PATH_FLAG_ADD_UNFOUND;

	return cpd_stat_follow_path(root, path, option);
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
				printf("del itself detected!\n");
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
cpd_stat_travel (CPD_STAT_ROOT* root, 
        CPD_STAT_TRAVEL_OPTION option, 
        CPD_STAT_VISIT visit, 
        CPD_STAT_VISIT post_visit, 
        void* extra)
{
    assert(root); 
    assert(visit || post_visit); 

    if (root->head.child_head)
    {
        return cpd_stat_travel_internal(root->head.child_head, option.flag, 
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
cpd_stat_unmark_visited(CPD_STAT_ROOT* root)
{
    CPD_STAT_TRAVEL_OPTION option;
    option.flag = 0;
    return cpd_stat_travel(root, option, cpd_stat_visit_unmark_visited, (CPD_STAT_VISIT)0, (void*) 0);
}

