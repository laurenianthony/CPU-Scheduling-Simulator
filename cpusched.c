#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_Q 20
#define MAX_P 20

struct Process {
    int pid, at, bt, ct, tat, wt, rt;
    int remaining;
    int priority;
};

struct GanttEntry {
    int pid;
    int start;
    int end;
};

struct Question {
    struct Process *p;
    int n;
    int algoChosen;
    int quantum;
    int priorityMode;
    struct GanttEntry timeline[2000];
    int tCount;
};

struct Question history[MAX_Q];
int qCount = 0;
int currentQ = -1;

void clearScreen();
void waitForEnter();
void showMainMenu();
void sortByArrival(int q);
void addToTimeline(int q, int pid, int start, int end);

void calculateFCFS(int q);
void calculateSJF_NP(int q);
void calculateSJF_P(int q);
void calculatePriority_NP(int q);
void calculatePriority_P(int q);
void calculateRR(int q);

void printGantt(int q);
void calculateAverages(int q);
void printProcessTable(int q);

int getInt(const char *prompt);

int main() {
    int choice;

    while (1) {
        showMainMenu();
        choice = getInt("> ");
        clearScreen();

        if (choice >= 1 && choice <= 4) {
            if (qCount >= MAX_Q) {
                printf("Maximum number of questions reached. Cannot add more entries.\n");
                waitForEnter();
                continue;
            }

            currentQ = qCount;
            history[currentQ].tCount = 0;
            history[currentQ].algoChosen = choice;

            int n;
            n = getInt("Enter number of processes: ");
            while(n <= 0 || n > MAX_P){
                printf("Error: Number of processes must be between 1 and %d.\n", MAX_P);
                waitForEnter();
                n = getInt("Enter number of processes: ");
            }

            history[currentQ].n = n;
            history[currentQ].p = malloc(sizeof(struct Process) * n);

            for (int i = 0; i < n; i++) {
                history[currentQ].p[i].pid = i + 1;

                char prompt[50];

                sprintf(prompt, "P%d Arrival Time: ", i+1);
                history[currentQ].p[i].at = getInt(prompt);
                while(history[currentQ].p[i].at < 0){
                    printf("Error: Arrival time cannot be negative.\n");
                    waitForEnter();
                    history[currentQ].p[i].at = getInt(prompt);
                }

                sprintf(prompt, "P%d Burst Time: ", i+1);
                history[currentQ].p[i].bt = getInt(prompt);
                while(history[currentQ].p[i].bt <= 0){
                    printf("Error: Burst time must be positive.\n");
                    waitForEnter();
                    history[currentQ].p[i].bt = getInt(prompt);
                }

                history[currentQ].p[i].remaining = history[currentQ].p[i].bt;
                history[currentQ].p[i].rt = -1;

                if (choice == 3) { 
                    sprintf(prompt, "P%d Priority: ", i+1);
                    history[currentQ].p[i].priority = getInt(prompt);
                } else {
                    history[currentQ].p[i].priority = 0;
                }
            }

            if (choice == 1) { 
                sortByArrival(currentQ);
                calculateFCFS(currentQ);
            }
            else if (choice == 2) {
                int mode;
                mode = getInt("SJF Mode (0 = Non-Preemptive, 1 = Preemptive): ");
                while(mode !=0 && mode !=1){
                    printf("Error: Invalid choice. Enter 0 for Non-Preemptive or 1 for Preemptive.\n");
                    waitForEnter();
                    mode = getInt("SJF Mode (0 = Non-Preemptive, 1 = Preemptive): ");
                }
                if (mode == 0) calculateSJF_NP(currentQ);
                else calculateSJF_P(currentQ);
            }
            else if (choice == 3) { 
                int mode;
                history[currentQ].priorityMode = getInt("Priority Type (0 = Lower number higher priority, 1 = Higher number higher priority): ");
                while(history[currentQ].priorityMode !=0 && history[currentQ].priorityMode !=1){
                    printf("Error: Invalid input. Enter 0 for Lower-number-high-priority or 1 for Higher-number-high-priority.\n");
                    waitForEnter();
                    history[currentQ].priorityMode = getInt("Priority Type (0 = Lower number higher priority, 1 = Higher number higher priority): ");
                }

                mode = getInt("Priority Mode (0 = Non-Preemptive, 1 = Preemptive): ");
                while(mode !=0 && mode !=1){
                    printf("Error: Invalid input. Enter 0 for Non-Preemptive or 1 for Preemptive.\n");
                    waitForEnter();
                    mode = getInt("Priority Mode (0 = Non-Preemptive, 1 = Preemptive): ");
                }

                if(mode==0) calculatePriority_NP(currentQ);
                else calculatePriority_P(currentQ);
            }
            else if (choice == 4) {
                history[currentQ].quantum = getInt("Enter Quantum: ");
                while(history[currentQ].quantum <= 0){
                    printf("Error: Quantum must be a positive integer.\n");
                    waitForEnter();
                    history[currentQ].quantum = getInt("Enter Quantum: ");
                }
                sortByArrival(currentQ);
                calculateRR(currentQ);
            }

            
            printProcessTable(currentQ);
            printGantt(currentQ);

            qCount++;
            waitForEnter();
        }

        else if (choice == 5) { 
            if(qCount==0){
                printf("No history available.\n");
                waitForEnter();
                continue;
            }

            for(int q=0;q<qCount;q++){
                printf("\nQuestion %d\n", q+1);
                printProcessTable(q);
                printGantt(q);
            }

            char ans;
            printf("\nDo you want to clear all history? (y/n): ");
            scanf(" %c",&ans);
            while(ans!='y' && ans!='Y' && ans!='n' && ans!='N'){
                printf("Error: Invalid input. Enter 'y' for Yes or 'n' for No: ");
                while(getchar()!='\n'); 
                scanf(" %c",&ans);
            }

            if(ans=='y' || ans=='Y'){
                for(int i=0;i<qCount;i++) free(history[i].p);
                qCount = 0;
                currentQ = -1;
                printf("All history has been cleared successfully.\n");
            }

            waitForEnter();
        }

        else if (choice == 0) { 
            for (int i = 0; i < qCount; i++)
                free(history[i].p);
            return 0;
        }

        else {
            printf("Error: Invalid choice. Please select a number between 0 and 5.\n");
            waitForEnter();
        }
    }
}


int getInt(const char *prompt){
    int value;
    while(1){
        printf("%s", prompt);
        if(scanf("%d",&value)==1){
            while(getchar()!='\n'); 
            return value;
        } else {
            while(getchar()!='\n'); 
            printf("\nInvalid input. Press Enter to try again...");
            getchar();
        }
    }
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitForEnter() {
    int c;
    printf("\nPress Enter to continue...");
    while ((c = getchar()) != '\n' && c != EOF); 
}


void showMainMenu() {
    clearScreen();
    printf("+----------------------------------------------+\n");
    printf("|        CPU SCHEDULING CALCULATOR             |\n");
    printf("+----------------------------------------------+\n");
    printf("| 1. FCFS                                      |\n");
    printf("| 2. SJF                                       |\n");
    printf("| 3. Priority                                  |\n");
    printf("| 4. Round Robin                               |\n");
    printf("| 5. View History                              |\n");
    printf("| 0. Exit                                      |\n");
    printf("+----------------------------------------------+\n");
}


void sortByArrival(int q) {
    for (int i = 0; i < history[q].n - 1; i++)
        for (int j = i + 1; j < history[q].n; j++)
            if (history[q].p[i].at > history[q].p[j].at) {
                struct Process t = history[q].p[i];
                history[q].p[i] = history[q].p[j];
                history[q].p[j] = t;
            }
}

void addToTimeline(int q, int pid, int start, int end) {
    if (start >= end) return;
    int tc = history[q].tCount;
    if(tc>0 && history[q].timeline[tc-1].pid==pid && history[q].timeline[tc-1].end==start)
        history[q].timeline[tc-1].end = end;
    else{
        history[q].timeline[tc].pid = pid;
        history[q].timeline[tc].start = start;
        history[q].timeline[tc].end = end;
        history[q].tCount++;
    }
}

void calculateFCFS(int q){
    int n = history[q].n;
    if (n <= 0) return;
    int time=0;
    for(int i=0;i<n;i++){
        if(time < history[q].p[i].at) {
            addToTimeline(q, 0, time, history[q].p[i].at);
            time = history[q].p[i].at;
        }
        int start = time;
        history[q].p[i].wt = time - history[q].p[i].at;
        history[q].p[i].rt = history[q].p[i].wt;
        time += history[q].p[i].bt;
        history[q].p[i].ct = time;
        history[q].p[i].tat = time - history[q].p[i].at;
        addToTimeline(q, history[q].p[i].pid, start, time);
    }
}


void calculateSJF_NP(int q){
    int n=history[q].n, completed=0, time=0;
    if (n <= 0) return;
    int done[MAX_P]={0};
   
    while(completed<n){
        int idx=-1, minBT=1e9;
        for(int i=0;i<n;i++){
            if(!done[i] && history[q].p[i].at<=time){
                if (history[q].p[i].bt < minBT) {
                    minBT=history[q].p[i].bt;
                    idx=i;
                } else if (history[q].p[i].bt == minBT) {
                    if (idx == -1 || history[q].p[i].at < history[q].p[idx].at || (history[q].p[i].at == history[q].p[idx].at && history[q].p[i].pid < history[q].p[idx].pid)) {
                        idx = i;
                    }
                }
            }
        }
        if(idx==-1){
            int nextArrival = 1e9;
            for(int i=0;i<n;i++) if(!done[i] && history[q].p[i].at < nextArrival) nextArrival = history[q].p[i].at;
            if (nextArrival == 1e9) break;
            addToTimeline(q, 0, time, nextArrival);
            time = nextArrival;
            continue;
        }
       
        int start = time;
        history[q].p[idx].wt = time - history[q].p[idx].at;
        history[q].p[idx].rt = history[q].p[idx].wt;
        time += history[q].p[idx].bt;
        history[q].p[idx].ct = time;
        history[q].p[idx].tat = time - history[q].p[idx].at;
        done[idx]=1;
        completed++;
        addToTimeline(q, history[q].p[idx].pid, start, time);
    }
}


void calculateSJF_P(int q){
    int n=history[q].n, completed=0, time=0;
    if (n <= 0) return;
    int remaining[MAX_P];
    for(int i=0;i<n;i++) {
        remaining[i]=history[q].p[i].bt;
        history[q].p[i].rt = -1;
    }


    int prev_pid = -1;
    int current_start = 0;


    while(completed<n){
        int idx=-1, minRem=1e9;
        for(int i=0;i<n;i++){
            if(history[q].p[i].at<=time && remaining[i]>0){
                if(remaining[i]<minRem){
                    minRem=remaining[i];
                    idx=i;
                } else if (remaining[i] == minRem) {
                    if (idx == -1 || history[q].p[i].at < history[q].p[idx].at || (history[q].p[i].at == history[q].p[idx].at && history[q].p[i].pid < history[q].p[idx].pid)) {
                        idx = i;
                    }
                }
            }
        }
       
        int current_pid = (idx == -1) ? 0 : history[q].p[idx].pid;
       
        if (current_pid != prev_pid) {
            if (time > 0) {
                 addToTimeline(q, prev_pid, current_start, time);
            }
            current_start = time;
            prev_pid = current_pid;
        }


        if(idx==-1){
            int nextArrival = 1e9;
            for(int i=0;i<n;i++) if(remaining[i]>0 && history[q].p[i].at < nextArrival) nextArrival = history[q].p[i].at;
            if (nextArrival == 1e9) break;
            time = nextArrival;
            continue;
        }
       
        if(history[q].p[idx].rt==-1) history[q].p[idx].rt=time - history[q].p[idx].at;
        remaining[idx]--;
        time++;
       
        if(remaining[idx]==0){
            history[q].p[idx].ct=time;
            history[q].p[idx].tat=history[q].p[idx].ct-history[q].p[idx].at;
            history[q].p[idx].wt=history[q].p[idx].tat-history[q].p[idx].bt;
            completed++;
        }
    }
    addToTimeline(q, prev_pid, current_start, time);
}


void calculatePriority_NP(int q){
    int n=history[q].n, completed=0, time=0;
    if (n <= 0) return;
    int done[MAX_P]={0};
    int mode = history[q].priorityMode;


    while(completed<n){
        int idx=-1;
        for(int i=0;i<n;i++){
            if(!done[i] && history[q].p[i].at<=time){
                if(idx == -1){
                    idx = i;
                } else {
                    int better = 0;
                    if (mode == 0) {
                        if (history[q].p[i].priority < history[q].p[idx].priority) better = 1;
                        else if (history[q].p[i].priority == history[q].p[idx].priority) better = 0;
                        else better = -1;
                    } else {
                        if (history[q].p[i].priority > history[q].p[idx].priority) better = 1;
                        else if (history[q].p[i].priority == history[q].p[idx].priority) better = 0;
                        else better = -1;
                    }
                   
                    if(better == 1){
                        idx = i;
                    } else if(better == 0){
                        if(history[q].p[i].at < history[q].p[idx].at){
                            idx = i;
                        } else if(history[q].p[i].at == history[q].p[idx].at && history[q].p[i].pid < history[q].p[idx].pid){
                            idx = i;
                        }
                    }
                }
            }
        }


        if(idx==-1){
            int nextArrival = 1e9;
            for(int i=0;i<n;i++) if(!done[i] && history[q].p[i].at < nextArrival) nextArrival = history[q].p[i].at;
            if (nextArrival == 1e9) break;
            addToTimeline(q, 0, time, nextArrival);
            time = nextArrival;
            continue;
        }
       
        int start = time;
        history[q].p[idx].wt = time-history[q].p[idx].at;
        history[q].p[idx].rt = history[q].p[idx].wt;
        time += history[q].p[idx].bt;
        history[q].p[idx].ct = time;
        history[q].p[idx].tat = time-history[q].p[idx].at;
        done[idx]=1;
        completed++;
        addToTimeline(q, history[q].p[idx].pid, start, time);
    }
}


void calculatePriority_P(int q){
    int n=history[q].n, completed=0, time=0;
    if (n <= 0) return;
    int remaining[MAX_P];
    int mode = history[q].priorityMode;
    for(int i=0;i<n;i++) {
        remaining[i]=history[q].p[i].bt;
        history[q].p[i].rt = -1;
    }
   
    int prev_pid = -1;
    int current_start = 0;


    while(completed<n){
        int idx = -1;
        for(int i=0;i<n;i++){
            if(history[q].p[i].at<=time && remaining[i]>0){
                if(idx == -1){
                    idx = i;
                } else {
                    int better = 0;
                    if (mode == 0) {
                        if (history[q].p[i].priority < history[q].p[idx].priority) better = 1;
                        else if (history[q].p[i].priority == history[q].p[idx].priority) better = 0;
                        else better = -1;
                    } else {
                        if (history[q].p[i].priority > history[q].p[idx].priority) better = 1;
                        else if (history[q].p[i].priority == history[q].p[idx].priority) better = 0;
                        else better = -1;
                    }
                   
                    if(better == 1){
                        idx = i;
                    } else if(better == 0){
                         if(history[q].p[i].at < history[q].p[idx].at){
                            idx = i;
                        } else if(history[q].p[i].at == history[q].p[idx].at && history[q].p[i].pid < history[q].p[idx].pid){
                            idx = i;
                        }
                    }
                }
            }
        }
       
        int current_pid = (idx == -1) ? 0 : history[q].p[idx].pid;
        if (current_pid != prev_pid) {
            if (time > 0) addToTimeline(q, prev_pid, current_start, time);
            current_start = time;
            prev_pid = current_pid;
        }


        if(idx==-1){
            int nextArrival = 1e9;
            for(int i=0;i<n;i++) if(remaining[i]>0 && history[q].p[i].at < nextArrival) nextArrival = history[q].p[i].at;
            if (nextArrival == 1e9) break;
            time = nextArrival;
            continue;
        }
       
        if(history[q].p[idx].rt==-1) history[q].p[idx].rt=time - history[q].p[idx].at;
        remaining[idx]--;
        time++;
       
        if(remaining[idx]==0){
            history[q].p[idx].ct=time;
            history[q].p[idx].tat=history[q].p[idx].ct-history[q].p[idx].at;
            history[q].p[idx].wt=history[q].p[idx].tat-history[q].p[idx].bt;
            completed++;
        }
    }
    addToTimeline(q, prev_pid, current_start, time);
}

void calculateRR(int q){
    int n=history[q].n, doneCount=0, time=0;
    if (n <= 0) return;
    int remaining[MAX_P];
    int queue[MAX_P + 1];
    int head=0, tail=0;
    int inQueue[MAX_P] = {0};


    for(int i=0;i<n;i++) {
        remaining[i]=history[q].p[i].bt;
        history[q].p[i].rt = -1;
    }


    sortByArrival(q);
   
    for(int i=0; i<n; i++) {
        if(history[q].p[i].at <= time) {
            queue[tail] = i;
            tail = (tail + 1) % (MAX_P + 1);
            inQueue[i] = 1;
        }
    }
   
    while(doneCount < n) {
        if(head != tail) {
            int i = queue[head];
            head = (head + 1) % (MAX_P + 1);
            inQueue[i] = 0;


            int t = remaining[i] > history[q].quantum ? history[q].quantum : remaining[i];
           
            if(history[q].p[i].rt == -1) history[q].p[i].rt = time - history[q].p[i].at;
           
            int start = time;
           
            for(int step=0; step<t; step++) {
                time++;
                for(int j=0; j<n; j++) {
                    if(j != i && !inQueue[j] && remaining[j] > 0 && history[q].p[j].at <= time) {
                        queue[tail] = j;
                        tail = (tail + 1) % (MAX_P + 1);
                        inQueue[j] = 1;
                    }
                }
            }
           
            addToTimeline(q, history[q].p[i].pid, start, time);
           
            remaining[i] -= t;
           
            if(remaining[i] == 0) {
                history[q].p[i].ct = time;
                history[q].p[i].tat = time - history[q].p[i].at;
                history[q].p[i].wt = history[q].p[i].tat - history[q].p[i].bt;
                doneCount++;
            } else {
                queue[tail] = i;
                tail = (tail + 1) % (MAX_P + 1);
                inQueue[i] = 1;
            }
        } else {
            int nextArrival = 1e9;
            for(int j=0; j<n; j++) {
                if(remaining[j] > 0 && history[q].p[j].at < nextArrival) {
                    nextArrival = history[q].p[j].at;
                }
            }
            if (nextArrival == 1e9) break;
           
            addToTimeline(q, 0, time, nextArrival);
            time = nextArrival;
           
            for(int j=0; j<n; j++) {
                if(!inQueue[j] && remaining[j] > 0 && history[q].p[j].at <= time) {
                    queue[tail] = j;
                    tail = (tail + 1) % (MAX_P + 1);
                    inQueue[j] = 1;
                }
            }
        }
    }
}

void printProcessTable(int q) {
    char *algoName;
    switch(history[q].algoChosen) {
        case 1: algoName = "FCFS"; break;
        case 2: algoName = "SJF"; break;
        case 3: algoName = "Priority"; break;
        case 4: algoName = "Round Robin"; break;
        default: algoName = "Unknown"; break;
    }
    printf("\n========== %s ==========\n\n", algoName);

    int showPriority = (history[q].algoChosen == 3); 

    if(showPriority)
        printf("+--------------------------------------------------------+\n| PID | AT | BT | PR   | CT | TAT | WT | RT |\n+--------------------------------------------------------+\n");
    else
        printf("+---------------------------------------------+\n| PID | AT | BT | CT | TAT | WT | RT |\n+---------------------------------------------+\n");

    for(int i=0;i<history[q].n;i++){
        struct Process p = history[q].p[i];
        if(showPriority){
            printf("| %3d | %2d | %2d | %4d | %2d | %3d | %2d | %2d |\n",
                   p.pid, p.at, p.bt, p.priority, p.ct, p.tat, p.wt, p.rt);
        } else {
            printf("| %3d | %2d | %2d | %2d | %3d | %2d | %2d |\n",
                   p.pid, p.at, p.bt, p.ct, p.tat, p.wt, p.rt);
        }
    }

    if(showPriority)
        printf("+--------------------------------------------------------+\n");
    else
        printf("+---------------------------------------------+\n");

    calculateAverages(q);
}

void calculateAverages(int q){
    int n = history[q].n;
    if (n <= 0) return;
    double avgTAT=0, avgWT=0;
    int totalBT=0;
    int totalTime=0;
    for(int i=0;i<n;i++){
        avgTAT += history[q].p[i].tat;
        avgWT  += history[q].p[i].wt;
        totalBT += history[q].p[i].bt;
        if(history[q].p[i].ct > totalTime) totalTime = history[q].p[i].ct;
    }
    printf("\n--- Average Performance Metrics ---\n");
    printf("Average Turnaround Time : %.2lf\n", avgTAT/n);
    printf("Average Waiting Time    : %.2lf\n", avgWT/n);
    if (totalTime > 0)
        printf("CPU Utilization : %.2lf%%\n", (double)totalBT/totalTime*100);
    else
        printf("CPU Utilization         : 0.00%%\n");
}

void printGantt(int q){
    int tCount = history[q].tCount;
    if(tCount == 0) {
        printf("No Gantt Chart generated.\n");
        return;
    }


    int blockWidth = 8;
    int gap = 2;
    printf("\n========== GANTT CHART ==========\n");


    for(int i=0;i<tCount;i++){
        printf("+");
        for(int j=0;j<blockWidth + gap;j++) printf("-");
    }
    printf("+\n");


    for(int i=0;i<tCount;i++){
        char pidStr[15];
        if(history[q].timeline[i].pid==0) strcpy(pidStr,"IDLE");
        else sprintf(pidStr,"P%d",history[q].timeline[i].pid);


        int left = (blockWidth + gap - (int)strlen(pidStr))/2;
        int right = blockWidth + gap - left - (int)strlen(pidStr);


        printf("|");
        for(int j=0;j<left;j++) printf(" ");
        printf("%s", pidStr);
        for(int j=0;j<right;j++) printf(" ");
    }
    printf("|\n");


    for(int i=0;i<tCount;i++){
        printf("+");
        for(int j=0;j<blockWidth + gap;j++) printf("-");
    }
    printf("+\n");


    for(int i=0;i<tCount;i++){
        printf("%d", history[q].timeline[i].start);
       
        int n_time = history[q].timeline[i].start;
        int len = 0;
        if (n_time == 0) len = 1;
        else {
            int temp = n_time;
            while (temp != 0) { temp /= 10; len++; }
        }


        int spaces = (blockWidth + gap) - len + 1;
        if (spaces < 1) spaces = 1;
       
        for(int j=0;j<spaces;j++) printf(" ");
       
        if(i==tCount-1) printf("%d", history[q].timeline[i].end);
    }
    printf("\n\n");
}

