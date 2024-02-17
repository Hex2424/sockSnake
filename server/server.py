from socket import *
import select

socketList = []
PLAYERS_WAIT_TO_JOIN = 3


def addToConnectionLoop(connSocket, host):
    socketList.append(connSocket)


def spiderMom():
    gameIsRunning = True
    for socket in socketList:
        byti = "s".encode(encoding='utf-8')
        socket.send(byti)

    while gameIsRunning:
        read_sockets, write_sockets, error_sockets = select.select(socketList, [], [])
        for socket in read_sockets:
            buffer = None
            try:
                buffer = socket.recv(1024)
                # print(len(buffer), "\n")
            except ConnectionResetError:
                socketList.remove(socket)
                # print("removing socket")
                continue
            # print(buffer)
            for socket2 in socketList:
                if socket != socket2:
                    # print(buffer)
                    socket2.send(buffer)


def initSocketWorkflow():
    while True:
        socketList.clear()
        with socket(AF_INET, SOCK_STREAM) as s:
            s.bind(('127.0.0.1', 4546))
            while len(socketList) < PLAYERS_WAIT_TO_JOIN:
                    print("truing")
                    s.listen()
                    sock, host = s.accept()
                    print("accepted socket")
                    addToConnectionLoop(sock, host)
            try:
                spiderMom()
            except:
                pass


initSocketWorkflow()
00000000 00000000
