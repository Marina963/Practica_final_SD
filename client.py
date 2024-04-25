from enum import Enum
import threading
import argparse
import socket

class client:

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum):
        OK = 0
        ERROR_1 = 1
        ERROR_2 = 2
        ERROR_3 = 3
        ERROR_4 = 4

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _user = ""
    _server_socket = None
    _wait = 0

    # ******************** METHODS *******************
    @staticmethod
    def wait_socket_connect():
        while(client._wait):
            newsd = client._server_socket.accept()[0]
            

    @staticmethod
    def register(user):
        # Función de registro de cliente
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR_2

        try:
            client.write_string(sd, b'REGISTER\0')
            client.write_string(sd, (user+'\0').encode())
            respuesta = ''

            respuesta = client.read_string(sd)
            if respuesta ==  '0':
                res = client.RC.OK
            elif respuesta == '1':
                res = client.RC.ERROR_1
            elif respuesta == '2':
                res = client.RC.ERROR_2
        finally:
            sd.close()
            return res

    @staticmethod
    def unregister(user):
        # Función de baja de registro de cliente
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR_2

        try:
            client.write_string(sd, b'UNREGISTER\0')
            client.write_string(sd, (user+'\0').encode())
           
            respuesta = ''

            respuesta = client.read_string(sd)
            if respuesta ==  '0':
                res = client.RC.OK
                
            elif respuesta == '1':
                res = client.RC.ERROR_1
                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                
        finally:
            sd.close()
            return res


    @staticmethod
    def connect(user):
        # Función de connect de cliente
        client._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client._server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        client._server_socket.bind(('localhost',0))
        client._server_socket.listen(8)

        client._wait = 0
        t = threading.Thread(target=client.wait_socket_connect)
        t.start()

        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR_3

        try:
            client.write_string(sd, b'CONNECT\0')
            client.write_string(sd, (user+'\0').encode())
            client.write_string(sd, (str(client._server_socket.getsockname()[1] ) + '\0').encode())
            respuesta = ''
            
            respuesta = client.read_string(sd)
          
            if respuesta ==  '0':
                res = client.RC.OK
                
            elif respuesta == '1':
                res = client.RC.ERROR_1
                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                
            elif respuesta == b'3':
                res = client.RC.ERROR_3
                
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
        res = client.RC.ERROR_3

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'DISCONNECT\0')
            client.write_string(sd, (user+'\0').encode())
            respuesta = ''

            # Se espera a la respuesta
            respuesta = client.read_string(sd)
             
            if respuesta ==  '0':
                res = client.RC.OK
                
            elif respuesta == '1':
                res = client.RC.ERROR_1
                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                
            elif respuesta == '3':
                res = client.RC.ERROR_3
                
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
        res = client.RC.ERROR_4

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'PUBLISH\0')
            client.write_string(sd, (client._user+'\0').encode())
            client.write_string(sd, (fileName + '\0').encode())
            client.write_string(sd, (description + '\0').encode())

            respuesta = ''

            # Se espera a la respuesta
            respuesta = client.read_string(sd)
        
                
            if respuesta ==  '0':
                res = client.RC.OK
                
            elif respuesta == '1':
                res = client.RC.ERROR_1
                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                
            elif respuesta == '3':
                res = client.RC.ERROR_3
                
            elif respuesta == '4':
                res = client.RC.ERROR_4
                
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
        res = client.RC.ERROR_1

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'DELETE\0')
            client.write_string(sd, (client._user+'\0').encode())
            client.write_string(sd, (fileName + '\0').encode())

            respuesta = ''

            # Se espera a la respuesta
          
            respuesta =client.read_string(sd)
            if respuesta ==  '0':
                res = client.RC.OK
                
            elif respuesta == '1':
                res = client.RC.ERROR_1

            elif respuesta == '2':
                res = client.RC.ERROR_2
                
            elif respuesta == '3':
                res = client.RC.ERROR_3
                
            elif respuesta == '4':
                res = client.RC.ERROR_4
                    
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
        res = client.RC.ERROR_3

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'LIST_USERS\0')
            client.write_string(sd, (client._user+'\0').encode())
            respuesta = ''

            # Se espera a la respuesta
            
            respuesta = client.read_string(sd)
            
            if respuesta ==  '0':
                res = client.RC.OK
                n_user = client.read_string(sd)
                
                print(n_user)
                lista = {}
                for i in range (n_user):
                    username= client.read_string(sd)
                    ip_user = client.read_string(sd)
                    port_user = client.read_number(sd)
                    lista[username] = (ip_user, port_user)
                
                print(lista)
                
            elif respuesta == '1':
                res = client.RC.ERROR_1
                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                
            elif respuesta == '3':
                res = client.RC.ERROR_3
                   
        finally:
            # Se cierra el socket
            sd.close()
            return res


    @staticmethod
    def  listcontent(user) :
       # Se crea el socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)

        # Se inicializa la respuesta
        res = client.RC.ERROR_1

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'LIST_CONTENT\0')
            client.write_string(sd, (user+'\0').encode())
           

            respuesta = ''

            # Se espera a la respuesta
          
            respuesta =client.read_string(sd)
            if respuesta ==  '0':
                res = client.RC.OK
                
            elif respuesta == '1':
                res = client.RC.ERROR_1

            elif respuesta == '2':
                res = client.RC.ERROR_2
                
            elif respuesta == '3':
                res = client.RC.ERROR_3
                
            elif respuesta == '4':
                res = client.RC.ERROR_4
                    
        finally:
            # Se cierra el socket
            sd.close()
            return res

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
                            resultado = client.register(line[1]) 
                            if resultado == client.RC.OK:
                                print("c> REGISTER OK")
                            elif resultado == client.RC.ERROR_1:
                                print("c> USERNAME IN USE")
                            elif resultado == client.RC.ERROR_2:
                                print("c> REGISTER FAIL")
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            resultado = client.unregister(line[1]) 
                            if resultado == client.RC.OK:
                                print("c> UNREGISTER OK")
                            elif resultado == client.RC.ERROR_1:
                                print("c> USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> UNREGISTER FAIL")
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client._user = line[1]
                            resultado = client.connect(line[1])
                            if resultado == client.RC.OK:
                                print("c> CONNECT OK")
                            elif resultado == client.RC.ERROR_1:
                                print("c> CONNECT FAIL, USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> USER ALREADY CONNECTED")
                            elif resultado == client.RC.ERROR_3:
                                print("c> CONNECT FAIL")
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
                            resultado = client.disconnect(line[1])
                            if resultado == client.RC.OK:
                            	print("c> DISCONNECT OK")
                            elif resultado == client.RC.ERROR_1:
                            	print("c> DISCONNECT FAIL / USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                            	print("c> DISCONNECT FAIL / USER NOT CONNECTED")
                            elif resultado == client.RC.ERROR_3:
                            	print("c> DISCONNECT FAIL")
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
            return False
        
        client._server = args.s
        client._port = args.p

        return True

    @staticmethod
    def read_string(sock):
        str = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break
            str += msg.decode()
            return str
        
    @staticmethod
    def write_string(sock, str):
        sock.sendall(str)
        
    @staticmethod
    def read_number(sock):
        a = client.read_string(sock)
        return(int(a,10))

    @staticmethod
    def write_number(sock, num):
        a = str(num) + b'\0'
        client.write_string(sock, a)

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
