/*
   3APA3A simpliest proxy server
   (c) 2002-2008 by ZARAZA <3APA3A@security.nnov.ru>

   please read License Agreement
*/

#include "proxy.h"

#define BUFSIZE (param->srv->bufsize?param->srv->bufsize:((param->service == S_UDPPM)?UDPBUFSIZE:TCPBUFSIZE))


int socksend(SOCKET sock, unsigned char * buf, int bufsize, int to){
 int sent = 0;
 int res;
 struct pollfd fds;

 fds.fd = sock;
 fds.events = POLLOUT;
 do {
	if(conf.timetoexit) return 0;
	res = so._poll(&fds, 1, to*1000);
	if(res < 0 && (errno == EAGAIN || errno == EINTR)) continue;
	if(res < 1) break;
	res = so._send(sock, (char *)buf + sent, bufsize - sent, 0);
	if(res < 0) {
		if(errno == EAGAIN || errno == EINTR) continue;
		break;
	}
	sent += res;
 } while (sent < bufsize);
 return sent;
}


int socksendto(SOCKET sock, struct sockaddr * sin, unsigned char * buf, int bufsize, int to){
 int sent = 0;
 int res;
 struct pollfd fds;

 fds.fd = sock;
 do {
	if(conf.timetoexit) return 0;
	fds.events = POLLOUT;
 	res = so._poll(&fds, 1, to);
	if(res < 0 && (errno == EAGAIN || errno == EINTR)) continue;
	if(res < 1) break;
	res = so._sendto(sock, (char *)buf + sent, bufsize - sent, 0, sin, SASIZE(sin));
	if(res < 0) {
		if(errno !=  EAGAIN && errno != EINTR) break;
		continue;
	}
	sent += res;
 } while (sent < bufsize);
 return sent;
}

int sockrecvfrom(SOCKET sock, struct sockaddr * sin, unsigned char * buf, int bufsize, int to){
	struct pollfd fds;
	SASIZETYPE sasize;
	int res;

	fds.fd = sock;
	fds.events = POLLIN;
	if(conf.timetoexit) return EOF;
	if (so._poll(&fds, 1, to)<1) return 0;
	sasize = SASIZE(sin);
	do {
		res = so._recvfrom(sock, (char *)buf, bufsize, 0, (struct sockaddr *)sin, &sasize);
	} while (res < 0 && (errno == EAGAIN || errno == EINTR));
	return res;
}

int sockgetcharcli(struct clientparam * param, int timeosec, int timeousec)
{
	if (param->cliinbuf && param->clioffset < param->cliinbuf)
	{
		return (int)param->local_buf[param->clioffset++];
	}
	return (int)*param->local_buf;
}


int sockgetcharsrv(struct clientparam * param, int timeosec, int timeousec){
	int len;
	int bufsize;

	if(!param->srvbuf){
		bufsize = BUFSIZE;
		if(param->ndatfilterssrv > 0 && bufsize < 32768) bufsize = 32768;
		if (!(param->srvbuf = (unsigned char *)myalloc(bufsize))) return 0;
		param->srvbufsize = bufsize;
		param->srvoffset = param->srvinbuf = 0;
		
	}
	if(param->srvinbuf && param->srvoffset < param->srvinbuf){
		return (int)param->srvbuf[param->srvoffset++];
	}
	param->srvoffset = param->srvinbuf = 0;
	if ((len = sockrecvfrom(param->remote_conn.socket, (struct sockaddr *)&param->sins, param->srvbuf, param->srvbufsize, timeosec * 1000 + timeousec)) <= 0) return EOF;
	param->srvinbuf = len;
	param->srvoffset = 1;
	param->nreads++;
	param->statssrv64 += len;
	return (int)*param->srvbuf;
}

int sockgetlinebuf(struct clientparam * param, DIRECTION which, char * buf, int bufsize, int delim, int to){
 int c;
 int i=0;
 if(bufsize<2) return 0;
 c = (which)?sockgetcharsrv(param, to, 0):sockgetcharcli(param, to, 0);
 if (c == EOF) {
	return 0;
 }
 do {
	 buf[i++] = c;
	 if (delim != EOF && c == delim) break;
 } while (i < bufsize && (c = (which) ? sockgetcharsrv(param, to, 0) : sockgetcharcli(param, to, 0)) != EOF);
 return i;
}

