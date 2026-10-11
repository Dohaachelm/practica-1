#include <stdio.h>// incluye funciones de entrada y salida
#include <sys/wait.h>//incluye wait, que permite un proceso de esperar a que termine uno de sus hijos
#include <unistd.h>//incluye fork, pause y otras
#include <stdlib.h>//incluye atoi y exit

void nada(){//no hace nada, maneja la alarma: cuando llega sigalrm, se ejecuta esta funcion vacia
}
//argc indica cuantos argumentos se han escrito
//argv contiene esos argumentos como texto
int main(int argc, char *argv[]){
	int x, y, i, j, pid;//pid guarda el resultafo de fork

	if(argc == 3){//comprueba que se hayan escrito dos argumentos, ademas del nombre del programa
		x = atoi(argv[1]);//convierte los dos argumentos de texto en numeros enteros
		y = atoi(argv[2]);
		if(x > 0 && y > 0){//continua solo si ambos numeros son mayores que cero
			for(i = 1; i <= y; i++){//repite el bucle y veces, cada repeticion intenta crear una rama nueva desde el proceso original
				pid = fork();//fork duplica el proceso:
				//el padre, recibe en pid el numero identificador del hijo, que es positivo
				// el hijo, que recibe 0
				if(pid == 0){//etse bloque solo lo ejecuta el hijo recien creado
					for(j = 1; j <= x - 1; j++){//el hijo intenta crear procesos adicionales para completar la cadena de esa rama.Como el proceso de la rama ya existe, crea hasta x-1 mas.
						pid = fork();// el proceso de la cadena crea otro proceso hijo
						if(pid !=  0){//esta condicion la cumple el proceso que acaba de llamar a fork y recibio un valor distinto de cero: el padre. El nuevo hijo recibio cero y sigue con la siguiente vuelta del bucle
							wait(NULL);// el padre espera hasta que termine su hijoo
							exit(0);//cuando el hijo termina,el padre de esta parte de la cadena tambien termina. Asi se evita que ese proceso siga creando mas ramas
						}
					}
					if(j == x){//se compruba si j ha llegado al valor x, en el proceso que ha ido creando la cadena. esta condicion identifica la hoja, es decir, el ultimo proceso de esa rama.
						signal(SIGALRM, nada);//ejecuta funcion nada
						alarm(10);//10 seg
						pause();//deja el programa detenido hasra que reciba una señal
						exit(0);//cunado pause termina, el proceso hoja termina
					}
				}
			}
			if(i == y + 1){//esta condicion se cumple al acabar el bucle exterior:mi ha llegado a y+1. Solo el proceso original llega a esra parte, porque cada hijo sigue el bloque de pid==0
				for(i = 1; i <= y; i++){//el proceso original espera a que terminen sus y hijps directos, es decir, las ramas que creó al principio
					wait(NULL);
				}
			}
		}
	}

	return 0;
}

