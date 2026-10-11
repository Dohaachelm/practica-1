	

#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


void partir(char *nombre, int tamanyo);

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Error. Uso: %s fichero tam_trozo\n", argv[0]);
	}
	else{
		partir(argv[1], atoi(argv[2])); // nombre del fichero y tamaño.
	}
	return 0;
}



void partir(char nombre[], int tamanyo){
	int i;
	int tubo[2]; // = {lectura, escritura}
	char nombreFichero[50];
	int fEntrada, fSalida;
	struct stat propFichero; 	// propiedades del fichero
	int 	numTrozos, 		// trozos que se van a generar
		numLeidos; 		// bytes leidos de la tuberia
 	char *trozoLeido;

	trozoLeido = (char *) malloc(sizeof(char) * tamanyo);	// es el vector de caracteres (bytes) que utiliza el padre para leer del fichero original.
	fEntrada = open(nombre, O_RDONLY); // PADRE: ABRE EL FICHERO EN MODO EN LECTURA.
	if(fEntrada < 0){
		printf("Error. No existe el fichero\n");
	}
	else{
		stat(nombre, &propFichero);	// le paso a stat la direccion de la variable propfichero para que me la rellene con la info del fichero.
		numTrozos = propFichero.st_size/tamanyo;	// el numero de trozos a leer = total del fichero / total del trozo
		if(propFichero.st_size % tamanyo != 0){
			numTrozos++; 	// si el tamaño del fichero no es multiplo del tamaño del trozo, hace falta un trozo para el resto... :)
		}
		for(i = 0; i < numTrozos; i++){
			pipe(tubo); 		// crea la tuberia distinta para cada hijo.
			if(fork() != 0){
				numLeidos = read(fEntrada, trozoLeido, tamanyo); 	// el padre lee del fichero
				write(tubo[1], trozoLeido, numLeidos);			// y escribe en el tubo.
			}
			else{

				// creo el nombre del fichero nombre.h00, nombre
				if(i < 10){ // si es menor que 10, le pongo un 0 delante.
					sprintf(nombreFichero, "%s.h0%d", nombre, i);
				}
				else{
					sprintf(nombreFichero, "%s.h%d", nombre, i);
				}
				fSalida = creat(nombreFichero, 0666);
				// aqui habria que hacer un bucle hasta que el hijo consiguiera leer
				// del tubo todo lo que tiene.
				//////////////////// bucle mientras que no lea todos los que queria leer.
				numLeidos = read(tubo[0], trozoLeido, tamanyo);		// lee del tubo
				write(fSalida, trozoLeido, numLeidos);			// escribe en el fichero
				/////////////////////
				close(fSalida);
				free(trozoLeido);
				break;
			}
		}
		if(i == numTrozos){
			free(trozoLeido);
			// bucle en el que el padre espera a los hijos.
			for(int i = 0; i < numTrozos; i++){
				wait(NULL);
			}
		}
	}
}

