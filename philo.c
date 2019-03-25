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

int errorHandler(char* message){
    fprintf(stderr,"%s: %s\n", message, strerror(errno));
    return -1;
}

int eat(char* id){
    int time =abs(randomGaussian(9,3)); 
    printf("%s Eating for %d seconds\n",id, time);
    sleep(time);
    return time;
}

int think(char* id){
    int time =abs(randomGaussian(9,3)); 
    printf("%s Thinking for %d seconds\n",id, time);
    sleep(time);
    return time;
}

int philosopherAction(char* id, int pos){
    srand(getpid());
    int eatTime = 0, thinkTime = 0;
    while(eatTime < 100){
        eatTime += eat(id);
        if(eatTime > 100) break;
        thinkTime += think(id);
    }
    printf("%s has finished eating!\n",id);
    return 1;
}

int main(){

    int SID = semget(IPC_PRIVATE, 5, IPC_CREAT|IPC_EXCL|0600);
    if(SID < 0) return errorHandler("CSTMERR Failed to create semaphores");
    int ph1 = fork(), ph2, ph3, ph4, ph5;
    if(ph1 < 0) return errorHandler("CSTMERR Failed to fork 1");
    if(ph1 == 0){
        // Philosopher 1
        philosopherAction("Philosopher 1", 0);
    }else{
        // Parent
        ph2 = fork();
        if(ph2 < 0) return errorHandler("CSTMERR Failed to fork 2");
        if(ph2 == 0){
            // Philosopher 2
            philosopherAction("Philosopher 2", 1);

        }else{
            // Parent
            ph3 = fork();
            if(ph3 < 0) return errorHandler("CSTMERR Failed to fork 3");
            if(ph3 == 0){
                // Philosopher 3
                philosopherAction("Philosopher 3", 2);

            }else{
                // Parent
                ph4 = fork();
                if(ph4 < 0) return errorHandler("CSTMERR Failed to fork 4");
                if(ph4 == 0){
                    // Philosopher 4
                    philosopherAction("Philosopher 4", 3);

                }else{
                    // Parent
                    ph5 = fork();
                    if(ph5 < 0) return errorHandler("CSTMERR Failed to fork 4");
                    if(ph5 == 0){
                        // Philosopher 5
                        philosopherAction("Philosopher 5", 4);
                        
                    }else{
                        // Parent
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        printf("This shouldn't be printed ever\n");

                    }

                }
            }
        }
    }

}
