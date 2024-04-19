from enum import Enum
import argparse
import socket

class client:

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum):
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _user = ""

    # ******************** METHODS *******************

    @staticmethod
    def register(user):
        # Función de registro de cliente
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR

        try:
            sd.sendall(b'REGISTER\0')
            sd.sendall((user+'\0').encode())
            respuesta = ''

            while True:
                respuesta = sd.recv(1)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
        finally:
            sd.close()
            return res

    @staticmethod
    def unregister(user):
        # Función de baja de registro de cliente
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR

        try:
            sd.sendall(b'UNREGISTER\0')
            sd.sendall((user+'\0').encode())
            respuesta = ''

            while True:
                respuesta = sd.recv(1)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
        finally:
            sd.close()
            return res


    @staticmethod
    def connect(user):
        # Función de connect de cliente
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR

        try:
            sd.sendall(b'CONNECT\0')
            sd.sendall((user+'\0').encode())
            sd.sendall((str(client._port) + '\0').encode())
            respuesta = ''

            while True:
                respuesta = sd.recv(1)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
                elif respuesta == b'3':
                    res = 3
                    break
        finally:
            sd.close()
            return res
    
    @staticmethod
    def  disconnect(user) :
        # Función de disconnect de cliente
        # Se crea el socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)

        # Se inicializa la respuesta
        res = client.RC.ERROR

        try:
            # Se manda la operación y el usuario
            sd.sendall(b'DISCONNECT\0')
            sd.sendall((user+'\0').encode())
            respuesta = ''

            # Se espera a la respuesta
            while True:
                respuesta = sd.recv(1)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
                elif respuesta == b'3':
                    res = 3
                    break
        finally:
            # Se cierra el socket
            sd.close()
            return res

    @staticmethod
    def  publish(fileName,  description) :
        # Función de publish de cliente

        # Se crea el socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)

        # Se inicializa la respuesta
        res = client.RC.ERROR

        try:
            # Se manda la operación y el usuario
            sd.sendall(b'PUBLISH\0')
            sd.sendall((client._user+'\0').encode())
            sd.sendall((fileName + '\0').encode())
            sd.sendall((description + '\0').encode())

            respuesta = ''

            # Se espera a la respuesta
            while True:
                respuesta = sd.recv(1)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
                elif respuesta == b'3':
                    res = 3
                    break
                elif respuesta == b'4':
                    res = 4
                    break
        finally:
            # Se cierra el socket
            sd.close()
            return res

    @staticmethod
    def  delete(fileName) :
        # Se crea el socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)

        # Se inicializa la respuesta
        res = client.RC.ERROR

        try:
            # Se manda la operación y el usuario
            sd.sendall(b'DELETE\0')
            sd.sendall((client._user+'\0').encode())
            sd.sendall((fileName + '\0').encode())

            respuesta = ''

            # Se espera a la respuesta
            while True:
                respuesta = sd.recv(1)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
                elif respuesta == b'3':
                    res = 3
                    break
                elif respuesta == b'4':
                    res = 4
                    break
        finally:
            # Se cierra el socket
            sd.close()
            return res

    @staticmethod
    def  listusers() :
        # Función de disconnect de cliente
        # Se crea el socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)

        # Se inicializa la respuesta
        res = client.RC.ERROR

        try:
            # Se manda la operación y el usuario
            sd.sendall(b'LIST_USERS\0')
            sd.sendall((client._user+'\0').encode())
            respuesta = ''

            # Se espera a la respuesta
            while True:
                respuesta = sd.recv(1)
                print(respuesta)
                if respuesta ==  b'0':
                    res = client.RC.OK
                    while(True):
                    	n_user = sd.recv(1)
                    	if (n_user > 0):
                    		break
                    		
                    lista = {}
                    for i in range (n_user):
                        username= (sd.recv(256)).decode()
                        ip_user = (sd.recv(32)).decode()
                        port_user = (sd.recv(8)).decode()
                        lista[username] = (ip_user, port_user)
                    break
                elif respuesta == b'1':
                    res = client.RC.ERROR
                    break
                elif respuesta == b'2':
                    res = client.RC.USER_ERROR
                    break
                elif respuesta == b'3':
                    res = 3
                    break
        finally:
            # Se cierra el socket
            sd.close()
            return res


    @staticmethod
    def  listcontent(user) :
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  getfile(user,  remote_FileName,  local_FileName) :
        #  Write your code here
        return client.RC.ERROR

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():

        while (True):
            try:
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):

                    line[0] = line[0].upper()

                    if (line[0]=="REGISTER") :
                        if (len(line) == 2) :
                            client.register(line[1])
                            print(client.register(line[1]))
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1])
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1])
                            client._user = line[1]
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1])
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            break
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")
            except Exception as e:
                print("Exception: " + str(e))

    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;
        
        client._server = args.s
        client._port = args.p

        return True


    # ******************** MAIN *********************
    @staticmethod
    def main(argv) :
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    client.main([])
