# ChatServer
Este es un chat, el cual contempla un servidor y diferentes clientes. La idea principal es enrutar los diferentes clientes a traves del servidor, para que solo se tengan que preocupar de a quien le envian el mensaje.
Ademas se encuentra un archivo protocol.h, el cual almacena los structs que son utilizados para la comunicacion.

## Ejecucion
Para ejecutar el programa y que funcione se debe:
    1. Compilar server.c y luego ejecutar server.
    2. Compilar client.c y luego ejecutar client.
    3. Se pueden ejecutar hasta 100 diferentes clientes simultaneos.
