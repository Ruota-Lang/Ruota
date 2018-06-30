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
	_c_exit();
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
	char * buffer = new char[(int)args[1]->getValue()];
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
	for (auto &p : boost::filesystem::directory_iterator(path)){
		std::string file = p.path().string().substr(path.length());
		while (file[0] == '\\' || file[0] == '/')
			file = file.substr(1);
		list.push_back(NEW_MEMORY(file));
	}
	return list;
}
std::vector<SP_MEMORY> __filesystem_path(std::vector<SP_MEMORY> args) {
	return {NEW_MEMORY(boost::filesystem::current_path().string() + Interpreter::path)};
}

std::vector<SP_MEMORY> __filesystem_mkdir(std::vector<SP_MEMORY> args) {
	boost::filesystem::create_directory(Interpreter::path.substr(1) + args[0]->toString());
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __filesystem_exists(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, boost::filesystem::exists(Interpreter::path.substr(1) + args[0]->toString())) };
}

std::vector<SP_MEMORY> __filesystem_copy(std::vector<SP_MEMORY> args) {
	std::string origin = Interpreter::path.substr(1) + args[0]->toString();
	std::string path = Interpreter::path.substr(1) + args[1]->toString();
	boost::filesystem::copy(origin, path);
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __filesystem_rename(std::vector<SP_MEMORY> args) {
	std::string origin = Interpreter::path.substr(1) + args[0]->toString();
	std::string path = Interpreter::path.substr(1) + args[1]->toString();
	boost::filesystem::rename(origin, path);
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __filesystem_size(std::vector<SP_MEMORY> args) {
	return { NEW_MEMORY(NUM, boost::filesystem::file_size(Interpreter::path.substr(1) + args[0]->toString())) };
}

std::vector<SP_MEMORY> __filesystem_remove(std::vector<SP_MEMORY> args) {
	boost::filesystem::remove(Interpreter::path.substr(1) + args[0]->toString());
	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __network_start(std::vector<SP_MEMORY> args) {
	/*	WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR) {
			return {NEW_MEMORY("WSAStartup failed: " + std::to_string(iResult)) };
		}
	return {NEW_MEMORY()};*/

	boost::asio::io_service * ios = new boost::asio::io_service();
	return { NEW_MEMORY((void*)ios)};
}
std::vector<SP_MEMORY> __network_create_socket(std::vector<SP_MEMORY> args) {
	/*SOCKET ConnectSocket = INVALID_SOCKET;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        WSACleanup();
        return {NEW_MEMORY("Error at socket(): " + std::to_string(WSAGetLastError()) )};
    }
	return {NEW_MEMORY(), NEW_MEMORY((void*)ConnectSocket)};*/
	boost::asio::io_service * ios = (boost::asio::io_service*)args[0]->getPointer();
	boost::asio::ip::tcp::socket * ConnectSocket = new boost::asio::ip::tcp::socket(*ios);
	return {NEW_MEMORY(), NEW_MEMORY((void*)ConnectSocket)};
}
std::vector< std::string > getHostByName(std::string hostname) {
  std::vector<std::string> addresses;
  boost::asio::io_service io_service;
  try {
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(hostname, "");
    boost::asio::ip::tcp::resolver::iterator destination = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    boost::asio::ip::tcp::endpoint endpoint;
    while (destination != end) {
      endpoint = *destination++;
      addresses.push_back(endpoint.address().to_string());
    }
  } catch(boost::system::system_error& error) {
	  throw std::runtime_error("Hostname not found!");
  }

  return(addresses);

}
std::vector<SP_MEMORY> __network_get_addresses(std::vector<SP_MEMORY> args) {	
	VEC_Memory ret;
	auto hosts = getHostByName(args[0]->toString());
	for (auto h : hosts)
		ret.push_back(NEW_MEMORY(h));
	return ret;
}
std::vector<SP_MEMORY> __network_connect(std::vector<SP_MEMORY> args) {
    /*struct sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(args[1]->toString().c_str());
    clientService.sin_port = htons(args[2]->getValue());
	SOCKET ConnectSocket = (SOCKET)args[0]->getPointer();
	int iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        WSACleanup();
        return {NEW_MEMORY("Unable to connect to server: " + std::to_string(WSAGetLastError()) )};
    }*/

	boost::asio::ip::tcp::socket * ConnectSocket = (boost::asio::ip::tcp::socket*)args[0]->getPointer();
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(args[1]->toString()), args[2]->getValue());
	ConnectSocket->connect(endpoint);
	return {NEW_MEMORY()};
}
std::vector<SP_MEMORY> __network_send(std::vector<SP_MEMORY> args) {	
	boost::asio::ip::tcp::socket * ConnectSocket = (boost::asio::ip::tcp::socket*)args[0]->getPointer();
	std::string message = args[1]->toString();

	//boost::array<char, 128> buf;	
	//std::copy(message.begin(),message.end(),buf.begin());

	boost::system::error_code error;
	ConnectSocket->write_some(boost::asio::buffer(message, message.size()), error);

	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __network_listen(std::vector<SP_MEMORY> args) {
	boost::asio::ip::tcp::socket * ConnectSocket = (boost::asio::ip::tcp::socket *)args[0]->getPointer();
	SP_LAMBDA callback = args[2]->getLambda();
	int iResult;
	int recvbuflen = args[1]->getValue();
	
	do {
		char * recvbuf = new char[(int)args[1]->getValue()];
		try {
			iResult = ConnectSocket->receive(boost::asio::buffer(recvbuf, recvbuflen));
			if ( iResult > 0 )
				callback->execute({NEW_MEMORY(std::string(recvbuf))});
		} catch (...){
			throw std::runtime_error("an error has occurred along the socket.");
		}
	} while(iResult > 0);

	return {NEW_MEMORY()};
}

std::vector<SP_MEMORY> __network_receive(std::vector<SP_MEMORY> args) {
	boost::asio::ip::tcp::socket * ConnectSocket = (boost::asio::ip::tcp::socket*)args[0]->getPointer();
	char * recvbuf = new char[(int)args[1]->getValue()];
	int recvbuflen = args[1]->getValue();
    boost::system::error_code error;
	size_t len = ConnectSocket->read_some(boost::asio::buffer(recvbuf, recvbuflen), error);
	return {NEW_MEMORY(std::string(recvbuf, len))};
}

std::vector<SP_MEMORY> __network_shutdown(std::vector<SP_MEMORY> args) {
	boost::asio::ip::tcp::socket * ConnectSocket = (boost::asio::ip::tcp::socket *)args[0]->getPointer();
	ConnectSocket->close();
	return {NEW_MEMORY()};
}

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
		Interpreter::addEmbed("network.start", &__network_start);
		Interpreter::addEmbed("network.create_socket", &__network_create_socket);
		Interpreter::addEmbed("network.connect", &__network_connect);
		Interpreter::addEmbed("network.send", &__network_send);
		Interpreter::addEmbed("network.listen", &__network_listen);
		Interpreter::addEmbed("network.receive", &__network_receive);
		Interpreter::addEmbed("network.get_addresses", &__network_get_addresses);
		Interpreter::addEmbed("network.shutdown", &__network_shutdown);
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