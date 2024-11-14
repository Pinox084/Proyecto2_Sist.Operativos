# Proyecto2_Sist.Operativos

1) Monitor de Multiples Hebras en el contexto de Productor-Consumidor.
2) Simulador de Memoria Virtual con mecanismos de reemplazo.

Estudiantes: 
Nicolás Pino Leal
Cesar Franco Mindiola
Nicolás Jarpa Jeldres
Pedro Gajardo Garfe

INSTRUCCIONES DE USO

#1
Para compilar el Monitor se debe ejecutar el archivo Monitor.cpp

Ejemplo de Compilación: g++ Monitor.cpp -o test

Para ejecutar el programa debe recibir 4 datos de entrada: Numero de productores, numero de consumidores, tamaño inicial de la cola y tiempo maximo de espera de consumidores en segundos.

Ejemplo de ejecución ./test -p 10 -c 5 -s 50 -t 1


#2

Para compilar el Simulador se debe ejecutar el archivo SimuladorMemoria.cpp
El simulador puede elegir entre los siguientes 4 algoritmos de reemplazo:
Optimo
FIFO
Least Recently Used(LRU)
Least Recently Used con ReLoj simple (LRUR)

Ejemplo de Compilación: g++ SimuladorMemoria.cpp -o test

Para ejecutar el programa debe ya de existir un archivo llamado referencias.txt con la secuencia que se va a analizar. (Ejemplo: 1 4 5 2 2 1 3)
Se debe especificar que algoritmo se quiere usar.

Ejemplo de ejecución ./test -m 3 -a LRU -f referencias.txt 

