#include "FileIO.h"

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