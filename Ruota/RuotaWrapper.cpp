#include "Ruota.h"

const char * os_compiled = {
	#include "compiled/System.ruo"
};

std::vector<SP_Memory> __error(std::vector<SP_Memory> args) {
	throw std::runtime_error(args[0]->toString());
}

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

std::vector<SP_Memory> __file_open(std::vector<SP_Memory> args) {
	String fname = Interpreter::path.substr(1) + args[0]->toString();
	while (fname[0] == '\\') fname = fname.substr(1);
	std::ifstream *file = new std::ifstream(fname);
	if (!file->is_open()){
		throw std::runtime_error("Error: cannot open file " + fname + "!");
	}
	return { new_memory((void*)file) };
}

std::vector<SP_Memory> __filew_open(std::vector<SP_Memory> args) {
	String fname = Interpreter::path.substr(1) + args[0]->toString();
	while (fname[0] == '\\') fname = fname.substr(1);
	std::ofstream *file = new std::ofstream(fname, args[1]->getValue() == 'a' ? std::ofstream::app : std::ofstream::out);
	if (!file->is_open()){
		throw std::runtime_error("Error: cannot open file " + fname + "!");
	}
	return { new_memory((void*)file) };
}

std::vector<SP_Memory> __file_close(std::vector<SP_Memory> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	file->close();
	delete file;
	return { new_memory() };
}

std::vector<SP_Memory> __filew_close(std::vector<SP_Memory> args) {
	std::ofstream *file = (std::ofstream*)args[0]->getPointer();
	file->close();
	delete file;
	return { new_memory() };
}

std::vector<SP_Memory> __filew_write(std::vector<SP_Memory> args) {
	std::ofstream *file = (std::ofstream*)args[0]->getPointer();
	*file << args[1]->toString();
	return { new_memory() };
}

std::vector<SP_Memory> __file_read_line(std::vector<SP_Memory> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	std::string line;
	if (getline(*file, line))
		return { new_memory(line) };
	else
		return { new_memory() };
}

std::vector<SP_Memory> __file_read(std::vector<SP_Memory> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	char * buffer = new char[args[1]->getValue()];
	if (file->read(buffer, args[1]->getValue()))
		return { new_memory(std::string(buffer, args[1]->getValue())) };
	else
		return { new_memory() };
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

std::vector<SP_Memory> __filesystem_listdir(std::vector<SP_Memory> args) {
	VEC_Memory list;
	String path = args[0]->toString();
	for (auto &p : std::filesystem::directory_iterator(path)){
		String file = p.path().string();
		list.push_back(new_memory(file));
	}
	return list;
}

std::vector<SP_Memory> __filesystem_mkdir(std::vector<SP_Memory> args) {
	std::filesystem::create_directory(args[0]->toString());
	return {new_memory()};
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
	return {new_memory(), new_memory((void*)ConnectSocket)};
}
std::vector<SP_Memory> __winsock_get_addresses(std::vector<SP_Memory> args) {	
	VEC_Memory ret;
	struct hostent *remoteHost;
	remoteHost = gethostbyname(args[0]->toString().c_str());
	struct in_addr addr;
	int i = 0;
	while (remoteHost->h_addr_list[i] != 0)
	{
		addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
		ret.push_back(new_memory(std::string(inet_ntoa(addr))));
	}
	return ret;
}
std::vector<SP_Memory> __winsock_connect(std::vector<SP_Memory> args) {
    struct sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(args[1]->toString().c_str());
    clientService.sin_port = htons(args[2]->getValue());
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
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
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        WSACleanup();
        closesocket(ConnectSocket);
        return {new_memory("Send failed: " + std::to_string(WSAGetLastError()) )};
    }
	return {new_memory()};
}
std::vector<SP_Memory> __winsock_listen(std::vector<SP_Memory> args) {
	SP_Lambda callback = args[2]->getLambda();
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult;
	int recvbuflen = args[1]->getValue();
	do {	
		char * recvbuf = new char[args[1]->getValue()];
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
			callback->execute({new_memory(std::string(recvbuf))});
        else if ( iResult == 0 )
            throw std::runtime_error("Connection closed");
        else
            throw std::runtime_error("Receive Failure: " + std::to_string(WSAGetLastError()));
    } while( iResult > 0 );
	return {new_memory()};
}

std::vector<SP_Memory> __winsock_receive(std::vector<SP_Memory> args) {
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	char * recvbuf = new char[args[1]->getValue()];
	int recvbuflen = args[1]->getValue();
	int iResult;
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if ( iResult > 0 )
		return {new_memory(std::string(recvbuf))};
	else if ( iResult == 0 )
		throw std::runtime_error("Connection closed");
	else
		throw std::runtime_error("Receive Failure: " + std::to_string(WSAGetLastError()));
}

std::vector<SP_Memory> __winsock_shutdown(std::vector<SP_Memory> args) {
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
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
	Interpreter::addEmbed("error", &__error);
	Interpreter::addEmbed("console.system", &__system);
	Interpreter::addEmbed("console.exit", &__exit);
	Interpreter::addEmbed("console.random", &__random);
	Interpreter::addEmbed("console.floor", &__floor);
	Interpreter::addEmbed("console.milli", &__milli);
	Interpreter::addEmbed("regex.search", &__regex_search);
	Interpreter::addEmbed("regex.replace", &__regex_replace);
	Interpreter::addEmbed("filesystem.listdir", &__filesystem_listdir);
	Interpreter::addEmbed("filesystem.mkdir", &__filesystem_mkdir);
	#ifdef _WIN32
		Interpreter::addEmbed("winsock.start", &__winsock_start);
		Interpreter::addEmbed("winsock.create_socket", &__winsock_create_socket);
		Interpreter::addEmbed("winsock.connect", &__winsock_connect);
		Interpreter::addEmbed("winsock.send", &__winsock_send);
		Interpreter::addEmbed("winsock.listen", &__winsock_listen);
		Interpreter::addEmbed("winsock.receive", &__winsock_receive);
		Interpreter::addEmbed("winsock.get_addresses", &__winsock_get_addresses);
		Interpreter::addEmbed("winsock.shutdown", &__winsock_shutdown);
	#endif
	#ifdef FILE_IO
		Interpreter::addEmbed("file.open", &__file_open);
		Interpreter::addEmbed("file.close", &__file_close);
		Interpreter::addEmbed("file.read_line", &__file_read_line);
		Interpreter::addEmbed("file.read", &__file_read);
		Interpreter::addEmbed("filew.open", &__filew_open);
		Interpreter::addEmbed("filew.close", &__filew_close);
		Interpreter::addEmbed("filew.write", &__filew_write);
	#endif
	this->current_dir = current_dir;
	while (this->current_dir.back() != '\\' && this->current_dir.back() != '/') {
		this->current_dir.pop_back();
		if (this->current_dir.empty())
			break;
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