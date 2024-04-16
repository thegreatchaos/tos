/**
 * \brief Table Of Servers
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <unordered_map>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define BACKLOG		1024
#define LISTEN_PORT	7777
#define MAX_EPOLL_NUM	4096
typedef struct DevInfo{
    char	ip[1024];
    char	ts[1024];
}DevInfo;
typedef std::unordered_map<std::string, DevInfo> StrMap;
typedef enum ReqType{
    RT_REG = 0,
    RT_PUL,
    RT_MAX
}ReqType;
int getFd(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(0 > fd){
	fprintf(stdout, "socket error, %s:%d\n", __FILE__, __LINE__);
	return -__LINE__;
    }
    int keepAlive = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(keepAlive));
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &keepAlive, sizeof(keepAlive));

    struct sockaddr_in addr = {0};
    addr.sin_family	=   AF_INET;
    addr.sin_port	=   htons(LISTEN_PORT);

    if(0 > bind(fd, (struct sockaddr*)&addr, sizeof(addr))){
	fprintf(stdout, "bind failure, %s:%d\n", __FILE__, __LINE__);
	return -__LINE__;
    }
    if(0 > listen(fd, BACKLOG)){
	fprintf(stdout, "listen failure, %s:%d\n", __FILE__, __LINE__);
	return -__LINE__;
    }
    return fd;
}
void split(const char* src, size_t* cur, char* to){
    size_t y = 0;
    while(*cur < strlen(src)){
	if('/' != src[*cur]){
	    to[y] = src[*cur];
	}else break;
	++*cur;
	++y;
    }
    to[y] = 0;
}
int main(int argc, char** argv){
    int listenFd = getFd();
    if(listenFd < 0){
	fprintf(stdout, "cannot create socket\n");
	return -__LINE__;
    }
    int epollFd	    =	epoll_create(MAX_EPOLL_NUM);
    if(0 > epollFd){
	perror("epoll_create");
	return -__LINE__;
    }
    struct epoll_event listenEvt, *rwEvt/*read/write event*/, lastPendingEvt;

    listenEvt.events	=   EPOLLIN;
    listenEvt.data.fd	=   listenFd;
    if(0 > epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &listenEvt)){
	perror("epoll_ctl");
	return -__LINE__;
    }

    rwEvt = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAX_EPOLL_NUM);

    struct sockaddr_in	clientAddr;
    socklen_t		clientLen = sizeof(clientAddr);
    StrMap  host2Ip;
    char* hBuf = (char*)malloc(1024);
    char* bBuf = (char*)malloc(8 * 1024 * 1024);
    while(true){
	int activeFdsCount = epoll_wait(epollFd, rwEvt, MAX_EPOLL_NUM, -1);
	//fprintf(stdout, "%d active count currently\n", activeFdsCount);
	for(int i = 0; i < activeFdsCount; i++){
	    if(rwEvt[i].data.fd == listenFd){///got an incoming connection
		int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientLen);
		if(0 > clientFd){
		    perror("accept");
		    continue;
		}
		char ip[20];
		//fprintf(stdout, "incoming connection from %s:%d\n", inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip)), ntohs(clientAddr.sin_port));
		lastPendingEvt.events	=	EPOLLIN | EPOLLET; ///
		lastPendingEvt.data.fd	=	clientFd;
		epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &lastPendingEvt);//after establishing a new connection from remote client, add event for READ only, no WRITE  
	    }else if(rwEvt[i].events & EPOLLIN){///ready for reading
		int clientFd = rwEvt[i].data.fd;
		char buf[1024];
		int readSz = read(clientFd, buf, 1024);
		if(0 > readSz){
		    perror("read");
		    continue;
		}else if(0 == readSz){///all data has been read
		    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &lastPendingEvt);///remove the last pending event
		    close(clientFd);
		}else{
		    char route[1024]; 
		    memset(route, 0, 1024);
		    size_t type = RT_REG;
		    size_t i = 0, j = 0;
		    while(' ' != buf[i++]);
		    size_t start = i++;
		    while(' ' != buf[i++]);
		    size_t stop = i-1;
		    memcpy(route, buf + start, stop - start + 1);
		    type = RT_PUL;
		    if(strlen(route) > 2)
			type = RT_REG;
		    switch(type){
		    case RT_REG:
			{
			    char host[1024];
			    DevInfo di;
			    size_t x = 1, y = 0;
			    split(route, &x, host);
			    ++x;
			    split(route, &x, di.ip);
			    ++x;
			    split(route, &x, di.ts);
			    sprintf(bBuf, "Registered:\nhost: %s\nip: %s\nLastUpdate: %s\n", host, di.ip, di.ts);
			    host2Ip[host] = di;
			}
			break;
		    case RT_PUL:
			sprintf(bBuf, "<table border=1px><tr><th>Hostname</th><th>Ip</th><th>LastUpdate</th></tr>");
			for(auto& i : host2Ip){
			    sprintf(bBuf + strlen(bBuf), "<tr><td>%s</td><td>%s</td><td>%s</td></tr>", i.first.c_str(), i.second.ip, i.second.ts);
			}
			sprintf(bBuf + strlen(bBuf), "</table>");
			break;
		    }
		    sprintf(hBuf, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n\r\n", strlen(bBuf));
		    write(clientFd, hBuf, strlen(hBuf));
		    write(clientFd, bBuf, strlen(bBuf));
		    close(clientFd);
		}
	    }else if(rwEvt[i].events & EPOLLOUT){/// ready for writing
	    }
	}
    }
    free(hBuf);
    free(bBuf);
    return 0;
}
