#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5))
		return (int) floor(mu + sigma * cos(f2) * f1);
	else
		return (int) floor(mu + sigma * sin(f2) * f1);
}

int errorHandler(char* message){ // Just a convenient method to have.
    fprintf(stderr,"CSTMERR %s: %s\n", message, strerror(errno));
    printf("%s: %s\n", message, strerror(errno));
    return -1;
}

int eat(char* id){  // EAT THE SPGAHETI
    int time =abs(randomGaussian(9,3)); 
    printf("%s Eating for %d seconds\n",id, time);
    sleep(time);
    return time;
}

int think(char* id){  // THONKING
    int time =abs(randomGaussian(11,7));
    printf("%s Thinking for %d seconds\n",id, time);
    sleep(time);
    return time;
}

int philosopherAction(char* id, int pos, int SID){
    struct sembuf sem_op_wait_right = {pos,-1,0};          // Set the indexes that are being waited on for each philosopher
    struct sembuf sem_op_wait_left = {(pos+1)%5,-1,0};     // In such a way that they are to the left and right of each philosopher.
    struct sembuf sem_op_signal_right = {pos,1,0};
    struct sembuf sem_op_signal_left = {(pos+1)%5,1,0};
    srand(getpid());
    int eatTime = 0, thinkTime = 0;
    while(1){
        if(pos%2){
            if(semop(SID, &sem_op_wait_left, 1) < 0) return errorHandler("Failed to wait for left chopstick");
            printf("%s acquired chopstick %d\n",id,((pos+1)%5));
            if(semop(SID, &sem_op_wait_right, 1) < 0) return errorHandler("Failed to wait for right chopstick");
            printf("%s acquired chopstick %d\n",id,pos);
        }else{ 
            if(semop(SID, &sem_op_wait_right, 1) < 0) return errorHandler("Failed to wait for right chopstick");
            printf("%s acquired chopstick %d\n",id,pos);
            if(semop(SID, &sem_op_wait_left, 1) < 0) return errorHandler("Failed to wait for left chopstick");
            printf("%s acquired chopstick %d\n",id,((pos+1)%5));
        }
        eatTime += eat(id);
        if(pos%2){
            if(semop(SID, &sem_op_signal_right, 1) < 0) return errorHandler("Failed to set down right chopstick");
            if(semop(SID, &sem_op_signal_left, 1) < 0) return errorHandler("Failed to set down left chopstick");
        }else{
            if(semop(SID, &sem_op_signal_left, 1) < 0) return errorHandler("Failed to set down left chopstick");
            if(semop(SID, &sem_op_signal_right, 1) < 0) return errorHandler("Failed to set down right chopstick");
        }
        if(eatTime > 100) break;
        thinkTime += think(id);
    }
    printf("%s has finished eating!\n",id);
    return 1;
}

int main(){
    int SID = semget(IPC_PRIVATE, 5, IPC_CREAT|IPC_EXCL|0600);
    for(int i = 0; i < 5; i++){
        if(semctl(SID, i,SETVAL,1) < 0) return errorHandler("Failed to set semiphore value to 1");
    }
    if(SID < 0) return errorHandler("Failed to create semaphores");
    int ph1 = fork(), ph2, ph3, ph4, ph5;
    if(ph1 < 0) return errorHandler("Failed to fork 1");
    if(ph1 == 0){
        // Philosopher 1
        philosopherAction("Philosopher 1", 0, SID);
    }else{
        // Parent
        ph2 = fork();
        if(ph2 < 0) return errorHandler("Failed to fork 2");
        if(ph2 == 0){
            // Philosopher 2
            philosopherAction("Philosopher 2", 1, SID);

        }else{
            // Parent
            ph3 = fork();
            if(ph3 < 0) return errorHandler("Failed to fork 3");
            if(ph3 == 0){
                // Philosopher 3
                philosopherAction("Philosopher 3", 2, SID);

            }else{
                // Parent
                ph4 = fork();
                if(ph4 < 0) return errorHandler("Failed to fork 4");
                if(ph4 == 0){
                    // Philosopher 4
                    philosopherAction("Philosopher 4", 3, SID);

                }else{
                    // Parent
                    ph5 = fork();
                    if(ph5 < 0) return errorHandler("Failed to fork 4");
                    if(ph5 == 0){
                        // Philosopher 5
                        philosopherAction("Philosopher 5", 4, SID);
                        
                    }else{
                        // Parent
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        if(semctl(SID, 0,IPC_RMID) < 0) return errorHandler("Failed to remove semiphore set.");
                    }

                }
            }
        }
    }

}
