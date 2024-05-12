# Práctica_final_SD
Guía de compilación y ejecución de la aplicación distribuida.

## Pasos de compilación
- Compila las RPC con el comando *make -f Makefile.info* en el directorio de las RPC.
- Compila el servidor con el comando *make* en el directorio principal.

## Pasos de ejecución
- Exporta la variable de entorno *LD_LIBRARY_PATH* con el path absoluto del directorio principal.
- Ejecuta el servicio web con el comando *pyhton3 servicio_web* en el directorio principal.
- Ejecuta el servidor de las RPC con el comando *./info_server* en el directorio de las RPC.
- Ejecuta el servidor con el comando */servidor -p puerto* en el directorio principal con un puerto distinto a 8080, ya que en ese se ejecuta el servicio_web.
- Ejecuta el cliente con el comando *python3 ./client.py -p puerto -s server* en el directorio principal donde puerto es el puerto donde se ejcuta el servidor y server es la IP del mismo.

Es importante que el servidor y el servidor de las RPC se ejecuten en la misma máquina, y que el servicio web y el cliente se ejecuten en la misma máquina. Estas parejas son indivisibles, y se pueden ejecutar en maquinas distintas o en la misma.
