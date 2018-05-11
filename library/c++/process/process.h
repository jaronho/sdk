/**********************************************************************
* Author:	jaron.ho
* Date:		2018-05-10
* Brief:	process
**********************************************************************/
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdio.h>
#include <vector>

class Process {  
public:
    /*
     * Brief:	enable process privilege
     * Param:	process - process handle, if NULL represent current process
     *           enabled - whether enable process privilege
     * Return:	0.ok
     *           1.open process token fail
     *           2.adjust token privileges fail
     */
    static int enablePrivilege(void* process = NULL, bool enabled = true);

    /*
     * Brief:	get process list
     * Param:	matchExeFile - whether only match the exe file will be returned, if NULL return all, e.g. "test.exe"
     * Return:	std::vector<ProcessInfo>
     */
    static std::vector<Process> getList(const char* matchExeFile = NULL);

    /*
     * Brief:	get process exe path
     * Param:	processId - process id
     * Return:	std::string, e.g. "C:/Program Files/"
     */
    static std::string getExePath(unsigned long processId);

public:
    /*
     * Brief:	current process exe path
     * Param:	void
     * Return:	std::string, e.g. "C:/Program Files/"
     */
    std::string exePath(void);

public:
    unsigned long id;                   /* process id */
    std::string exeFile;                /* exe file, e.g. "test.exe" */
};

#endif	// _PROCESS_H_
