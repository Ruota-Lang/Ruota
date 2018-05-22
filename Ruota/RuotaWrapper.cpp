#include "Ruota.h"

const char * os_compiled = {
	#include "compiled/System.ruo"
};

std::vector<SP_Memory> __error(std::vector<SP_Memory> args) {
	throw std::runtime_error(ws2s(args[0]->toString()).c_str());
}

std::vector<SP_Memory> __system(std::vector<SP_Memory> args) {
	return { new_memory(NUM, system(ws2s(args[0]->toString()).c_str())) };
}

std::vector<SP_Memory> __getenv(std::vector<SP_Memory> args) {
	auto env = getenv(ws2s(args[0]->toString()).c_str());
	if (env == NULL)
		return { new_memory() };
	return { new_memory(s2ws(std::string(env))) };
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
		throw std::runtime_error(ws2s(L"Error: cannot open file " + fname + L"!"));
	}
	return { new_memory((void*)file) };
}

std::vector<SP_Memory> __filew_open(std::vector<SP_Memory> args) {
	String fname = Interpreter::path.substr(1) + args[0]->toString();
	while (fname[0] == '\\') fname = fname.substr(1);
	std::ofstream *file = new std::ofstream(fname, args[1]->getValue() == 'a' ? std::ofstream::app : std::ofstream::out);
	if (!file->is_open()){
		throw std::runtime_error(ws2s(L"Error: cannot open file " + fname + L"!"));
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
	*file << ws2s(args[1]->toString());
	return { new_memory() };
}

std::vector<SP_Memory> __file_read_line(std::vector<SP_Memory> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	std::string line;
	if (getline(*file, line))
		return { new_memory(s2ws(line)) };
	else
		return { new_memory() };
}

std::vector<SP_Memory> __file_read(std::vector<SP_Memory> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	char * buffer = new char[args[1]->getValue()];
	if (file->read(buffer, args[1]->getValue()))
		return { new_memory(s2ws(std::string(buffer, args[1]->getValue()))) };
	else
		return { new_memory() };
}

std::vector<SP_Memory> __milli(std::vector<SP_Memory> args) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	return { new_memory(NUM, timeMillis) };
}

std::vector<SP_Memory> __regex_search(std::vector<SP_Memory> args) {
	std::string s = ws2s(args[0]->toString());
	std::regex e(ws2s(args[1]->toString()));
	std::smatch m;
	VEC_Memory ret;
	while (std::regex_search(s,m,e)){
		VEC_Memory curr;
		for (auto x : m) {
			curr.push_back(new_memory(s2ws(x.str())));
		}
		ret.push_back(new_memory(curr));
		s = m.suffix().str();
	}
	return ret;
}

std::vector<SP_Memory> __regex_replace(std::vector<SP_Memory> args) {
	std::string s = ws2s(args[0]->toString());
	std::string r = ws2s(args[1]->toString());
	std::regex e(ws2s(args[2]->toString()));
	std::string result;
	std::regex_replace(std::back_inserter(result), s.begin(), s.end(), e, r);
	return {new_memory(s2ws(result))};
}

std::vector<SP_Memory> __filesystem_listdir(std::vector<SP_Memory> args) {
	VEC_Memory list;
	String path = Interpreter::path.substr(1) + args[0]->toString();
	for (auto &p : std::filesystem::directory_iterator(path)){
		String file = s2ws(p.path().string().substr(path.length()));
		while (file[0] == '\\' || file[0] == '/')
			file = file.substr(1);
		list.push_back(new_memory(file));
	}
	return list;
}
std::vector<SP_Memory> __filesystem_path(std::vector<SP_Memory> args) {
	return {new_memory(s2ws(std::filesystem::current_path().string()) + Interpreter::path)};
}

std::vector<SP_Memory> __filesystem_mkdir(std::vector<SP_Memory> args) {
	std::filesystem::create_directory(Interpreter::path.substr(1) + args[0]->toString());
	return {new_memory()};
}

std::vector<SP_Memory> __filesystem_exists(std::vector<SP_Memory> args) {
	return { new_memory(NUM, std::filesystem::exists(Interpreter::path.substr(1) + args[0]->toString())) };
}

std::vector<SP_Memory> __filesystem_copy(std::vector<SP_Memory> args) {
	String origin = Interpreter::path.substr(1) + args[0]->toString();
	String path = Interpreter::path.substr(1) + args[1]->toString();
	std::filesystem::copy(origin, path);
	return {new_memory()};
}

std::vector<SP_Memory> __filesystem_rename(std::vector<SP_Memory> args) {
	String origin = Interpreter::path.substr(1) + args[0]->toString();
	String path = Interpreter::path.substr(1) + args[1]->toString();
	std::filesystem::rename(origin, path);
	return {new_memory()};
}

std::vector<SP_Memory> __filesystem_size(std::vector<SP_Memory> args) {
	return { new_memory(NUM, std::filesystem::file_size(Interpreter::path.substr(1) + args[0]->toString())) };
}

std::vector<SP_Memory> __filesystem_remove(std::vector<SP_Memory> args) {
	std::filesystem::remove(Interpreter::path.substr(1) + args[0]->toString());
	return {new_memory()};
}

#ifdef _WIN32
std::vector<SP_Memory> __winsock_start(std::vector<SP_Memory> args) {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR) {
			return {new_memory(L"WSAStartup failed: " + std::to_wstring(iResult)) };
		}
	return {new_memory()};
}
std::vector<SP_Memory> __winsock_create_socket(std::vector<SP_Memory> args) {
	SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        WSACleanup();
        return {new_memory(L"Error at socket(): " + std::to_wstring(WSAGetLastError()) )};
    }
	return {new_memory(), new_memory((void*)ConnectSocket)};
}
std::vector<SP_Memory> __winsock_get_addresses(std::vector<SP_Memory> args) {	
	VEC_Memory ret;
	struct hostent *remoteHost;
	remoteHost = gethostbyname(ws2s(args[0]->toString()).c_str());
	struct in_addr addr;
	int i = 0;
	while (remoteHost->h_addr_list[i] != 0) {
		addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
		ret.push_back(new_memory(new_string(s2ws(inet_ntoa(addr)))));
	}
	return ret;
}
std::vector<SP_Memory> __winsock_connect(std::vector<SP_Memory> args) {
    struct sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(ws2s(args[1]->toString()).c_str());
    clientService.sin_port = htons(args[2]->getValue());
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        WSACleanup();
        return {new_memory(L"Unable to connect to server: " + std::to_wstring(WSAGetLastError()) )};
    }
	return {new_memory()};
}
std::vector<SP_Memory> __winsock_send(std::vector<SP_Memory> args) {
	const char * sendbuf = ws2s(args[1]->toString()).c_str();
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        WSACleanup();
        closesocket(ConnectSocket);
        return {new_memory(L"Send failed: " + std::to_wstring(WSAGetLastError()) )};
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
			callback->execute({new_memory(s2ws(std::string(recvbuf)))});
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
		return {new_memory(s2ws(std::string(recvbuf)))};
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
		return {new_memory(L"Shutdown Failure: " + std::to_wstring(WSAGetLastError()))};
    }
	return {new_memory()};
}
#endif

RuotaWrapper::RuotaWrapper(String current_dir){
	Interpreter::addEmbed(L"error", &__error);
	Interpreter::addEmbed(L"console.system", &__system);
	Interpreter::addEmbed(L"console.getenv", &__getenv);
	Interpreter::addEmbed(L"console.exit", &__exit);
	Interpreter::addEmbed(L"console.random", &__random);
	Interpreter::addEmbed(L"console.floor", &__floor);
	Interpreter::addEmbed(L"console.milli", &__milli);
	Interpreter::addEmbed(L"regex.search", &__regex_search);
	Interpreter::addEmbed(L"regex.replace", &__regex_replace);
	Interpreter::addEmbed(L"filesystem.listdir", &__filesystem_listdir);
	Interpreter::addEmbed(L"filesystem.mkdir", &__filesystem_mkdir);
	Interpreter::addEmbed(L"filesystem.exists", &__filesystem_exists);
	Interpreter::addEmbed(L"filesystem.copy", &__filesystem_copy);
	Interpreter::addEmbed(L"filesystem.size", &__filesystem_size);
	Interpreter::addEmbed(L"filesystem.path", &__filesystem_path);
	Interpreter::addEmbed(L"filesystem.rename", &__filesystem_rename);
	Interpreter::addEmbed(L"filesystem.remove", &__filesystem_remove);
	#ifdef _WIN32
		Interpreter::addEmbed(L"winsock.start", &__winsock_start);
		Interpreter::addEmbed(L"winsock.create_socket", &__winsock_create_socket);
		Interpreter::addEmbed(L"winsock.connect", &__winsock_connect);
		Interpreter::addEmbed(L"winsock.send", &__winsock_send);
		Interpreter::addEmbed(L"winsock.listen", &__winsock_listen);
		Interpreter::addEmbed(L"winsock.receive", &__winsock_receive);
		Interpreter::addEmbed(L"winsock.get_addresses", &__winsock_get_addresses);
		Interpreter::addEmbed(L"winsock.shutdown", &__winsock_shutdown);
	#endif
	#ifdef FILE_IO
		Interpreter::addEmbed(L"file.open", &__file_open);
		Interpreter::addEmbed(L"file.close", &__file_close);
		Interpreter::addEmbed(L"file.read_line", &__file_read_line);
		Interpreter::addEmbed(L"file.read", &__file_read);
		Interpreter::addEmbed(L"filew.open", &__filew_open);
		Interpreter::addEmbed(L"filew.close", &__filew_close);
		Interpreter::addEmbed(L"filew.write", &__filew_write);
	#endif
	this->current_dir = current_dir;
	while (this->current_dir.back() != '\\' && this->current_dir.back() != '/') {
		this->current_dir.pop_back();
		if (this->current_dir.empty())
			break;
	}
	this->interpreter = new Interpreter(this->current_dir);
	main_scope = new_scope(nullptr);
	this->interpreter->generate(s2ws(os_compiled), main_scope, L"\\");
	this->interpreter->execute(main_scope);
}


SP_Memory RuotaWrapper::runLine(String line) {
	this->interpreter->generate(line , main_scope, L"\\");
	return this->interpreter->execute(main_scope);
}