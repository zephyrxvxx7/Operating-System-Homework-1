// parent.cpp: 定義主控台應用程式的進入點。
//

// OrdinaryPipe.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <string>
#include <iostream>
using namespace std;


#define BUFSIZE 2048 
#define filePath "aFile.txt"

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

//HANDLE g_hInputFile = NULL;

void CreateChildProcess(void);
void WriteToPipe(string);
void ReadFromPipe(void);
string readFile(string);

int main(int argc, TCHAR *argv[])
{
	SECURITY_ATTRIBUTES saAttr;
	string sendSomething = "";
	cout << endl << "->Start of parent execution." << endl;

	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 
	CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);

	// Ensure the read handle to the pipe for STDOUT is not inherited. 
	SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

	// Create a pipe for the child process's STDIN. 
	CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0);

	// Ensure the write handle to the pipe for STDIN is not inherited. 
	SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);
	cout << endl << "->Success to set initial status." << endl;

	// Create the child process. 
	CreateChildProcess();
	cout << endl << "->Success to create the child process." << endl;

	sendSomething = readFile(filePath);
	cout << endl << "->Success to read the file." << endl;

	// Write to the pipe that is the standard input for a child process. 
	// Data is written to the pipe's buffers, so it is not necessary to wait
	// until the child process is running before writing data.
	cout << endl << "->Start of sending message: " << sendSomething << endl;
	WriteToPipe(sendSomething);
	cout << endl << "->Success to WriteToPipe." << endl;


	// Read from pipe that is the standard output for child process. 
	ReadFromPipe();

	cout << endl << "->End of parent execution." << endl;

	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 

	return 0;
}

void CreateChildProcess()
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	TCHAR szCmdline[] = TEXT("child");
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

					   // If an error occurs, exit the application. 
	if (!bSuccess)
		cout << "CreateProcess failed!!" << endl;
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}

void WriteToPipe(string input_str)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{
	DWORD dwWritten;
	BOOL bSuccess = FALSE;

	const char *str = input_str.c_str();

	bSuccess = WriteFile(g_hChildStd_IN_Wr, str, (DWORD)(strlen(str) + 1), &dwWritten, NULL); //將字串寫入g_hChildStd_IN_Wr

																							  // Close the pipe handle so the child process stops reading. 
	if (!CloseHandle(g_hChildStd_IN_Wr))
		cout << "StdInWr CloseHandle failed!!" << endl;
}

void ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	int flag = 1;
	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) {
			cout << endl << " no data to read." << endl;
			break;
		}

		if (flag != 1) //第一次會收到來自自己的STDOUT，因此用flag略過
		{
			//清除從child傳過來後的buffer當中不必要的訊息
			for (int i = 0; i < BUFSIZE; i++) {
				if (chBuf[i] < 32 || chBuf[i]>126) {
					chBuf[i] = '\0';
					break;
				}
			}
			//印出收到的訊息
			cout << endl << "->Parent read from pipe: " << chBuf << endl;
			break;
		}
		flag++;
	}
}

string readFile(string file) {
	char readSomething[BUFSIZE] = "";

	fstream fin;
	fin.open(file, ios::in);
	if (fin)
		fin.read(readSomething, sizeof(readSomething));
	else
		cout << "couldn't find the file." << endl;
	fin.close();

	return readSomething;
}