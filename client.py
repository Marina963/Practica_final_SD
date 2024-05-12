from enum import Enum
import threading
import argparse
import socket
import zeep
import os

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
    _info_users = None
    _content = None
    _z_client = zeep.Client(wsdl = "http://localhost:8080/?wsdl")
                            

    # ******************** METHODS *******************
    @staticmethod
    def wait_socket_connect():
        # Función que se encarga de manejar el hilo
        while(client._wait):
        
            # Acepta una petición
            newsd = client._server_socket.accept()[0]

            if (client._wait == 0):
                newsd.close()
                return
            
            # Consigue el nombre del fichero
            file_name = client.read_string(newsd)
            if (client._user == ""):
                client.write_string(newsd, b"2\0")
                newsd.close()
            else:
                # Manda el fichero
                file_name = os.path.abspath("users_files/" + client._user + "/" + file_name)
                try:
                    with open(file_name, "r") as file_in:                    
                        file_info = file_in.read()
                    res = "0\n" + file_info + "\0"
                    client.write_string(newsd, res.encode('utf-8'))
                    newsd.close()
                except:
                    client.write_string(newsd, b"1\0")
                    newsd.close()
                    

    @staticmethod
    def register(user):
        # Función de registro de cliente
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR_2

        try:
            client.write_string(sd, b'REGISTER\0')
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (user+'\0').encode('utf-8'))
            respuesta = ''

            respuesta = client.read_char(sd)
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
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (user+'\0').encode('utf-8'))
           
            respuesta = ''

            respuesta = client.read_char(sd)
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
    
        # Crea el socket para get_file y el thread   
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('localhost',0))
        server_socket.listen(8)
        
        t = threading.Thread(target=client.wait_socket_connect, daemon=True)

		# Crea el socket que se conecta al servidor
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)
        res = client.RC.ERROR_3

        try:
        	# Manda los datos
            client.write_string(sd, b'CONNECT\0')
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (user+'\0').encode('utf-8'))
            client.write_string(sd, (str(server_socket.getsockname()[1] ) + '\0').encode('utf-8'))
            respuesta = ''
            
            # Espera la respuesta
            respuesta = client.read_char(sd)
          
            if respuesta == '0':
                res = client.RC.OK
                
                # Si la respuesta es efectiva, guarda el socket y lanza el hilo
                client._wait = 1
                client._server_socket = server_socket
                t.start()
                
            elif respuesta == '1':
            	# Si la respuesta es un error, cerrará el descriptor de socket
                res = client.RC.ERROR_1
                server_socket.close()
                                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                server_socket.close()
                                
            elif respuesta == b'3':
                res = client.RC.ERROR_3
                server_socket.close()
                
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
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (user+'\0').encode('utf-8'))
            respuesta = ''

            # Se espera a la respuesta
            respuesta = client.read_char(sd)
             
            if respuesta ==  '0':
                res = client.RC.OK
                client._wait = 0
                client._server_socket(socket.SHUT_RDWR)
                client._server_socket.close()
                
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
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (client._user+'\0').encode('utf-8'))
            client.write_string(sd, (fileName + '\0').encode('utf-8'))
            client.write_string(sd, (description + '\0').encode('utf-8'))

            respuesta = ''

            # Se espera a la respuesta
            respuesta = client.read_char(sd)
                
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
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (client._user+'\0').encode('utf-8'))
            client.write_string(sd, (fileName + '\0').encode('utf-8'))

            respuesta = ''

            # Se espera a la respuesta
          
            respuesta =client.read_char(sd)
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
    def listusers() :
        # Función de disconnect de cliente
        if (client._user == ''):
            return client.RC.ERROR_2
        
        # Se crea el socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, client._port)
        sd.connect(server_address)

        # Se inicializa la respuesta
        res = client.RC.ERROR_3

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'LIST_USERS\0')
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (client._user+'\0').encode('utf-8'))
            respuesta = ''

            # Se espera a la respuesta
            respuesta = client.read_string(sd)
            
            if respuesta[0] == '0':
                res = client.RC.OK
                respuesta = respuesta.split("\n")
                
                client._info_users = {}
                n_user = int(respuesta[1])
                for i in range(n_user):
                    client._info_users[respuesta[3*i+2]] = (respuesta[3*i+3], respuesta[3*i+4])
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
        res = client.RC.ERROR_4

        try:
            # Se manda la operación y el usuario
            client.write_string(sd, b'LIST_CONTENT\0')
            client.write_string(sd, (client._z_client.service.get_time() +'\0').encode('utf-8'))
            client.write_string(sd, (client._user+'\0').encode('utf-8'))
            client.write_string(sd, (user+'\0').encode('utf-8'))
           

            respuesta = ''

            # Se espera a la respuesta
          
            respuesta = client.read_string(sd)
            
            if respuesta[0] ==  '0':
                res = client.RC.OK
                respuesta = respuesta.split("\n")
                
                client._content = {}
                n_user = int(respuesta[1])
                for i in range(n_user):
                    client._content[respuesta[i*2+2]] = respuesta[i*2+3]
                
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
    def  getfile(user, remote_FileName, local_FileName) :
        # Se crea el socket
        res = client.RC.ERROR_2
        
        found = False
        for filename in client._content.keys():
            if (remote_FileName == filename):
                found = True
        if not found:
            return res
            
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            sd.connect((client._info_users[user][0], int(client._info_users[user][1])))
        except:
            sd.close()
            return res
        
        try:
            # Se manda el fichero que se quiere obtener
            client.write_string(sd, (remote_FileName+'\0').encode('utf-8'))
            
            # Espera a una respuesta
            respuesta = ''
            respuesta = client.read_string(sd)

            if respuesta[0] == '0':
                res = client.RC.OK
                file_name = os.path.abspath("users_files/" + client._user + "/" + local_FileName)
                file_out = open(file_name, "w")
                file_out.write(respuesta[2:])
                file_out.close()
            
            elif respuesta == '1':
                res = client.RC.ERROR_1
                
            elif respuesta == '2':
                res = client.RC.ERROR_2
                
        finally:
            sd.close()
            return res

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
                            client.disconnect(line[1])
                            resultado = client.unregister(line[1]) 
                            if resultado == client.RC.OK:
                                print("c> UNREGISTER OK")
                                if (client._user != ''):
                                    client.disconnect(client._user)
                                    client._user = ''
                            elif resultado == client.RC.ERROR_1:
                                print("c> USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> UNREGISTER FAIL")
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            if (client._user != '' and client._user != line[1]):
                                client.disconnect(client._user)
                            resultado = client.connect(line[1])
                            client._user = line[1]
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
                            resultado = client.publish(line[1], description)
                            if resultado == client.RC.OK:
                                print("c> PUBLISH OK")
                            elif resultado == client.RC.ERROR_1:
                                print("c> PUBLISH FAIL, USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> PUBLISH FAIL, USER NOT CONNECTED")
                            elif resultado == client.RC.ERROR_3:
                                print("c> PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
                            elif resultado == client.RC.ERROR_4:
                                print("c> PUBLISH FAIL")
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            resultado = client.delete(line[1])
                            if resultado == client.RC.OK:
                                print("c> DELETE OK")
                            elif resultado == client.RC.ERROR_1:
                                print("c> DELETE FAIL, USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> DELETE FAIL, USER NOT CONNECTED")
                            elif resultado == client.RC.ERROR_3:
                                print("c> DELETE FAIL, CONTENT NOT PUBLISHED")
                            elif resultado == client.RC.ERROR_4:
                                print("c> DELETE FAIL")
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            resultado = client.listusers()
                            if resultado == client.RC.OK:
                                print("c> LIST_USERS OK")
                                for key in client._info_users.keys():
                                    print("\t" + key + "\t" + client._info_users[key][0]+"\t"+client._info_users[key][1])
                            elif resultado == client.RC.ERROR_1:
                                print("c> LIST_USERS FAIL, USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> LIST_USERS FAIL, USER NOT CONNECTED")
                            elif resultado == client.RC.ERROR_3:
                                print("c> LIST_USERS FAIL")
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            resultado = client.listcontent(line[1])
                            if resultado == client.RC.OK:
                                print("c> LIST_CONTENT OK")
                                for key in client._content.keys():
                                    print("\t" + key + "\t" + client._content[key])
                            elif resultado == client.RC.ERROR_1:
                                print("c> LIST_CONTENT FAIL, USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> LIST_CONTENT FAIL, USER NOT CONNECTED")
                            elif resultado == client.RC.ERROR_3:
                                print("c> LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
                            elif resultado == client.RC.ERROR_4:
                                print("c> LIST_CONTENT FAIL")
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            resultado = client.disconnect(line[1])
                            if resultado == client.RC.OK:
                                print("c> DISCONNECT OK")
                                client._user = ''
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
                            client.listusers()
                            client.listcontent(line[1])
                            resultado = client.getfile(line[1], line[2], line[3])
                            if resultado == client.RC.OK:
                                print("c> GET_FILE OK")
                            elif resultado == client.RC.ERROR_1:
                                print("c> GET_FILE FAIL / FILE NOT EXIST")
                            elif resultado == client.RC.ERROR_2:
                                print("c> GET_FILE FAIL")
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            if (client._user != ''):
                                client.disconnect(client._user)
                                client._user = ''
                            return 
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
    def read_char(sock):
        char = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break
            char += msg.decode('utf-8')
            return char
    
    @staticmethod
    def read_string(sock):
        str = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break
            str += msg.decode('utf-8')
        return str
        
    @staticmethod
    def write_string(sock, message):
        sock.sendall(message)
        
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
