
        if(remaining[idx]==0){
            history[q].p[idx].ct=time;
            history[q].p[idx].tat=history[q].p[idx].ct-history[q].p[idx].at;
            history[q].p[idx].wt=history[q].p[idx].tat-history[q].p[idx].bt;
            completed++;
        }
    }
    addToTimeline(q, prev_pid, current_start, time);
}
