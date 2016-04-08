ifndef GXX
	GXX=g++
endif

all: main
debug: FLAGS = -g -o0
debug: main

main:
	$(GXX) $(FLAGS) -Wall -o argyle -O2 Server.cpp Connection.cpp main.cpp HttpRequest.cpp HttpResponse.cpp DataReader.cpp FilenameResolver.cpp FileReader.cpp ErrorReader.cpp Logger.cpp Utils.cpp Config.cpp
