#define MODULEUUIDSTR_ECSBAS_APP                 "\xfd\xcf\x17\xa4\xf4\xf9\x40\x28\xb7\x1b\xed\x64\x1b\x2c\x3c\xb5"//need to send this str to server if client want to connect to it.
#define MODULEUUIDSTR_ECSBAS_GUI                 "\x7d\xfe\xf3\x20\x6b\x31\x42\x4f\x8b\x31\x58\x1e\x22\xd1\x21\x6f"//need to send this str to client if server want to connect to it.

#include <thread>
#include <mutex>
#include <condition_variable>
#include <climits>
#include <iostream>
#include <fstream>
#include <regex>


#include <experimental/filesystem> // C++-standard header file name  
//#include <filesystem> // Microsoft-specific implementation header file name  


//undone do checking (validation) for all requests about null-terminated string containing null inbetween. Note that even Browser allows TextEncoder.encode/Blob's ctor taking string containing '\u0000'

//INSTR is case-sensitive, change to LIKE when necessary
//optimize change LIKE to INSTR, which should be faster (no need to escape anything, works for blob) (but cannot include wildcard inside query)
//todo change all system() to something like posix_spawn, to avoid popup of shell

//todo write a Transactional Filesystem Library with sqlite and c++ std::filesystem. Filename is stored in sqlite. Physical filename is ephemeral.

#include <SDL.h>
#include <sqlite3.h>
using namespace std::experimental::filesystem::v1;  
using namespace std;
#define NEED_TO_AVERT_RACES
#include <cpprs.h>
#include <cpprs_sqlite_mod.h>
#include <tc.h>
inline sqlite3 *databaseA;
inline sqlite3_int64 availid;
inline mutex logm;
inline mutex fem;
inline condition_variable fecv;
inline mutex dbm;
inline condition_variable dbcv;
inline long long rl;
inline unsigned char *rb;
#define RESLEN_NO_RESULT 0
#define RESLEN_DB_ALREADY_SHUT_DOWN -1
#define RESLEN_ERR -2
inline unsigned char dbready;
#define DB_INIT_IN_PROGRESS 0
#define DB_INIT_READY 1
#define DB_INIT_ERR 2
inline path e_customizable_p;
inline path e_p;
#define LOG_I_CIFNOTNULLSQLITEFREEs(i,c) {lock_guard<mutex> lgcs{logm};STD_CLOG_TIME_FILE_FUNC_LINE<<i;if(c){clog<<' '<<c;sqlite3_free(c);}clog<<'\n'<<std::flush;}
#define LOG_I_Cs(i,c) {lock_guard<mutex> lgcs{logm};STD_CLOG_TIME_FILE_FUNC_LINE<<i<<' '<<c<<'\n'<<std::flush;}
#define LOG_Is(i) {lock_guard<mutex> lgcs{logm};STD_CLOG_TIME_FILE_FUNC_LINE<<i<<'\n'<<std::flush;}
#define LOG_ERR_NOEXCEPTs {try{LOG_ERRs}catch(...){}}
#define LOG_E_NOEXCEPTs(e){try{LOG_Es(e)}catch(...){}}
#define LOG_UNREACHABLEs LOG_ERRs
#define LOG_ERRs {lock_guard<mutex> lgcs{logm};STD_CLOG_TIME_FILE_FUNC_LINE_FLUSH;}
#define LOG_Es(e) {lock_guard<mutex> lgcs{logm};STD_CLOG_TIME_FILE_FUNC_LINE_EX_FLUSH(e);}
#define LOG_ERR_TRY_GET_SDL_ERRs {lock_guard<mutex> lgcs{logm};STD_CLOG_TIME_FILE_FUNC_LINE;sdlgeterrorwritelog();}


