import logging
from datetime import datetime
from spyne import ServiceBase, rpc, Unicode, Application
from wsgiref.simple_server import make_server
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication


class Fecha(ServiceBase):
    @rpc(_returns=Unicode)
    def get_time(ctx):
        fecha = datetime.now()
        fecha = fecha.strftime("%d/%m/%Y  %H:%M:%S")
        return fecha

application = Application(services = [Fecha], tns='http://localhost:8080', in_protocol= Soap11(), out_protocol= Soap11())
application = WsgiApplication(application)


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)
    server = make_server('localhost', 8080, application)
    server.serve_forever()