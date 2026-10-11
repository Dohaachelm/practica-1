#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>


// variables globales donde se almacenan los pids
pid_t arb, a, b, x, y, z, pid;

// esto lo hace A
void ejecuta_pstree(){
	char arb_cad[100];
	sprintf(arb_cad, "%d", arb);
	if(fork() == 0){
		// el proceso se convierte en el comando.
		execlp("pstree", "pstree", "-c", arb_cad, NULL);
	}
	else{
		wait(NULL); // para que el hijo acabe el comando
		printf("mandando señal a arb: %d\n", arb);
		kill(arb, SIGUSR2);	// para que inicie la destruccion del arbo.
		pause();
	}
}

void destroy_ejec(){
	printf("ejec => arb\n");
	kill(a, SIGUSR2);
}

void destroy_A(){
	printf("A => B\n");
	kill(b, SIGUSR2);
}

void destroy_B(){

}

void nada(){

}

/*
	argv[0] = "./ejec"
	argv[2] = "123"
*/
int main(int argc, char *argv[]){
	int i, tiempo;

	if(argc != 2){
		printf("Usage: ./ejec tiempo\n");
	}
	else{
		tiempo = atoi(argv[1]);	// tiempo = atoi("123") => tiempo = 123
		arb = getpid();
		printf("Soy el proceso ejec: mi pid es %d\n", arb);
		a = fork();	// al hijo 0
		if(a != 0){
			// ==================== ESTOY EN EJEC =======================
			signal(SIGUSR2, destroy_ejec);
			pause(); // espero

			wait(NULL);
			printf("Soy ejec (%d) y muero\n", arb);
			exit(0);
		}
		else{
			// ==================== ESTOY EN A ===========================
			a = getpid();
			printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n",a, arb);
			b = fork();
			if(b != 0){
				// =============== ESTOY EN A ============================
				signal(SIGUSR1, ejecuta_pstree);
				signal(SIGUSR2, destroy_A);
				pause(); // aqui me llega SIGUSR1
				wait(NULL);
				printf("Soy A (%d) y muero\n", a);
			}
			else{
				// ============== ESTOY EN B ==============================
				b = getpid();
				printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n", b, a, arb);

				x = fork();
				if(x == 0){
					printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n", getpid(), b, a, arb);
					signal(SIGUSR2, nada);
					pause();
					printf("Soy X (%d) y muero\n", getpid());
					exit(0);
				}
				y = fork();
				if(y == 0){
					printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n", getpid(), b, a, arb);
					signal(SIGUSR2, nada);
					pause();
					printf("Soy Y (%d) y muero\n", getpid());
					exit(0);
				}
				z = fork();
				if(z == 0){
					printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n", getpid(), b, a, arb);
					signal(SIGALRM, nada); // ME PREPARO PARA LA SEÑAL, SIGNAL SI ME LLEGA SIGARLM EJECUTARE EL METODO => nada
					alarm(tiempo);	// a loh dieh segundoh te va a llegar una señalica que se llama SIGALRM.
					pause(); 	// me duermo
					kill(a, SIGUSR1);
					signal(SIGUSR2, nada);
					pause();
					printf("Soy Z (%d) y muero\n", getpid());
					exit(0);
				}
				signal(SIGUSR2, nada);
				pause(); // me espero a la destruccion.
				printf("B manda señales a hijos\n");
				kill(x, SIGUSR2); // x se despierte y muerre
				wait(NULL); // me espero a x se muera
				kill(y, SIGUSR2);
				wait(NULL);
				kill(z, SIGUSR2);
				wait(NULL);
				exit(0); // B muere
			}
		}
	}
	return 0;
}


