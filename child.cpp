// child.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <windows.h>
#include <sstream>
#include <string>
#include <iostream>
using namespace std;

#define BUFSIZE 2048 

string calculate(string str) {
	string op;
	stringstream ss;
	int a, b, ans;
	ss << str;
	ss >> a >> op >> b;
	

	if (op == "+")
		ans = a + b;
	else if (op == "-")
		ans = a - b;
	else if (op == "*")
		ans = a * b;
	else if (op == "/")
		ans = a / b;
	else
		ans = 0;

	ss.clear();
	ss.str("");

	ss << ans;
	return ss.str();
}

int main(void)
{
	CHAR chBuf[BUFSIZE];
	DWORD dwRead, dwWritten;
	HANDLE hStdin, hStdout;
	BOOL bSuccess;
	

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (
		(hStdout == INVALID_HANDLE_VALUE) ||
		(hStdin == INVALID_HANDLE_VALUE)
		)
		ExitProcess(1);

	// This simple algorithm uses the existence of the pipes to control execution.
	// It relies on the pipe buffers to ensure that no data is lost.
	// Larger applications would use more advanced process control.

	while(true)
	{
		// Read from standard input and stop on error or no data.
		bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0)
			break;

		string tmpChBuf = chBuf; 
		tmpChBuf = calculate(tmpChBuf);
		const char *str = tmpChBuf.c_str();

		cout << "now cuBuf is:" << str; //利用STDOUT將訊息送回pipe，讓parent接收
		bSuccess = WriteFile(hStdout, str, dwRead, &dwWritten, NULL);
		if (!bSuccess)
			break;
	}

	return 0;
}