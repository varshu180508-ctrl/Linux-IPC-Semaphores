#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define NUM_LOOPS 10

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void wait_semaphore(int sem_set_id)
{
    struct sembuf sem_op;

    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;

    if (semop(sem_set_id, &sem_op, 1) == -1) {
        perror("semop wait");
        exit(1);
    }
}

void signal_semaphore(int sem_set_id)
{
    struct sembuf sem_op;

    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;

    if (semop(sem_set_id, &sem_op, 1) == -1) {
        perror("semop signal");
        exit(1);
    }
}

int main()
{
    int sem_set_id;
    union semun sem_val;
    pid_t child_pid;

    /* Create semaphore */
    sem_set_id = semget(IPC_PRIVATE, 1, 0600);

    if (sem_set_id == -1) {
        perror("semget");
        exit(1);
    }

    printf("Semaphore set created. ID = %d\n", sem_set_id);

    /* Initialize semaphore to 0 */
    sem_val.val = 0;

    if (semctl(sem_set_id, 0, SETVAL, sem_val) == -1) {
        perror("semctl");
        exit(1);
    }

    /* Create child process */
    child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        /* Child process - Consumer */

        for (int i = 0; i < NUM_LOOPS; i++) {
            wait_semaphore(sem_set_id);

            printf("Consumer: %d\n", i);
            fflush(stdout);
        }

        exit(0);
    }
    else {
        /* Parent process - Producer */

        for (int i = 0; i < NUM_LOOPS; i++) {
            printf("Producer: %d\n", i);
            fflush(stdout);

            signal_semaphore(sem_set_id);

            usleep(500000);
        }

        /* Wait for child */
        wait(NULL);

        /* Remove semaphore */
        if (semctl(sem_set_id, 0, IPC_RMID, sem_val) == -1) {
            perror("semctl remove");
            exit(1);
        }

        printf("Semaphore removed.\n");
    }

    return 0;
}
