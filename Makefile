all: jchat_server jchat_client

jchat_server:
	make -f Makefile_server build

jchat_client:
	make -f Makefile_client build

clean:
	rm -f build/jchat_server build/jchat_client
	find ./ -name \*.o -type f -delete

test_server:
	./build/prg_jchat_server

test_client:
	./build/prg_jchat_client

install:
	mkdir -p bin
	cp build/jchat_server bin/jchat_server
	cp build/jchat_client bin/jchat_client

uninstall:
	rm -rf bin

.PHONY: all jchat_server jchat_client clean test_server test_client install uninstall
