define(FUNCTION,`$1 [image="LOCATION/function.png", shape=none];')dnl
define(COROUTINE,`$1 [image="LOCATION/coroutine.png", shape=none];')dnl
define(THREAD, `$1 [image="LOCATION/thread.png", shape=none];')dnl
define(PROCESS, `$1 [image="LOCATION/process.png", shape=none];')dnl
define(KTHREAD, `$1 [image="LOCATION/kthread.png", shape=none];')dnl
define(INTERRUPT, `$1 [image="LOCATION/interrupt.png", shape=none];')dnl
define(SOFTIRQ, `$1 [image="LOCATION/softirq.png", shape=none];')dnl
define(CALLBACK, `$1 [image="LOCATION/callback.png", shape=none, margin="0.5,0"];')dnl
define(TIMER, `$1 [image="LOCATION/timer.png", shape=none, margin="0,0.5"];')dnl
define(ARRAY, `$1 [image="LOCATION/array.png", shape=none, labelloc="b"];')dnl
define(QUEUE, `$1 [image="LOCATION/queue.png", shape=none, labelloc="b"];')dnl
define(LIST, `$1 [image="LOCATION/list.png", shape=none, labelloc="b"];')dnl
define(WORKQUEUE, `$1 [image="LOCATION/workqueue.png", shape=none, labelloc="b"];')dnl
define(WORK, `$1 [image="LOCATION/work.png", shape=none, labelloc="c", margin="0,0.5"];')dnl
define(DELAYWORK, `$1 [image="LOCATION/delaywork.png", shape=none, labelloc="c", margin="0,0.5"];')dnl
define(COMP, `$1 [shape=component, style=filled, fillcolor="darkgoldenrod3", label=ifelse($2,,"$1","$2")];')dnl
define(LOGIC, `$1 [shape=signature, style=filled, fillcolor="chocolate2", label=ifelse($2,,"$1","$2")];')dnl
define(FILE, `$1 [shape=note, style=filled, fillcolor="darkseagreen2", label=ifelse($2,,"$1","$2")];')dnl
define(APP, `$1 [shape=doublecircle, style=filled, fillcolor="firebrick2", label=ifelse($2,,"$1","$2")];')dnl
define(DATA_BEGIN,`$1 [shape=record, color="firebrick", style=solid, label="{$1')dnl
define(DATA_MEM, `|<$1> $1')dnl
define(DATA_END,`}"]')dnl
define(MODULE_BEGIN, 
`subgraph cluster_$1 {
    shape=box
    label=$1
    color="dodgerblue"
    style=bold')dnl
define(MODULE_END, `}')dnl

define(EDGE_LABEL, `label=ifelse($1,,"","$1")')dnl
define(CALL, `$1 -> $2 [arrowhead="normal", color="firebrick2", EDGE_LABEL($3)]')dnl
define(IF_CALL, `$1 -> $2 [arrowhead="lnormal", color="firebrick2", EDGE_LABEL($3)]')dnl
define(LOOP_CALL, `$1 -> $2 [arrowhead="crow", color="firebrick2", EDGE_LABEL($3)]')dnl
define(CREATE, `$1 -> $2 [arrowhead="icurve", color="firebrick4", EDGE_LABEL($3)]')dnl
define(LINK, `$1 -> $2 [arrowhead="dot", color="darkseagreen4", EDGE_LABEL($3)]')dnl
define(OPERATE, `$1 -> $2 [arrowhead="onormal", color="darkseagreen4", EDGE_LABEL($3)]')dnl
define(INSERT, `$1 -> $2 [arrowhead="curve", color="darkorange1", EDGE_LABEL($3)]')dnl
define(MSG, `$1 -> $2 [arrowhead="onormal", color="darkseagreen4", style="dashed", EDGE_LABEL($3)]')dnl
define(REGISTER, `$1 -> $2 [arrowhead="ocurve", color="firebrick4", EDGE_LABEL($3)]')dnl

define(ICALL, `$2 -> $1 [arrowtail="normal", dir=back, color="firebrick2", EDGE_LABEL($3)]')dnl
define(IF_ICALL, `$2 -> $1 [arrowtail="lnormal", dir=back, color="firebrick2", EDGE_LABEL($3)]')dnl
define(LOOP_ICALL, `$2 -> $1 [arrowtail="crow", dir=back, color="firebrick2", EDGE_LABEL($3)]')dnl
define(ICREATE, `$2 -> $1 [arrowtail="icurve", dir=back, color="firebrick4", EDGE_LABEL($3)]')dnl
define(ILINK, `$2 -> $1 [arrowtail="dot", dir=back, color="darkseagreen4", EDGE_LABEL($3)]')dnl
define(IOPERATE, `$2 -> $1 [arrowhead="onormal", dir=back, color="darkseagreen4", EDGE_LABEL($3)]')dnl
define(IINSERT, `$2 -> $1 [arrowtail="curve", dir=back, color="darkorange1", EDGE_LABEL($3)]')dnl
define(IMSG, `$2 -> $1 [arrowtail="onormal", dir=back, color="darkseagreen4", style="dashed", EDGE_LABEL($3)]')dnl
define(IREGISTER, `$2 -> $1 [arrowtail="ocurve", dir=back, color="firebrick4", EDGE_LABEL($3)]')dnl

define(FORCE_ORDER, `{rank = same; $1; $2; $1 -> $2 [style=invis];}')dnl

define(HTTPSITE, `http://127.0.0.1:8000')dnl
define(HYPERLINK, `$1 [href="HTTPSITE$2"]')dnl 
