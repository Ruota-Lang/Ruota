#include "Ruota.h"

#ifdef _WIN32
	std::vector<SOCKET> RuotaWrapper::sockets = {};
#endif

const char * os_compiled = {
	#include "compiled/System.ruo"
};

std::vector<SP_Memory> __system(std::vector<SP_Memory> args) {
	return { new_memory(NUM, system(args[0]->toString().c_str())) };
}

std::vector<SP_Memory> __exit(std::vector<SP_Memory> args) {
	quick_exit(0);
	return { new_memory() }; //superfluous?
}

std::vector<SP_Memory> __random(std::vector<SP_Memory> args) {
	return { new_memory(NUM, (long double)rand() / RAND_MAX) };
}

std::vector<SP_Memory> __floor(std::vector<SP_Memory> args) {
	return { new_memory(NUM, std::floor(args[0]->getValue())) };
}

std::vector<SP_Memory> __raw_file(std::vector<SP_Memory> args) {
	std::ifstream myfile(args[0]->toString());
	std::string line = "";
	std::string content = "";
	if (myfile.is_open()){
		while (getline(myfile, line)){
			content += line + "\n";
		}
		myfile.close();
	} else {
		throw std::runtime_error("Error: cannot open file " + args[1]->toString() + "!");
	}
	return { new_memory(content) };
}

std::vector<SP_Memory> __milli(std::vector<SP_Memory> args) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	return { new_memory(NUM, timeMillis) };
}

std::vector<SP_Memory> __regex_search(std::vector<SP_Memory> args) {
	String s = args[0]->toString();
	std::regex e(args[1]->toString());
	std::smatch m;
	VEC_Memory ret;
	while (std::regex_search(s,m,e)){
		VEC_Memory curr;
		for (auto x : m) {
			curr.push_back(new_memory(x.str()));
		}
		ret.push_back(new_memory(curr));
		s = m.suffix().str();
	}
	return ret;
}

std::vector<SP_Memory> __regex_replace(std::vector<SP_Memory> args) {
	String s = args[0]->toString();
	String r = args[1]->toString();
	std::regex e(args[2]->toString());
	std::string result;
	std::regex_replace(std::back_inserter(result), s.begin(), s.end(), e, r);
	return {new_memory(result)};
}

#ifdef _WIN32
std::vector<SP_Memory> __winsock_start(std::vector<SP_Memory> args) {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR) {
			return {new_memory("WSAStartup failed: " + std::to_string(iResult)) };
		}
	return {new_memory()};
}
std::vector<SP_Memory> __winsock_create_socket(std::vector<SP_Memory> args) {
	SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        WSACleanup();
        return {new_memory("Error at socket(): " + std::to_string(WSAGetLastError()) )};
    }
	RuotaWrapper::sockets.push_back(ConnectSocket);
	return {new_memory(NUM, RuotaWrapper::sockets.size() - 1)};
}
std::vector<SP_Memory> __winsock_connect(std::vector<SP_Memory> args) {
    struct sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(args[1]->toString().c_str());
    clientService.sin_port = htons(args[2]->getValue());
	SOCKET ConnectSocket = RuotaWrapper::sockets[args[0]->getValue()];
	int iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        WSACleanup();
        return {new_memory("Unable to connect to server: " + std::to_string(WSAGetLastError()) )};
    }
	return {new_memory()};
}
std::vector<SP_Memory> __winsock_send(std::vector<SP_Memory> args) {
	const char * sendbuf = args[1]->toString().c_str();
	SOCKET ConnectSocket = RuotaWrapper::sockets[args[0]->getValue()];
	int iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        WSACleanup();
        return {new_memory("Send failed: " + std::to_string(WSAGetLastError()) )};
        closesocket(ConnectSocket);
    }
	return {new_memory()};
}
std::vector<SP_Memory> __winsock_receive(std::vector<SP_Memory> args) {
	SP_Lambda callback = args[1]->getLambda();
	SOCKET ConnectSocket = RuotaWrapper::sockets[args[0]->getValue()];
	char recvbuf[512];
	int recvbuflen = 512;
	int iResult = -1;
	do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
			callback->execute({new_memory(std::string(recvbuf, recvbuflen))});
        /*else if ( iResult == 0 )
            return {new_memory("Connection closed")};
        else
            return {new_memory("Receive Failure: " + std::to_string(WSAGetLastError()))};*/
    } while( iResult > 0 );
	return {new_memory()};
}

std::vector<SP_Memory> __winsock_shutdown(std::vector<SP_Memory> args) {
	SOCKET ConnectSocket = RuotaWrapper::sockets[args[0]->getValue()];
	int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
		return {new_memory("Shutdown Failure: " + std::to_string(WSAGetLastError()))};
    }
	return {new_memory()};
}
#endif

RuotaWrapper::RuotaWrapper(String current_dir){

	Interpreter::addEmbed("console.system", &__system);
	Interpreter::addEmbed("console.exit", &__exit);
	Interpreter::addEmbed("console.random", &__random);
	Interpreter::addEmbed("console.floor", &__floor);
	Interpreter::addEmbed("console.raw_file", &__raw_file);
	Interpreter::addEmbed("console.milli", &__milli);
	Interpreter::addEmbed("regex.search", &__regex_search);
	Interpreter::addEmbed("regex.replace", &__regex_replace);
	#ifdef _WIN32
		Interpreter::addEmbed("winsock.start", &__winsock_start);
		Interpreter::addEmbed("winsock.create_socket", &__winsock_create_socket);
		Interpreter::addEmbed("winsock.connect", &__winsock_connect);
		Interpreter::addEmbed("winsock.send", &__winsock_send);
		Interpreter::addEmbed("winsock.receive", &__winsock_receive);
		Interpreter::addEmbed("winsock.shutdown", &__winsock_shutdown);
	#endif
	this->current_dir = current_dir;
	while (this->current_dir.back() != '\\') {
		this->current_dir.pop_back();
	}
	this->interpreter = new Interpreter(this->current_dir);
	main_scope = new_scope(nullptr);
	this->interpreter->generate(os_compiled, main_scope, "\\");
	this->interpreter->execute(main_scope);
}


SP_Memory RuotaWrapper::runLine(String line) {
	this->interpreter->generate(line , main_scope, "\\");
	return this->interpreter->execute(main_scope);
}