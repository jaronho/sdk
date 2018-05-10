/**********************************************************************
* Author:	jaron.ho
* Date:		2018-05-10
* Brief:	logfile
**********************************************************************/
#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <stdio.h>
#include <vector>

#pragma warning(disable: 4996)

class Daemon {
public:
    class ProcessInfo {
    public:
        unsigned int processId;             /* process id */
        std::string exeFile;                /* exe file, e.g. "test.exe" */
        std::string exePath;                /* exe path, e.g. "D:/test/" */
    };
    
public:
    /*
     * Brief:	get process infos current time
     * Param:	matchExeFile - whether only match the exe file will be returned, if NULL return all, e.g. "test.exe"
     *          matchExePath - whether only match the exe path will be returned, if NULL return all, must end of '/', e.g. "D:/test/"
     *          queryExePath - whether query exe path    
     * Return:	std::vector<ProcessInfo>
     */
    static std::vector<ProcessInfo> getProcessInfos(const char* matchExeFile = NULL, const char* matchExePath = NULL, bool queryExePath = true);
};

#ifdef __cplusplus
}
#endif

#endif	// _DAEMON_H_
