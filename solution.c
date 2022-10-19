#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#define PARENT_SEM 0
#define CHILD_SEM 1
#define ITERATIONS 3

void spam(const char* letter, int times);
void lock_sem(int sem_id, int sem_num);
void release_sem(int sem_id, int sem_num);

int main(int argc, char *argv[])
{
    int pid, semid;
    // Buffer de operaciones del semaforo
    struct sembuf sb = {0, -1, 0};

    // Se crea el semaforo
    if ((semid = semget(IPC_PRIVATE, 2, IPC_CREAT|0600)) == -1)
    {
        perror("semget");
        exit(1);
    }

    /* Se comprueba que no hubo error en fork() */
    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(-1);
    }

    if (pid != 0)
    { /* Proceso Padre */
        for (int i=0; i<ITERATIONS; i++) {
            /* *** Comienzo de la sección critica: *** */
            spam("+", 10);
            /* *** Fin de la sección critica *** */
            // Semaforo en verde para el hijo
            release_sem(semid, CHILD_SEM);
            // Semaforo en rojo para el padre
            lock_sem(semid, PARENT_SEM);
        }
    }
    else
    { /* Proceso Hijo */
        for (int i=0; i<ITERATIONS; i++) {
            // Semaforo en rojo para el hijo
            lock_sem(semid, CHILD_SEM);
            /* *** Comienzo de la sección critica: *** */
            spam("o", 10);
            /* *** Fin de la sección critica *** */
            // Semaforo en verde para el padre
            release_sem(semid, PARENT_SEM);
        }
    }
}

void spam(const char* letter, int times)
{
    for (int j = 0; j < times; j++)
    {
        write(STDOUT_FILENO, letter, 1);
    }
}

void sem_set(int sem_id, struct sembuf* sb, int sem_num, int sem_op) {
    sb->sem_num = sem_num;
    sb->sem_op = sem_op;
    sb->sem_flg = 0;
    // Semaforo en verde para el padre
    if (semop(sem_id, sb, 1) == -1) 
    { 
        perror("semop");
        exit(1);
    }
}

void lock_sem(int sem_id, int sem_num) {
    struct sembuf sb;
    sem_set(sem_id, &sb, sem_num, -1);
}

void release_sem(int sem_id, int sem_num) {
    struct sembuf sb;
    sem_set(sem_id, &sb, sem_num, 1);
}

