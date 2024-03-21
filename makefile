cpp=g++
PARSER=parser
DEBUG=debug
HTTP_SERVER = http_server

.PHONY:all
all:$(PARSER) $(DEBUG) $(HTTP_SERVER)

$(PARSER):parser.cpp
	$(cpp) -o $@ $^ -std=c++11 -lboost_system -lboost_filesystem
$(DEBUG):debug.cpp
	$(cpp) -o $@ $^ -std=c++11 -ljsoncpp
$(HTTP_SERVER):http_server.cpp
	$(cpp) -o $@ $^ -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -rf $(PARSER) $(DEBUG) $(HTTP_SERVER)