
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void crear_arbol(int nivel, int total_verticales, int total_finales,
                 pid_t pid_original, pid_t *pids_verticales,
                 pid_t *pids_finales) {
    int i;

    if (nivel < total_verticales) {
        pid_t pid_hijo = fork();

        if (pid_hijo < 0) {
            perror("Error al crear proceso");
            return;
        }

        if (pid_hijo == 0) {
            pids_verticales[nivel] = getpid();

            printf("Soy el proceso %d. Mis padres son: %d",
                   (int)getpid(), (int)pid_original);

            for (i = 0; i < nivel; i++) {
                printf(", %d", (int)pids_verticales[i]);
            }

            printf("\n");
            fflush(stdout);

            crear_arbol(nivel + 1, total_verticales, total_finales,
                        pid_original, pids_verticales, pids_finales);

            exit(0);
        }

        waitpid(pid_hijo, NULL, 0);
        return;
    }

    /* El último proceso vertical crea los hijos finales. */
    for (i = 0; i < total_finales; i++) {
        pid_t pid_hijo = fork();

        if (pid_hijo < 0) {
            perror("Error al crear hijo final");
            break;
        }

        if (pid_hijo == 0) {
            int j;

            pids_finales[i] = getpid();

            printf("Soy el subhijo %d, mis padres son: %d",
                   (int)getpid(), (int)pid_original);

            for (j = 0; j < total_verticales; j++) {
                printf(", %d", (int)pids_verticales[j]);
            }

            printf("\n");
            fflush(stdout);

            /* Mantiene el proceso vivo unos segundos para observarlo. */
            sleep(10);
            exit(0);
        }
    }

    /* El último proceso vertical espera a todos sus hijos finales. */
    for (i = 0; i < total_finales; i++) {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    int total_verticales;
    int total_finales;
    int memoria_verticales;
    int memoria_finales;
    pid_t *pids_verticales;
    pid_t *pids_finales;
    pid_t pid_original;
    int i;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s x y\n", argv[0]);
        return 1;
    }

    total_verticales = atoi(argv[1]);
    total_finales = atoi(argv[2]);

    if (total_verticales <= 0 || total_finales <= 0) {
        fprintf(stderr, "x e y deben ser mayores que cero.\n");
        return 1;
    }

    pid_original = getpid();

    memoria_verticales = shmget(
        IPC_PRIVATE,
        sizeof(pid_t) * total_verticales,
        IPC_CREAT | 0600
    );

    if (memoria_verticales < 0) {
        perror("Error en shmget");
        return 1;
    }

    memoria_finales = shmget(
        IPC_PRIVATE,
        sizeof(pid_t) * total_finales,
        IPC_CREAT | 0600
    );

    if (memoria_finales < 0) {
        perror("Error en shmget");
        shmctl(memoria_verticales, IPC_RMID, NULL);
        return 1;
    }

    pids_verticales = (pid_t *) shmat(memoria_verticales, NULL, 0);
    pids_finales = (pid_t *) shmat(memoria_finales, NULL, 0);

    if (pids_verticales == (void *) -1 ||
        pids_finales == (void *) -1) {
        perror("Error en shmat");

        if (pids_verticales != (void *) -1) {
            shmdt(pids_verticales);
        }

        if (pids_finales != (void *) -1) {
            shmdt(pids_finales);
        }

        shmctl(memoria_verticales, IPC_RMID, NULL);
        shmctl(memoria_finales, IPC_RMID, NULL);
        return 1;
    }

    for (i = 0; i < total_verticales; i++) {
        pids_verticales[i] = -1;
    }

    for (i = 0; i < total_finales; i++) {
        pids_finales[i] = -1;
    }

    crear_arbol(0, total_verticales, total_finales,
                pid_original, pids_verticales, pids_finales);

    printf("Soy el superpadre (%d), mis hijos finales son: ",
           (int)pid_original);

    for (i = 0; i < total_finales; i++) {
        printf("%d", (int)pids_finales[i]);

        if (i < total_finales - 1) {
            printf(", ");
        }
    }

    printf("\n");

    shmdt(pids_verticales);
    shmdt(pids_finales);

    shmctl(memoria_verticales, IPC_RMID, NULL);
    shmctl(memoria_finales, IPC_RMID, NULL);

    return 0;
}
