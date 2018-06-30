#include "Network.h"

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
