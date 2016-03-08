.PHONY: all jchat_server jchat_client clean test_server test_client install \
	uninstall

all: jchat_server jchat_client

jchat_server: jchat_server/src/main.cpp jchat_server/src/core/chat_server.cpp
	mkdir -p build
	g++ -g -pthread -std=c++11 -o build/jchat_server \
	-I jchat_lib/ \
	-I jchat_common/ \
	-I jchat_server/include/ \
	jchat_server/src/core/chat_server.cpp \
	jchat_server/src/main.cpp

jchat_client: jchat_client/src/main.cpp
	mkdir -p build
	g++ -g -pthread -std=c++11 -o build/jchat_client \
	-I jchat_lib/ \
	-I jchat_common/ \
	-I jchat_client/include/ \
	jchat_client/src/core/chat_client.cpp \
	jchat_client/src/main.cpp

clean:
	rm -f build/jchat_server build/jchat_client

test_server:
	./build/jchat_server

test_client:
	./build/jchat_client

install:
	mkdir -p bin
	cp build/jchat_server bin/jchat_server
	cp build/jchat_client bin/jchat_client

uninstall:
	rm -rf bin
