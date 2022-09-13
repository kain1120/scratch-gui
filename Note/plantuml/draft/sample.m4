include(`elements.m4')dnl
digraph G {
    FUNCTION(function)
    COROUTINE(coroutine)
    THREAD(thread)
    PROCESS(process)
    KTHREAD(kthread)
    INTERRUPT(interrupt)
    SOFTIRQ(softirq)
    CALLBACK(callback)
    TIMER(timer)
    ARRAY(array)
    LIST(list)
    QUEUE(queue)
    WORKQUEUE(workqueue)
    WORK(work)
    DELAYWORK(delaywork)

    COMP(comp)
    LOGIC(logic, logic.me)

    MODULE_BEGIN(module)
    DATA_BEGIN(data)
    DATA_MEM(member)
    DATA_END(data)
    MODULE_END(module)

    CALL(function, coroutine)
    IF_CALL(function, thread, thread)
    LOOP_CALL(function, process, process)

    CREATE(function, kthread)
    REGISTER(function, callback)

    LINK(comp, data)
    LINK(data:member, callback)

    IMSG(interrupt, softirq)
    IMSG(interrupt, workqueue)

    LINK(workqueue, work)
    LINK(workqueue, delaywork)
    
    ICREATE(work, timer)
    CALL(logic, function)

    IINSERT(work, queue)
    INSERT(work, list)
    IINSERT(data, array)

    OPERATE(function, timer)
}

