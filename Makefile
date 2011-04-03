all:
	g++ -Wall -o argyle -O2 Server.cpp Connection.cpp main.cpp HttpRequest.cpp HttpResponse.cpp DataReader.cpp FilenameResolver.cpp FileReader.cpp ErrorReader.cpp Logger.cpp Utils.cpp Config.cpp
