// SERVER-WEB.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SERVER-WEB.h"
#include <time.h>
#ifdef _DEBUG
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define new DEBUG_NEW
#endif



CWinApp theApp;

using namespace std;
DWORD WINAPI ClientThread(LPVOID);
string html(char* request);
string getHtmlContent(char * path);
bool login(string, string);
void openFile();
void saveLog(string);
void createUser(string, string);

const string header = "HTTP/1.1 200 OK\nContent-Type: text/html;\nConnection: close\n";
char username[64][64];
char password[64][64];
int account = 0;

int main()
{
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			wprintf(L"Fatal Error: MFC initialization failed\n"); /// 
			nRetCode = 1;
		}
		else
		{
			//To do code here ... 
			openFile(); // openfile for reading username and password 
			WSADATA wsa;
			if (AfxSocketInit(&wsa))
				cout << "Initialized WSA\n";
			else {
				cout << "Failed to initilize";
				return -1;
			}
			// request => response 
			CSocket listener;
			listener.Create(80); // port 
			listener.Listen(5); // backlog

			int ret;
			char buf[1024]; // 50

			while (1 == 1) {
				CSocket client;
				SOCKADDR_IN addr;
				int len = sizeof(addr);
				listener.Accept(client, (SOCKADDR*)&addr, &len);

				ret = client.Receive(buf, sizeof(buf), 0);
				if (ret > 0) buf[ret] = 0; else continue;
				//save log
				string log(buf);
				int i = 0;
				while (true) {
					if (log.at(i) == '\n') {
						log = log.substr(0, i);
						break;
					};
					i++;
				}
				char* ip = inet_ntoa(addr.sin_addr);
				int port = ntohs(addr.sin_port);

				char save[256];
				sprintf(save, "\tIP ADDRESS: %s\tPORT\t%i REQUEST: %s", ip, port, log.c_str());
				cout << "SAVE: "<< save;
				saveLog(save);

				string response = html(buf); //
				char buff[1024];
				strcpy(buff, response.c_str());// string => char* 
				client.Send(buff, strlen(buff), 0);


			}
			system("pause");
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		wprintf(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
	}

	return nRetCode;
}

string html(char* request)
{
	string response = "";
	cout << request;
	if (strncmp(request, "GET / HTTP/1.1", 14) == 0) { // request home page 
		response += header;
		response += "\n";
		response += getHtmlContent("E:\\home.html");


	}
	else if (strncmp(request, "POST /login HTTP/1.1", 20) == 0) {

		response += header;
		string r(request);
		int pos = r.find_last_of("\n");
		string param = r.substr(pos + 1);
		int posN = param.find_first_of("&");
		string usr = param.substr(9, posN - 9);
		string psw = param.substr(posN + 10);

		if (login(usr, psw)) {
			response += "\n\n";
			response += getHtmlContent("E:\\login.html");
		}
		else {
			response += "\n";
			response += getHtmlContent("E:\\fail.html");
		}
	}
	else if (strncmp(request, "GET /register HTTP/1.1", 20) == 0) {
		response += header;
		response += "\n";
		response += getHtmlContent("E:\\register.html");
	}
	else if (strncmp(request, "POST /new_user HTTP/1.1", 23) == 0) {

		string r(request);
		int pos = r.find_last_of("\n");
		string param = r.substr(pos + 1);
		int posN = param.find_first_of("&");
		string usr = param.substr(9, posN - 9);
		string psw = param.substr(posN + 10);
		cout << usr.c_str() << "and" << psw.c_str();
		createUser(usr, psw);

		response += header;
		response += "\n";
		response += getHtmlContent("E:\\home.html");
	}
	else if (strncmp(request, "GET /logout HTTP/1.1", 20) == 0) {
		response += header;
		response += "\n";
		response += getHtmlContent("E:\\home.html");
	}
	else if (strncmp(request, "GET /allusers HTTP/1.1", 22) == 0) {
		response += header;
		response += "\n";
		response += "<html><body>";
		for (int i = 0; i < account; i++)
		{
			response.append("<t>");
			response.append(username[i]);
			response.append("</t><br>");
		}
		response.append("<a href=\"/\">Home Page</a>"); // <a href="/">Home Page</a> 
		response.append("</body></html>\r\n\r\n\r\n");
	}
	else {
		response += header;
		response += "\n";
		response += "<html><b>Page not found</b></html>\n\r\n\r\n";
	};
	return response;
}

string getHtmlContent(char * path) {
	FILE* f;
	char buf[1024];
	string content;
	f = fopen(path, "r");
	int ret;
	while (1 == 1) {
		ret = fread(buf, 1, sizeof(buf), f);
		if (ret <= 0) break;
		if (ret < 1023) buf[ret] = 0;
		content.append(buf);
	}
	fclose(f);
	content.append("\r\n\r\n\r\n");
	return content;

}

bool login(string user, string pass) {
	for (int i = 0; i < account; i++)
	{
		if (strcmp(username[i], user.c_str()) == 0)
			if (strcmp(password[i], pass.c_str()) == 0) {
				return true;
			}
	}
	return false;
}

void openFile() {
	FILE* f;
	f = fopen("E:\\users.txt", "r");
	account = 0;
	int ret = 0;
	while (true) {
		ret = fscanf(f, "%s%s", username[account], password[account]);
		if (ret < 0) break;
		account++;
	}
	cout << "List username and password:" << endl;
	for (int i = 0; i < account; i++)
	{
		cout << username[i] << ":" << password[i] << ":" << endl;
	}
	fclose(f);
}
void createUser(string u, string p) {
	FILE* f;
	f = fopen("E:\\users.txt", "a+");
	fwrite(u.c_str(), 1, strlen(u.c_str()), f);
	fwrite(" ", 1, 1, f);
	fwrite(p.c_str(), 1, strlen(p.c_str()), f);
	fwrite("\n", 1, 1, f);
	fclose(f);
	openFile();
}
void saveLog(string log) { // 12h52m55s IP 127.0.0.1 port 6532 GET / HTTP/1.1
	FILE* f;
	f = fopen("E:\\log.txt", "a+");
	cout << "LOG: ";
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char * time_now = asctime(timeinfo);
	fwrite(time_now, 1, strlen(time_now) - 1, f);
	fwrite(log.c_str(), 1, strlen(log.c_str())-1, f);
	fwrite("\n", 1, 1, f);

	fclose(f);
}