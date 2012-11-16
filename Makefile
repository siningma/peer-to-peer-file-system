CC = g++
CFLAGS = -Wall
LDFLAGS = -L/home/scf-22/csci551b/openssl/lib -lcrypto -lpthread -lsocket -lnsl -lresolv
IFLAGS =  -I/home/scf-22/csci551b/openssl/include

all: sv_node

sv_node: node.o tool.o bitVector.o meta.o fileSystem.o message.o main.o join.o log.o handler.o handler2.o checkMsg.o checkResMsg.o helloMsg.o joinMsg.o joinResMsg.o notifyMsg.o statusMsg.o statusResMsg.o storeMsg.o searchMsg.o searchResMsg.o getMsg.o getResMsg.o deleteMsg.o
	$(CC) $(CFLAGS) -o sv_node node.o tool.o bitVector.o meta.o fileSystem.o message.o main.o join.o log.o handler.o handler2.o checkMsg.o checkResMsg.o helloMsg.o joinMsg.o joinResMsg.o notifyMsg.o statusMsg.o statusResMsg.o storeMsg.o searchMsg.o searchResMsg.o getMsg.o getResMsg.o deleteMsg.o $(LDFLAGS)	

main.o: system/main.cpp system/node.h system/tool.h protocol/message.h
	$(CC) $(CFLAGS) -c system/main.cpp $(IFLAGS)

handler.o: system/handler.cpp system/node.h system/tool.h system/log.h system/helper.h protocol/message.h protocol/checkMsg.h protocol/checkResMsg.h protocol/helloMsg.h protocol/joinMsg.h protocol/joinResMsg.h protocol/notifyMsg.h protocol/statusMsg.h
	$(CC) $(CFLAGS) -c system/handler.cpp $(IFLAGS)
	
handler2.o: system/handler2.cpp system/node.h system/tool.h system/log.h system/helper.h protocol/message.h protocol/deleteMsg.h protocol/getMsg.h protocol/getResMsg.h protocol/deleteMsg.h protocol/searchMsg.h protocol/searchResMsg.h protocol/storeMsg.h 
	$(CC) $(CFLAGS) -c system/handler2.cpp $(IFLAGS)

join.o: system/join.cpp system/node.h system/tool.h system/log.h system/helper.h protocol/message.h protocol/checkMsg.h protocol/checkResMsg.h protocol/helloMsg.h protocol/joinMsg.h protocol/joinResMsg.h protocol/notifyMsg.h protocol/statusMsg.h
	$(CC) $(CFLAGS) -c system/join.cpp $(IFLAGS)
	
node.o: system/node.cpp system/node.h filesys/fileSystem.h system/tool.h system/log.h system/helper.h protocol/message.h protocol/checkMsg.h protocol/checkResMsg.h protocol/helloMsg.h protocol/joinMsg.h protocol/joinResMsg.h protocol/notifyMsg.h protocol/statusMsg.h filesys/fileSystem.h protocol/deleteMsg.h protocol/getMsg.h protocol/getResMsg.h protocol/deleteMsg.h protocol/searchMsg.h protocol/searchResMsg.h protocol/storeMsg.h 
	$(CC) $(CFLAGS) -c system/node.cpp $(IFLAGS)
	
log.o: system/log.cpp system/log.h protocol/message.h
	$(CC) $(CFLAGS) -c system/log.cpp $(IFLAGS)
	
tool.o:	system/tool.cpp system/tool.h
	$(CC) $(CFLAGS) -c system/tool.cpp $(IFLAGS)

bitVector.o: filesys/bitVector.cpp filesys/bitVector.h system/tool.h 
	$(CC) $(CFLAGS) -c filesys/bitVector.cpp $(IFLAGS)

meta.o:	filesys/meta.cpp filesys/meta.h filesys/bitVector.h system/tool.h
	$(CC) $(CFLAGS) -c filesys/meta.cpp $(IFLAGS)

fileSystem.o: filesys/fileSystem.cpp filesys/fileSystem.h filesys/meta.h filesys/bitVector.h system/tool.h
	$(CC) $(CFLAGS) -c filesys/fileSystem.cpp $(IFLAGS)
	
message.o: protocol/message.cpp protocol/message.h system/tool.h
	$(CC) $(CFLAGS) -c protocol/message.cpp $(IFLAGS)
	
checkMsg.o: protocol/checkMsg.cpp protocol/checkMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/checkMsg.cpp $(IFLAGS)
	
checkResMsg.o: protocol/checkResMsg.cpp protocol/checkResMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/checkResMsg.cpp	$(IFLAGS)
	
helloMsg.o: protocol/helloMsg.cpp protocol/helloMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/helloMsg.cpp $(IFLAGS)
	
joinMsg.o: protocol/joinMsg.cpp protocol/joinMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/joinMsg.cpp $(IFLAGS)
	
joinResMsg.o: protocol/joinResMsg.cpp protocol/joinResMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/joinResMsg.cpp $(IFLAGS)	

notifyMsg.o: protocol/notifyMsg.cpp protocol/notifyMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/notifyMsg.cpp $(IFLAGS)
		
statusMsg.o: protocol/statusMsg.cpp protocol/statusMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/statusMsg.cpp $(IFLAGS)	
	
statusResMsg.o: protocol/statusResMsg.cpp protocol/statusResMsg.h protocol/message.h system/helper.h
	$(CC) $(CFLAGS) -c protocol/statusResMsg.cpp $(IFLAGS)
	
storeMsg.o: protocol/storeMsg.cpp protocol/storeMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/storeMsg.cpp $(IFLAGS)	
	
searchMsg.o: protocol/searchMsg.cpp protocol/searchMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/searchMsg.cpp $(IFLAGS)
	
searchResMsg.o: protocol/searchResMsg.cpp protocol/searchResMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/searchResMsg.cpp $(IFLAGS)	
	
getMsg.o: protocol/getMsg.cpp protocol/getMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/getMsg.cpp $(IFLAGS)
	
getResMsg.o: protocol/getResMsg.cpp protocol/getResMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/getResMsg.cpp $(IFLAGS)
	
deleteMsg.o: protocol/deleteMsg.cpp protocol/deleteMsg.h protocol/message.h
	$(CC) $(CFLAGS) -c protocol/deleteMsg.cpp $(IFLAGS)

clean:
	rm -f *.o
	rm -f sv_node

