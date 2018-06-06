#include "Ruota.h"

const char * os_compiled = {
	#include "compiled/System.ruo"
};

std::vector<SP_MEMORY> __error(std::vector<SP_MEMORY> args) {
	throw std::runtime_error(args[0]->toString());
}

std::vector<SP_MEMORY> __system(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, system(args[0]->toString().c_str())) };
}

std::vector<SP_MEMORY> __getenv(std::vector<SP_MEMORY> args) {
	auto env = getenv(args[0]->toString().c_str());
	if (env == NULL)
		return { NEW_MEMORY() };
	return { NEW_MEMORY(std::string(env)) };
}

std::vector<SP_MEMORY> __exit(std::vector<SP_MEMORY> args) {
	quick_exit(0);
	return { NEW_MEMORY() }; //superfluous?
}

std::vector<SP_MEMORY> __random(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, (long double)rand() / RAND_MAX) };
}

std::vector<SP_MEMORY> __floor(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, std::floor(args[0]->getValue())) };
}

std::vector<SP_MEMORY> __file_open(std::vector<SP_MEMORY> args) {
	std::string fname = Interpreter::path.substr(1) + args[0]->toString();
	while (fname[0] == '\\') fname = fname.substr(1);
	std::ifstream *file = new std::ifstream(fname);
	if (!file->is_open()){
		throw std::runtime_error("Error: cannot open file " + fname + "!");
	}
	return { NEW_MEMORY((void*)file) };
}

std::vector<SP_MEMORY> __filew_open(std::vector<SP_MEMORY> args) {
	std::string fname = Interpreter::path.substr(1) + args[0]->toString();
	while (fname[0] == '\\') fname = fname.substr(1);
	std::ofstream *file = new std::ofstream(fname, args[1]->getValue() == 'a' ? std::ofstream::app : std::ofstream::out);
	if (!file->is_open()){
		throw std::runtime_error("Error: cannot open file " + fname + "!");
	}
	return { NEW_MEMORY((void*)file) };
}

std::vector<SP_MEMORY> __file_close(std::vector<SP_MEMORY> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	file->close();
	delete file;
	return { NEW_MEMORY() };
}

std::vector<SP_MEMORY> __filew_close(std::vector<SP_MEMORY> args) {
	std::ofstream *file = (std::ofstream*)args[0]->getPointer();
	file->close();
	delete file;
	return { NEW_MEMORY() };
}

std::vector<SP_MEMORY> __filew_write(std::vector<SP_MEMORY> args) {
	std::ofstream *file = (std::ofstream*)args[0]->getPointer();
	*file << args[1]->toString();
	return { NEW_MEMORY() };
}

std::vector<SP_MEMORY> __file_read_line(std::vector<SP_MEMORY> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	std::string line;
	if (getline(*file, line))
		return { NEW_MEMORY(line) };
	else
		return { NEW_MEMORY() };
}

std::vector<SP_MEMORY> __file_read(std::vector<SP_MEMORY> args) {
	std::ifstream *file = (std::ifstream*)args[0]->getPointer();
	char * buffer = new char[args[1]->getValue()];
	if (file->read(buffer, args[1]->getValue()))
		return { NEW_MEMORY(std::string(buffer, args[1]->getValue())) };
	else
		return { NEW_MEMORY() };
}

std::vector<SP_MEMORY> __milli(std::vector<SP_MEMORY> args) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	return { NEW_MEMORY(NUM, timeMillis) };
}

std::vector<SP_MEMORY> __regex_search(std::vector<SP_MEMORY> args) {
	std::string s = args[0]->toString();
	std::regex e(args[1]->toString());
	std::smatch m;
	VEC_Memory ret;
	while (std::regex_search(s,m,e)){
		VEC_Memory curr;
		for (auto x : m) {
			curr.push_back(NEW_MEMORY(x.str()));
		}
		ret.push_back(NEW_MEMORY(curr));
		s = m.suffix().str();
	}
	return ret;
}

std::vector<SP_MEMORY> __regex_replace(std::vector<SP_MEMORY> args) {
	std::string s = args[0]->toString();
	std::string r = args[1]->toString();
	std::regex e(args[2]->toString());
	std::string result;
	std::regex_replace(std::back_inserter(result), s.begin(), s.end(), e, r);
	return {NEW_MEMORY(result)};
}

std::vector<SP_MEMORY> __filesystem_listdir(std::vector<SP_MEMORY> args) {
	VEC_Memory list;
	std::string path = Interpreter::path.substr(1) + args[0]->toString();
	for (auto &p : std::filesystem::directory_iterator(path)){
		std::string file = p.path().string().substr(path.length());
		while (file[0] == '\\' || file[0] == '/')
			file = file.substr(1);
		list.push_back(NEW_MEMORY(file));
	}
	return list;
}
std::vector<SP_MEMORY> __filesystem_path(std::vector<SP_MEMORY> args) {
	return {NEW_MEMORY(std::filesystem::current_path().string() + Interpreter::path)};
}

std::vector<SP_MEMORY> __filesystem_mkdir(std::vector<SP_MEMORY> args) {
	std::filesystem::create_directory(Interpreter::path.substr(1) + args[0]->toString());
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __filesystem_exists(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, std::filesystem::exists(Interpreter::path.substr(1) + args[0]->toString())) };
}

std::vector<SP_MEMORY> __filesystem_copy(std::vector<SP_MEMORY> args) {
	std::string origin = Interpreter::path.substr(1) + args[0]->toString();
	std::string path = Interpreter::path.substr(1) + args[1]->toString();
	std::filesystem::copy(origin, path);
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __filesystem_rename(std::vector<SP_MEMORY> args) {
	std::string origin = Interpreter::path.substr(1) + args[0]->toString();
	std::string path = Interpreter::path.substr(1) + args[1]->toString();
	std::filesystem::rename(origin, path);
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __filesystem_size(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, std::filesystem::file_size(Interpreter::path.substr(1) + args[0]->toString())) };
}

std::vector<SP_MEMORY> __filesystem_remove(std::vector<SP_MEMORY> args) {
	std::filesystem::remove(Interpreter::path.substr(1) + args[0]->toString());
	return {NEW_MEMORY()};
}

#ifdef _WIN32
std::vector<SP_MEMORY> __winsock_start(std::vector<SP_MEMORY> args) {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR) {
			return {NEW_MEMORY("WSAStartup failed: " + std::to_string(iResult)) };
		}
	return {NEW_MEMORY()};
}
std::vector<SP_MEMORY> __winsock_create_socket(std::vector<SP_MEMORY> args) {
	SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        WSACleanup();
        return {NEW_MEMORY("Error at socket(): " + std::to_string(WSAGetLastError()) )};
    }
	return {NEW_MEMORY(), NEW_MEMORY((void*)ConnectSocket)};
}
std::vector<SP_MEMORY> __winsock_get_addresses(std::vector<SP_MEMORY> args) {	
	VEC_Memory ret;
	struct hostent *remoteHost;
	remoteHost = gethostbyname(args[0]->toString().c_str());
	struct in_addr addr;
	int i = 0;
	while (remoteHost->h_addr_list[i] != 0)
	{
		addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
		ret.push_back(NEW_MEMORY(std::string(inet_ntoa(addr))));
	}
	return ret;
}
std::vector<SP_MEMORY> __winsock_connect(std::vector<SP_MEMORY> args) {
    struct sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(args[1]->toString().c_str());
    clientService.sin_port = htons(args[2]->getValue());
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        WSACleanup();
        return {NEW_MEMORY("Unable to connect to server: " + std::to_string(WSAGetLastError()) )};
    }
	return {NEW_MEMORY()};
}
std::vector<SP_MEMORY> __winsock_send(std::vector<SP_MEMORY> args) {
	const char * sendbuf = args[1]->toString().c_str();
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        WSACleanup();
        closesocket(ConnectSocket);
        return {NEW_MEMORY("Send failed: " + std::to_string(WSAGetLastError()) )};
    }
	return {NEW_MEMORY()};
}
std::vector<SP_MEMORY> __winsock_listen(std::vector<SP_MEMORY> args) {
	SP_LAMBDA callback = args[2]->getLambda();
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult;
	int recvbuflen = args[1]->getValue();
	do {	
		char * recvbuf = new char[args[1]->getValue()];
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
			callback->execute({NEW_MEMORY(std::string(recvbuf))});
        else if ( iResult == 0 )
            throw std::runtime_error("Connection closed");
        else
            throw std::runtime_error("Receive Failure: " + std::to_string(WSAGetLastError()));
    } while( iResult > 0 );
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __winsock_receive(std::vector<SP_MEMORY> args) {
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	char * recvbuf = new char[args[1]->getValue()];
	int recvbuflen = args[1]->getValue();
	int iResult;
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if ( iResult > 0 )
		return {NEW_MEMORY(std::string(recvbuf))};
	else if ( iResult == 0 )
		throw std::runtime_error("Connection closed");
	else
		throw std::runtime_error("Receive Failure: " + std::to_string(WSAGetLastError()));
}

std::vector<SP_MEMORY> __winsock_shutdown(std::vector<SP_MEMORY> args) {
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
		return {NEW_MEMORY("Shutdown Failure: " + std::to_string(WSAGetLastError()))};
    }
	return {NEW_MEMORY()};
}
#endif

RuotaWrapper::RuotaWrapper(std::string current_dir){
	Interpreter::addEmbed("error", &__error);
	Interpreter::addEmbed("console.system", &__system);
	Interpreter::addEmbed("console.getenv", &__getenv);
	Interpreter::addEmbed("console.exit", &__exit);
	Interpreter::addEmbed("console.random", &__random);
	Interpreter::addEmbed("console.floor", &__floor);
	Interpreter::addEmbed("console.milli", &__milli);
	Interpreter::addEmbed("regex.search", &__regex_search);
	Interpreter::addEmbed("regex.replace", &__regex_replace);
	Interpreter::addEmbed("filesystem.listdir", &__filesystem_listdir);
	Interpreter::addEmbed("filesystem.mkdir", &__filesystem_mkdir);
	Interpreter::addEmbed("filesystem.exists", &__filesystem_exists);
	Interpreter::addEmbed("filesystem.copy", &__filesystem_copy);
	Interpreter::addEmbed("filesystem.size", &__filesystem_size);
	Interpreter::addEmbed("filesystem.path", &__filesystem_path);
	Interpreter::addEmbed("filesystem.rename", &__filesystem_rename);
	Interpreter::addEmbed("filesystem.remove", &__filesystem_remove);
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
	main_scope = NEW_SCOPE(nullptr);
	this->interpreter->generate(os_compiled, main_scope, "\\");
	this->interpreter->execute(main_scope);
}


SP_MEMORY RuotaWrapper::runLine(std::string line) {
	this->interpreter->generate(line , main_scope, "\\");
	
	#ifdef DEBUG
	//std::cout << " >> >> PRE:\n";
	//debugPrint();
	#endif

	auto temp = this->interpreter->execute(main_scope);

	#ifdef DEBUG
	std::cout << " >> >> POST:\n";
	debugPrint();
	#endif

	return temp;
}

#ifdef DEBUG
void RuotaWrapper::debugPrint() {
	std::cout << "------------------------------------\nMEM:\t+" << Memory::reference_add << "\t-" << Memory::reference_del << "\t" << Memory::reference_add - Memory::reference_del << std::endl;
	std::cout << "LAM:\t+" << Lambda::reference_add << "\t-" << Lambda::reference_del << "\t" << Lambda::reference_add - Lambda::reference_del << std::endl;
	std::cout << "NOD:\t+" << Node::reference_add << "\t-" << Node::reference_del << "\t" << Node::reference_add - Node::reference_del << std::endl;
	std::cout << "SCO:\t+" << Scope::reference_add << "\t-" << Scope::reference_del << "\t" << Scope::reference_add - Scope::reference_del << std::endl;
}
#endif

RuotaWrapper::~RuotaWrapper() {
	delete this->interpreter;
	this->main_scope = nullptr;
}