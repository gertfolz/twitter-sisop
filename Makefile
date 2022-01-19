CC = g++
STD=-std=c++17

SRC := src/
INC := include/
BIN := bin/
OBJ := obj/
LIB := -lpthread -lsqlite3
FLAG := -Wall

all: ${OBJ}serverApp.o ${OBJ}clientApp.o ${OBJ}database.o ${OBJ}message.o ${OBJ}user.o ${OBJ}follow.o ${OBJ}notification.o
	mkdir -p $(BIN)
	${CC} ${STD} serverApp.o server.o database.o message.o user.o follow.o notification.o -o ${BIN}server ${LIB} ${FLAG}
	${CC} ${STD} clientApp.o client.o database.o message.o user.o follow.o notification.o -o ${BIN}client ${LIB} ${FLAG}

${OBJ}serverApp.o: ${SRC}serverApp.cpp ${SRC}server.cpp ${INC}server.hpp
	${CC} ${STD} -c ${SRC}serverApp.cpp ${SRC}server.cpp -I ${INC} ${FLAG}

${OBJ}clientApp.o: ${SRC}clientApp.cpp ${SRC}client.cpp ${INC}client.hpp
	${CC} ${STD} -c ${SRC}clientApp.cpp ${SRC}client.cpp -I ${INC} ${FLAG}

${OBJ}database.o: ${SRC}database.cpp ${INC}database.hpp
	${CC} ${STD} -c ${SRC}database.cpp -I ${INC} ${FLAG}

${OBJ}message.o: ${SRC}message.cpp ${INC}message.hpp
	${CC} ${STD} -c ${SRC}message.cpp -I ${INC} ${FLAG}

${OBJ}user.o: ${SRC}user.cpp ${INC}user.hpp
	${CC} ${STD} -c ${SRC}user.cpp -I ${INC} ${FLAG}

${OBJ}follow.o: ${SRC}follow.cpp ${INC}follow.hpp
	${CC} ${STD} -c ${SRC}follow.cpp -I ${INC} ${FLAG}

${OBJ}notification.o: ${SRC}notification.cpp ${INC}notification.hpp
	${CC} ${STD} -c ${SRC}notification.cpp -I ${INC} ${FLAG}

clean:
	rm ${BIN}server ${BIN}client *.o 

.PHONY: all clean