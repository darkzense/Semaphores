#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>

void spam(const char* letter, int times);

int main(int argc, char *argv[])
{
    int pid, semid;
    struct sembuf sb = {0, -1, 0};
    if ((semid = semget(IPC_PRIVATE, 2, IPC_CREAT|0600)) == -1)
    {
        perror("semget");
        exit(1);
    }

    /* Se crea un proceso hijo */
    pid = fork();
    /* Se comprueba que no hubo error en fork() */
    if (pid == -1)
    {
        perror("fork");
        exit(-1);
    }

    if (pid != 0)
    { /* Proceso Padre */
        for (int i=0; i<3; i++) {
            /* Comienzo de la sección critica: */
            spam("+", 10);
            /* Fin de la sección critica */

            sb.sem_num = 0;
            sb.sem_op = 1;
            // Semaforo en verde para el hijo
            if (semop(semid, &sb, 1) == -1) 
            { 
                perror("semop");
                exit(1);
            }

            sb.sem_num = 1;
            sb.sem_op = -1;
            // Semaforo en rojo para el padre
            if (semop(semid, &sb, 1) == -1) 
            { 
                perror("semop");
                exit(1);
            }
        }
    }
    else
    { /* Proceso Hijo */
        for (int i=0; i<3; i++) {
            sb.sem_num = 0;
            sb.sem_op = -1;
            // Semaforo en rojo para el hijo
            if (semop(semid, &sb, 1) == -1) 
            { 
                perror("semop");
                exit(1);
            }
            /* Comienzo de la sección critica: */
            spam("o", 10);
            /* Fin de la sección critica */
            sb.sem_num = 1;
            sb.sem_op = 1;
            // Semaforo en verde para el padre
            if (semop(semid, &sb, 1) == -1) 
            { 
                perror("semop");
                exit(1);
            }
        }
    }
}

void spam(const char* letter, int times)
{
    for (int j = 0; j < times; j++)
    {
        write(STDOUT_FILENO, letter, 1);
        sleep(1);
    }
}
