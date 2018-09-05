/**********************************************************************
* Author:	jaron.ho
* Date:		2018-05-10
* Brief:	process
**********************************************************************/
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

class Process {
public:
    /*
     * Brief:	enable process privilege
     * Param:	process - process handle, if NULL represent current process
     *           enabled - whether enable process privilege
     * Return:	0.ok
     *          1.open process token fail
     *          2.adjust token privileges fail
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

    /*
     * Brief:	kill process
     * Param:	processId - process id
     * Return:	0.ok
     *          1.can not open process
     *          2.kill fail
     */
    static int kill(unsigned long processId);

    /*
     * Brief:	kill application
     * Param:	appName - application name, e.g. "C:/Program Files/Notepad++/notepad++.exe" or "notepad++.exe"
     * Return:	void
     */
    static void killApp(const char* appName);

    /*
     * Brief:	create new process to run application
     * Param:	appName - application name, must be absolute path, e.g. "C:/Program Files/Notepad++/notepad++.exe"
     *          workingDir - application working directory, must be absolute path, e.g. "C:/Program Files/Notepad++/"
     *          newConsole - is application run use it's own console
     * Return:	0.ok
     *          1.appName is NULL or empty
     *          2.appName is not absolute path
     *          3.workingDir is not absolute path
     *          4.create process fail
     */
    static int runApp(const char* appName, const char* workingDir = NULL, bool newConsole = false);

    /*
     * Brief:	check whether application file is exist
     * Param:	appName - application name, must be absolute path, e.g. "C:/Program Files/Notepad++/notepad++.exe"
     * Return:	0.exist
     *          1.appName is NULL or empty
     *          2.application file is not exist
     */
    static int isAppFileExist(const char* appName);

    /*
     * Brief:	shell execute
     * Param:	filename - file name, e.g. "calc.exe" or "C:/Program Files/Notepad++/notepad++.exe"
     *          workingDir - application working directory, must be absolute path, e.g. "C:/Program Files/Notepad++/"
     *          showCmd - show cmd, [1, 11], refer to SW_NORMAL
     * Return:	0.ok
     *          1.filename is NULL or empty
     *          2.execute failed
     */
    static int shellExecute(const char* filename, const char* workingDir = NULL, int showCmd = 1);

public:
    /*
     * Brief:	current process exe path
     * Param:	void
     * Return:	std::string, e.g. "C:/Program Files/"
     */
    const std::string& exePath(void);

public:
    unsigned long id;                   /* process id */
    std::string exeFile;                /* exe file, e.g. "test.exe" */

private:
    std::string mExePath;               /* exe path, e.g. "C:/Program Files/" */
};

#endif	// _PROCESS_H_
