#ifndef FILE_IO_H
#define FILE_IO_H

#include "../Ruota.h"

std::vector<SP_MEMORY> __file_open(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __filew_open(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __file_close(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __filew_close(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __filew_write(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __file_read_line(std::vector<SP_MEMORY> args);
std::vector<SP_MEMORY> __file_read(std::vector<SP_MEMORY> args);

#endif