/*
   3APA3A simpliest proxy server
   (c) 2002-2008 by ZARAZA <3APA3A@security.nnov.ru>

   please read License Agreement

   $Id: proxy.c,v 1.107 2014-04-07 20:35:07 vlad Exp $
*/


#include "proxy.h"

extern struct clientparam defparam;
extern struct srvparam srv;

#define RETURN(xxx) do_clear(pclient, xxx);return xxx;

char * proxy_stringtable[] = {
/* 0 */	"HTTP/1.0 400 Bad Request\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>400 Bad Request</title></head>\r\n"
	"<body><h2>400 Bad Request</h2></body></html>\r\n",

/* 1 */	"HTTP/1.0 502 Bad Gateway\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>502 Bad Gateway</title></head>\r\n"
	"<body><h2>502 Bad Gateway</h2><h3>Host Not Found or connection failed</h3></body></html>\r\n",

/* 2 */	"HTTP/1.0 503 Service Unavailable\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>503 Service Unavailable</title></head>\r\n"
	"<body><h2>503 Service Unavailable</h2><h3>You have exceeded your traffic limit</h3></body></html>\r\n",

/* 3 */	"HTTP/1.0 503 Service Unavailable\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>503 Service Unavailable</title></head>\r\n"
	"<body><h2>503 Service Unavailable</h2><h3>Recursion detected</h3></body></html>\r\n",

/* 4 */	"HTTP/1.0 501 Not Implemented\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>501 Not Implemented</title></head>\r\n"
	"<body><h2>501 Not Implemented</h2><h3>Required action is not supported by proxy server</h3></body></html>\r\n",

/* 5 */	"HTTP/1.0 502 Bad Gateway\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>502 Bad Gateway</title></head>\r\n"
	"<body><h2>502 Bad Gateway</h2><h3>Failed to connect parent proxy</h3></body></html>\r\n",

/* 6 */	"HTTP/1.0 500 Internal Error\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>500 Internal Error</title></head>\r\n"
	"<body><h2>500 Internal Error</h2><h3>Internal proxy error during processing your request</h3></body></html>\r\n",

/* 7 */	"HTTP/1.0 407 Proxy Authentication Required\r\n"
	"Proxy-Authenticate: Basic realm=\"proxy\"\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>407 Proxy Authentication Required</title></head>\r\n"
	"<body><h2>407 Proxy Authentication Required</h2><h3>Access to requested resource disallowed by administrator or you need valid username/password to use this resource</h3></body></html>\r\n",

/* 8 */	"HTTP/1.0 200 Connection established\r\n\r\n",

/* 9 */	"HTTP/1.0 200 Connection established\r\n"
	"Content-Type: text/html\r\n\r\n",

/* 10*/	"HTTP/1.0 404 Not Found\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>404 Not Found</title></head>\r\n"
	"<body><h2>404 Not Found</h2><h3>File not found</body></html>\r\n",
	
/* 11*/	"HTTP/1.0 403 Forbidden\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>403 Access Denied</title></head>\r\n"
	"<body><h2>403 Access Denied</h2><h3>Access control list denies you to access this resource</body></html>\r\n",

/* 12*/	"HTTP/1.0 407 Proxy Authentication Required\r\n"
#ifndef NOCRYPT
	"Proxy-Authenticate: NTLM\r\n"
#endif
	"Proxy-Authenticate: basic realm=\"proxy\"\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>407 Proxy Authentication Required</title></head>\r\n"
	"<body><h2>407 Proxy Authentication Required</h2><h3>Access to requested resource disallowed by administrator or you need valid username/password to use this resource</h3></body></html>\r\n",

/* 13*/	"HTTP/1.0 407 Proxy Authentication Required\r\n"
	"Proxy-Connection: keep-alive\r\n"
	"Content-Length: 0\r\n"
	"Proxy-Authenticate: NTLM ",

/* 14*/	"HTTP/1.0 403 Forbidden\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<pre>",

/* 15*/	"HTTP/1.0 503 Service Unavailable\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>503 Service Unavailable</title></head>\r\n"
	"<body><h2>503 Service Unavailable</h2><h3>Your request violates configured policy</h3></body></html>\r\n",

/* 16*/	"HTTP/1.0 401 Authentication Required\r\n"
	"Proxy-Authenticate: basic realm=\"FTP Server\"\r\n"
	"Proxy-Connection: close\r\n"
	"Content-type: text/html; charset=us-ascii\r\n"
	"\r\n"
	"<html><head><title>401 FTP Server requires authentication</title></head>\r\n"
	"<body><h2>401 FTP Server requires authentication</h2><h3>This FTP server rejects anonymous access</h3></body></html>\r\n",

/* 17*/ "HTTP/1.1 100 Continue\r\n"
	"\r\n",

	NULL
};

void local_alloc_cb(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf);
void remote_close_cb(uv_handle_t* handle);
void do_close(struct clientparam * pclient);
void do_clear(struct clientparam * pclient, int rescode);
void local_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* rbuf);
void remote_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
void remote_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* rbuf);
void local_write_cb(uv_write_t* req, int status);
void remote_write_cb(uv_write_t* req, int status);
int do_local_read(struct clientparam * pclient);
int do_remote_read(struct clientparam * pclient);
int do_local_write(struct clientparam * pclient, const uv_buf_t * bufs, unsigned int nbufs);
int do_local_write(struct clientparam * pclient, unsigned char * buf, int buflen);
int do_remote_write(struct clientparam * pclient, const uv_buf_t * bufs, unsigned int nbufs);

int onrecv_clientrequest(struct clientparam * pclient);
int on_serverconnected(struct clientparam * pclient);

static void logclienterr(struct clientparam * param, char * format, ...)
{
	fprintf(stderr, "%p>>", param);
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	fflush(stderr);
}

static void logerr(char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	fflush(stderr);
}

static void logurl(struct clientparam * param, char * buf, char * req, int ftp){
 char *sb;
 char *se;
 int len;

 if (param->srv->logfunc == NULL)return;
 if(!buf) req = NULL;
 if(req) {
	len = (int)strlen(req);
	if(len > (LINESIZE - 1)) len = LINESIZE - 1;
	memcpy(buf, req, len + 1);
	buf[LINESIZE - 1] = 0;
	sb = strchr(buf, '\r');
	if(sb)*sb = 0;
	if(ftp && (se = strchr(buf + 10, ':')) && (sb = strchr(se, '@')) ) {
		strcpy(se, sb);
	}
 }
 if(param->res != 555)(*param->srv->logfunc)(param, (unsigned char *)(req?buf:NULL));
}

void decodeurl(char *s, int allowcr){
 char *d = s;
 unsigned u;

 while(*s){
	if(*s == '%' && ishex(s[1]) && ishex(s[2])){
		sscanf((char *)s+1, "%2x", &u);
		if(allowcr && u != '\r')*d++ = u;
		else if (u != '\r' && u != '\n') {
			if (u == '\"' || u == '\\') *d++ = '\\';
			else if (u == 255) *d++ = 255;
			*d++ = u;
		}
		s+=3;
	}
	else if(!allowcr && *s == '?') {
		break;
	}
	else if(*s == '+') {
		*d++ = ' ';
		s++;
	}
	else {
		*d++ = *s++;
	}
 }
 *d = 0;
}

void file2url(unsigned char *sb, unsigned char *buf, unsigned bufsize, int * inbuf, int skip255){
 for(; *sb; sb++){
	if((bufsize - *inbuf)<16)break;
	if(*sb=='\r'||*sb=='\n')continue;
	if(isallowed(*sb))buf[(*inbuf)++]=*sb;
	else if(*sb == '\"'){
		memcpy(buf+*inbuf, "%5C%22", 6);
		(*inbuf)+=6;
	}
	else if(skip255 && *sb == 255 && *(sb+1) == 255) {
		memcpy(buf+*inbuf, "%ff", 3);
		(*inbuf)+=3;
		sb++;
        }
	else {
		sprintf((char *)buf+*inbuf, "%%%.2x", (unsigned)*sb);
		(*inbuf)+=3;
	}
 }
}

static void local_alloc_cb(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf)
{
	ASSERT(handle != NULL);
	struct clientparam * param = (struct clientparam *)handle->data;
	if (param->cliinbuf == param->clioffset)
	{
		param->cliinbuf = param->clioffset = 0;
	}
	else if (param->clioffset)
	{
		memmove(param->local_buf, param->local_buf + param->clioffset, param->cliinbuf - param->clioffset);
		param->cliinbuf -= param->clioffset;
		param->clioffset = 0;
	}

	ASSERT(param->cliinbuf < sizeof(param->local_buf));
	if (param->cliinbuf >= sizeof(param->local_buf))
	{
		buf->len = 0;
		buf->base = 0;
		return;
	}

	buf->len = sizeof(param->local_buf) - param->cliinbuf;
	buf->base = param->local_buf + param->cliinbuf;
	memset(buf->base, 0, buf->len);
	ASSERT(buf->base != NULL);
}

static void remote_close_cb(uv_handle_t* handle)
{
	struct clientparam * pclient = (struct clientparam *)handle->data;

	//freeparam(pclient);
}

void do_close(struct clientparam * pclient)
{
	uv_read_stop((uv_stream_t*)&pclient->local_conn);
	uv_read_stop((uv_stream_t*)&pclient->remote_conn);

	uv_shutdown((uv_shutdown_t*)&pclient->local_shutdown_req, (uv_stream_t*)&pclient->local_conn, NULL);
	uv_shutdown((uv_shutdown_t*)&pclient->remote_shutdown_req, (uv_stream_t*)&pclient->remote_conn, NULL);

	uv_close((uv_handle_t*)&pclient->local_conn, NULL);
	uv_close((uv_handle_t*)&pclient->remote_conn, remote_close_cb);
}

void do_clear(struct clientparam * pclient, int rescode)
{
	pclient->res = rescode; 
	do_close(pclient);
}

static void local_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* rbuf)
{
	if (nread == 0)
	{
		//WSAEWOULDBLOCK
		return;
	}

	ASSERT(stream != NULL);
	struct clientparam * pclient = (struct clientparam *)stream->data;
	if (nread < 0)
	{
		LOG_CLIENT_ERR(pclient, "local_read_cb error: %s\n", uv_err_name((int)nread));
		ASSERT(nread == UV_ECONNRESET || nread == UV_EOF);
		
		do_close(pclient);
		return;
	}

	pclient->cliinbuf += nread;

	switch (pclient->step)
	{
	case CLIENT_STEP_NONE:
		if (onrecv_clientrequest(pclient) == 0)
		{
			pclient->step = CLIENT_STEP_CONNECTING;
		}
		break;
	default:
		break;
	}
}

static void remote_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	ASSERT(handle != NULL);
	struct clientparam * param = (struct clientparam *)handle->data;
	if (param->srvinbuf == param->srvoffset)
	{
		param->srvinbuf = param->srvoffset = 0;
	}
	else if (param->srvoffset)
	{
		memmove(param->remote_buf, param->remote_buf + param->srvoffset, param->srvinbuf - param->srvoffset);
		param->srvinbuf -= param->srvoffset;
		param->srvoffset = 0;
	}

	ASSERT(param->srvinbuf < sizeof(param->remote_buf));
	if (param->srvinbuf >= sizeof(param->remote_buf))
	{
		buf->len = 0;
		buf->base = 0;
		return;
	}

	buf->len = sizeof(param->remote_buf) - param->srvinbuf;
	buf->base = param->remote_buf + param->srvinbuf;
	memset(buf->base, 0, buf->len);
	ASSERT(buf->base != NULL);
}

static void remote_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* rbuf)
{
	if (nread == 0)
	{
		//WSAEWOULDBLOCK
		return;
	}

	ASSERT(stream != NULL);
	struct clientparam * pclient = (struct clientparam *)stream->data;
	if (nread < 0)
	{
		LOG_CLIENT_ERR(pclient, "remote_read_cb error: %s\n", uv_err_name((int)nread));
		ASSERT(nread == UV_ECONNRESET || nread == UV_EOF);

		do_close(pclient);
		return;
	}

	pclient->srvinbuf += nread;

	DEBUG_LOG("proxy=%d\n", nread);

	switch (pclient->step)
	{
	case CLIENT_STEP_CONNECTED:
		do_local_write(pclient, rbuf, 1);
		pclient->srvinbuf = pclient->srvoffset = 0;
		break;
	default:
		break;
	}
}

static void local_write_cb(uv_write_t* req, int status)
{

}

static void remote_write_cb(uv_write_t* req, int status)
{

}

int do_local_read(struct clientparam * pclient)
{
	int r = 0;
	r = uv_read_start((uv_stream_t*)&pclient->local_conn, local_alloc_cb, local_read_cb);
	ASSERT(r == 0);
	return r;
}

int do_remote_read(struct clientparam * pclient)
{
	int r = 0;
	r = uv_read_start((uv_stream_t*)&pclient->remote_conn, remote_alloc_cb, remote_read_cb);
	ASSERT(r == 0);
	return r;
}

int do_local_write(struct clientparam * pclient, const uv_buf_t * bufs, unsigned int nbufs)
{
	int r = 0;
	r = uv_write((uv_write_t *)&pclient->local_write_req, (uv_stream_t*)&pclient->local_conn, bufs, nbufs, local_write_cb);
	ASSERT(r == 0);
	return r;
}

int do_local_write(struct clientparam * pclient, unsigned char * buf, int buflen)
{
	int r = 0;
	uv_buf_t bufs;
	bufs.base = (char *)buf;
	bufs.len = buflen;
	r = uv_write((uv_write_t *)&pclient->local_write_req, (uv_stream_t*)&pclient->local_conn, &bufs, 1, local_write_cb);
	ASSERT(r == 0);
	return r;
}

int do_remote_write(struct clientparam * pclient, const uv_buf_t * bufs, unsigned int nbufs)
{
	int r = 0;
	r = uv_write((uv_write_t *)&pclient->remote_write_req, (uv_stream_t*)&pclient->remote_conn, bufs, nbufs, remote_write_cb);
	ASSERT(r == 0);
	return r;
}

int do_remote_write(struct clientparam * pclient, unsigned char * buf, int buflen)
{
	int r = 0;
	uv_buf_t bufs;
	bufs.base = (char *)buf;
	bufs.len = buflen;
	r = uv_write((uv_write_t *)&pclient->remote_write_req, (uv_stream_t*)&pclient->remote_conn, &bufs, 1, remote_write_cb);
	ASSERT(r == 0);
	return r;
}

void procy_cb(uv_work_t* req)
{
	int r = 0;
	struct clientparam * pclient = (struct clientparam *)req->data;
	ASSERT(r == 0);
	do_local_read(pclient);
}

void on_remote_connect(uv_connect_t* req, int status)
{
	ASSERT(req != NULL);

	struct clientparam * pclient = (struct clientparam *)req->data;
	if (status != 0)
	{
		switch (status)
		{
		case UV_ETIMEDOUT:
			LOG_CLIENT_ERR(pclient, "err>>connect failed! time out, status=%d, addr=%s:%d\n", status,
				inet_ntoa(pclient->sins.sin_addr), ntohs(pclient->sins.sin_port));
			break; 
		case UV_ECANCELED:
			LOG_CLIENT_ERR(pclient, "err>>connect failed! canceled, status=%d, addr=%s:%d\n", status,
				inet_ntoa(pclient->sins.sin_addr), ntohs(pclient->sins.sin_port));
			break;
		case UV_EADDRNOTAVAIL:
			LOG_CLIENT_ERR(pclient, "err>>connect failed! addr not available, status=%d, addr=%s:%d\n", status,
				inet_ntoa(pclient->sins.sin_addr), ntohs(pclient->sins.sin_port));
			break;
		default:
			LOG_CLIENT_ERR(pclient, "err>>connect failed! status=%d, addr=%s:%d\n", status,
				inet_ntoa(pclient->sins.sin_addr), ntohs(pclient->sins.sin_port));
			break;
		}

		if (uv_is_closing((uv_handle_t*)&pclient->remote_conn))
		{
			return;
		}

		do_close(pclient);
		return;
	}

	struct clientparam * param = pclient;
	struct sockaddr_in bindsa;
	SASIZETYPE size = sizeof(param->sins);
	memset(&bindsa, 0, sizeof(bindsa));
	bindsa.sin_family = AF_INET;
	bindsa.sin_port = param->extport;
	bindsa.sin_addr.s_addr = param->extip;
	if (param->srv->targetport && !bindsa.sin_port && ntohs(*SAPORT(&param->sincr)) > 1023) bindsa.sin_port = *SAPORT(&param->sincr);

	if (param->operation >= 256 || (param->operation & CONNECT)){
#ifdef _WIN32
		unsigned long ul = 1;
#endif
		param->nconnects++;
#ifdef _WIN32
		ioctlsocket(param->remote_conn.socket, FIONBIO, &ul);
#else
		fcntl(param->remote_conn.socket, F_SETFL, O_NONBLOCK);
#endif
		if (so._getsockname(param->remote_conn.socket, (struct sockaddr *)&bindsa, &size) == -1)
		{
			do_close(pclient);
			return;
		}
		param->extip = bindsa.sin_addr.s_addr;
	}
	else 
	{
		if (so._getsockname(param->remote_conn.socket, (struct sockaddr *)&param->sins, &size) == -1)
		{
			do_close(pclient);
			return;
		}
	}

	pclient->step = CLIENT_STEP_CONNECTED;
	if (on_serverconnected(pclient) == 0)
	{
		do_remote_read(pclient);
	}
}

int on_serverconnected(struct clientparam * pclient)
{
	struct clientparam * param = pclient;
	SASIZETYPE sasize;
	int & ftp = pclient->ftp;
	int & ckeepalive = pclient->ckeepalive;
	int & isconnect = pclient->isconnect;
	int res = 0;
	int redirect = 0;
	char *& req = pclient->reqbuf;
	int reqlen = 0;
	int & inbuf = pclient->inbuf;
	int anonymous = param->srv->singlepacket;
	char * buf = pclient->buf;
	int & keepalive = pclient->keepalive;
	unsigned char username[1024];

	if (*SAFAMILY(&param->srv->intsa) == AF_INET &&
		(param->sins.sin_addr.s_addr == ((struct sockaddr_in *)&param->srv->intsa)->sin_addr.s_addr && param->sins.sin_port == *SAPORT(&param->srv->intsa))) {
		RETURN(519);
	}
	sasize = sizeof(struct sockaddr_in);
	if (so._getpeername(param->remote_conn.socket, (struct sockaddr *)&param->sins, &sasize))
	{
		RETURN(520);
	}
#define FTPBUFSIZE 1536

/*	if (ftp && param->redirtype != R_HTTP)
	{
		SOCKET s;
		int mode = 0;
		int i = 0;
		char ftpbuf[FTPBUFSIZE];
		int inftpbuf = 0;

		if (!ckeepalive)
		{
			inftpbuf = FTPBUFSIZE - 20;
			res = ftplogin(param, ftpbuf, &inftpbuf);
			if (res)
			{
				if (res == 700 || res == 701)
				{
					socksend(param->clisock, (unsigned char *)proxy_stringtable[16], (int)strlen(proxy_stringtable[16]), conf.timeouts[STRING_S]);
					socksend(param->clisock, (unsigned char *)ftpbuf, inftpbuf, conf.timeouts[STRING_S]);
				}
				RETURN(res);
			}
		}
		ckeepalive = 1;
		if (ftpbase) myfree(ftpbase);
		ftpbase = NULL;
		if (!(sp = strchr((char *)ss, ' '))){ RETURN(799); }
		*sp = 0;

		decodeurl(ss, 0);
		i = (int)strlen((char *)ss);
		if (!(ftpbase = (unsigned char *)myalloc(i + 2))){ RETURN(21); }
		memcpy(ftpbase, ss, i);
		if (ftpbase[i - 1] != '/') ftpbase[i++] = '/';
		ftpbase[i] = 0;
		memcpy(buf, "<pre><hr>\n", 10);
		inbuf = 10;
		if (inftpbuf) {
			memcpy(buf + inbuf, ftpbuf, inftpbuf);
			inbuf += inftpbuf;
			memcpy(buf + inbuf, "<hr>", 4);
			inbuf += 4;
		}
		if (ftpbase[1] != 0){
			memcpy(buf + inbuf, "[<A HREF=\"..\">..</A>]\n", 22);
			inbuf += 22;
		}
		inftpbuf = FTPBUFSIZE - (20 + inftpbuf);
		res = ftpcd(param, ftpbase, ftpbuf, &inftpbuf);
		if (res){
			res = ftptype(param, (unsigned char *)"I");
			if (res)RETURN(res);
			ftpbase[--i] = 0;
			ftps = ftpcommand(param, param->operation == FTP_PUT ? (unsigned char *)"PUT" : (unsigned char *)"RETR", ftpbase);
		}
		else {
			if (inftpbuf){
				memcpy(buf + inbuf, ftpbuf, inftpbuf);
				inbuf += inftpbuf;
				memcpy(buf + inbuf, "<hr>", 4);
				inbuf += 4;
			}
			ftps = ftpcommand(param, (unsigned char *)"LIST", NULL);
			mode = 1;
		}
		if (ftps == INVALID_SOCKET){ RETURN(780); }
		if (!mode){
			socksend(param->clisock, (unsigned char *)proxy_stringtable[8], (int)strlen(proxy_stringtable[8]), conf.timeouts[STRING_S]);
			s = param->remsock;
			param->remsock = ftps;
			if ((param->operation == FTP_PUT) && (contentlength64 > 0)) param->waitclient64 = contentlength64;
			res = sockmap(param, conf.timeouts[CONNECTION_L]);
			if (res == 99) res = 0;
			so._closesocket(ftps);
			ftps = INVALID_SOCKET;
			param->remsock = s;
		}
		else {
			int headsent = 0;
			int gotres = -1;

			s = param->remsock;
			if (param->srvoffset < param->srvinbuf){
				gotres = ftpres(param, (unsigned char *)buf + inbuf, bufsize - (inbuf + 100));
				if (gotres) inbuf = (int)strlen((char *)buf);
			}

			param->remsock = ftps;
			if (gotres <= 0) for (; (res = sockgetlinebuf(param, SERVER, ftpbuf, FTPBUFSIZE - 20, '\n', conf.timeouts[STRING_S])) > 0; i++){
				int isdir = 0;
				int islink = 0;
				int filetoken = -1;
				int sizetoken = -1;
				int modetoken = -1;
				int datetoken = -1;
				int spaces = 1;
				char * tokens[10];
				unsigned wordlen[10];
				unsigned char j = 0;
				int space = 1;

				ftpbuf[res] = 0;
				if (!i && ftpbuf[0] == 't' && ftpbuf[1] == 'o' && ftpbuf[2] == 't'){
					mode = 2;
					continue;
				}
				if (!isnumber(*ftpbuf) && mode == 1) mode = 2;
				for (sb = ftpbuf; *sb; sb++){
					if (!space && isspace(*sb)){
						space = 1;
						wordlen[j] = (unsigned)(sb - tokens[j]);
						j++;
					}
					if (space && !isspace(*sb)){
						space = 0;
						tokens[j] = sb;
						if (j == 8)break;
					}
				}
				if (mode == 1){
					if (j < 4) continue;
					if (!(isdir = !memcmp(tokens[2], "<DIR>", wordlen[2])) && !isnumber(*tokens[2])){
						continue;
					}
					datetoken = 0;
					wordlen[datetoken] = ((unsigned)(tokens[1] - tokens[0])) + wordlen[1];
					sizetoken = 2;
					filetoken = 3;
					spaces = 10;
				}
				else {
					if (j < 8 || wordlen[0] != 10) continue;
					if (j < 8 || !isnumber(*tokens[4])) mode = 3;
					if (*tokens[0] == 'd') isdir = 1;
					if (*tokens[0] == 'l') islink = 1;
					modetoken = 0;
					sizetoken = (mode == 2) ? 4 : 3;
					filetoken = (mode == 2) ? 8 : 7;
					datetoken = (mode == 2) ? 5 : 4;
					tokens[filetoken] = tokens[filetoken - 1];
					while (*tokens[filetoken] && !isspace(*tokens[filetoken]))tokens[filetoken]++;
					if (*tokens[filetoken]){
						tokens[filetoken]++;
					}
					wordlen[datetoken] = (unsigned)(tokens[filetoken] - tokens[datetoken]);
					wordlen[filetoken] = (unsigned)strlen((char *)tokens[filetoken]);
				}

				if (modetoken >= 0) memcpy(buf + inbuf, tokens[modetoken], 11);
				else memcpy(buf + inbuf, "---------- ", 11);
				inbuf += 11;
				if ((int)wordlen[datetoken] + 256 > bufsize - inbuf) continue;
				memcpy(buf + inbuf, tokens[datetoken], wordlen[datetoken]);
				inbuf += wordlen[datetoken];
				if (isdir){
					memcpy(buf + inbuf, "       DIR", 10);
					inbuf += 10;
				}
				else if (islink){
					memcpy(buf + inbuf, "      LINK", 10);
					inbuf += 10;
				}
				else{
					unsigned k;
					if (wordlen[sizetoken] > 10) wordlen[sizetoken] = 10;
					for (k = 10; k > wordlen[sizetoken]; k--){
						buf[inbuf++] = ' ';
					}
					memcpy(buf + inbuf, tokens[sizetoken], wordlen[sizetoken]);
					inbuf += wordlen[sizetoken];
				}
				memcpy(buf + inbuf, " <A HREF=\"", 10);
				inbuf += 10;
				sb = NULL;
				if (islink) sb = strstr((char *)tokens[filetoken], " -> ");
				if (sb) sb += 4;

				else sb = (char *)tokens[filetoken];
				if (*sb != '/' && ftpbase)file2url(ftpbase, (unsigned char *)buf, bufsize, (int *)&inbuf, 1);
				file2url((unsigned char *)sb, (unsigned char *)buf, bufsize, (int *)&inbuf, 0);

				if (isdir)buf[inbuf++] = '/';
				memcpy(buf + inbuf, "\">", 2);
				inbuf += 2;
				for (sb = tokens[filetoken]; *sb; sb++){
					if ((bufsize - inbuf) < 16)break;
					if (*sb == '<'){
						memcpy(buf + inbuf, "&lt;", 4);
						inbuf += 4;
					}
					else if (*sb == '>'){
						memcpy(buf + inbuf, "&gt;", 4);
						inbuf += 4;
					}
					else if (*sb == '\r' || *sb == '\n'){
						continue;
					}
					else if (islink && sb[0] == ' ' && sb[1] == '-'
						&& sb[2] == '>'){
						memcpy(buf + inbuf, "</A> ", 5);
						inbuf += 5;
					}
					else buf[inbuf++] = *sb;
				}
				if (islink != 2){
					memcpy(buf + inbuf, "</A>", 4);
					inbuf += 4;
				}
				buf[inbuf++] = '\n';

				if ((bufsize - inbuf) < LINESIZE){
					if (bufsize > 20000){
						if (!headsent++){
							socksend(param->clisock, (unsigned char *)proxy_stringtable[9], (int)strlen(proxy_stringtable[9]), conf.timeouts[STRING_S]);
						}
						if ((unsigned)socksend(param->clisock, (unsigned char *)buf, inbuf, conf.timeouts[STRING_S]) != inbuf){
							RETURN(781);
						}
						inbuf = 0;
					}
					else {
						if (!(newbuf = (char *)myrealloc(buf, bufsize + BUFSIZE))){ RETURN(21); }
						buf = newbuf;
						bufsize += BUFSIZE;
					}
				}
			}
			memcpy(buf + inbuf, "<hr>", 4);
			inbuf += 4;
			so._closesocket(ftps);
			ftps = INVALID_SOCKET;
			param->remsock = s;
			if (inbuf){
				buf[inbuf] = 0;
				if (gotres < 0) res = ftpres(param, (unsigned char *)buf + inbuf, bufsize - inbuf);
				else res = gotres;
				inbuf = (int)strlen((char *)buf);
				if (!headsent){
					sprintf(ftpbuf,
						"HTTP/1.0 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"Proxy-Connection: keep-alive\r\n"
						"Content-Length: %d\r\n\r\n",
						inbuf);
					socksend(param->clisock, (unsigned char *)ftpbuf, (int)strlen(ftpbuf), conf.timeouts[STRING_S]);
				}
				socksend(param->clisock, (unsigned char *)buf, inbuf, conf.timeouts[STRING_S]);
				if (res){ RETURN(res); }
				if (!headsent)goto REQUESTEND;
			}
			RETURN(0);
		}
		RETURN(res);
	}
	*/

/*
	if (isconnect && param->redirtype != R_HTTP) 
	{
		do_local_write(param, (unsigned char *)proxy_stringtable[8], (int)strlen(proxy_stringtable[8]));
		if (param->redirectfunc) return (*param->redirectfunc)(param);
		param->res = sockmap(param, conf.timeouts[CONNECTION_L]);
		if (param->redirectfunc) return (*param->redirectfunc)(param);
		RETURN(param->res);
	}
	*/

	if (!req || param->redirtype != R_HTTP) 
	{
		reqlen = 0;
	}
	/*
	else 
	{
		redirect = 1;
		if (socksend(param->remsock, (unsigned char *)req, (res = (int)strlen((char *)req)), conf.timeouts[STRING_L]) != res) 
		{
			RETURN(518);
		}
		param->statscli64 += res;
		param->nwrites++;
	}
	*/
	inbuf = 0;
#ifndef ANONYMOUS
	if (anonymous != 1){
		sprintf((char*)buf + strlen((char *)buf), "Via: 1.1 ");
		gethostname((char *)(buf + strlen((char *)buf)), 256);
		sprintf((char*)buf + strlen((char *)buf), ":%d (%s %s)\r\nX-Forwarded-For: ", (int)ntohs(*SAPORT(&param->srv->intsa)), conf.stringtable ? conf.stringtable[2] : (unsigned char *)"", conf.stringtable ? conf.stringtable[3] : (unsigned char *)"");
		if (!anonymous)myinet_ntop(*SAFAMILY(&param->sincr), SAADDR(&param->sincr), (char *)buf + strlen((char *)buf), 64);
		else {
			unsigned long tmp;

			tmp = ((unsigned long)myrand(param, sizeof(struct clientparam)) << 16) ^ (unsigned long)rand();
			myinet_ntop(AF_INET, &tmp, (char *)buf + strlen((char *)buf), 64);
		}
		sprintf((char*)buf + strlen((char *)buf), "\r\n");
	}
#endif
	if (keepalive <= 1) sprintf((char*)buf + strlen((char *)buf), "%s: %s\r\n", (param->redirtype == R_HTTP) ? "Proxy-Connection" : "Connection", keepalive ? "keep-alive" : "close");
	if (param->extusername){
		sprintf((char*)buf + strlen((char *)buf), "%s: basic ", (redirect) ? "Proxy-Authorization" : "Authorization");
		sprintf((char*)username, "%.32s:%.64s", param->extusername, param->extpassword ? param->extpassword : (unsigned char*)"");
		en64(username, (unsigned char *)buf + strlen((char *)buf), (int)strlen((char *)username));
		sprintf((char*)buf + strlen((char *)buf), "\r\n");
	}
	sprintf((char*)buf + strlen((char *)buf), "\r\n");
	if ((res = do_remote_write(param, (unsigned char *)buf + reqlen, (int)strlen((char *)buf + reqlen)) != 0))
	{
		RETURN(518);
	}

	return 0;
}

int onrecv_clientrequest(struct clientparam * pclient)
{
	int i = 0;
	char	*sb = NULL, *sg = NULL, *se = NULL, *sp = NULL,
		*su = NULL, *ss = NULL;
	char *& req = pclient->reqbuf;
	char * buf = pclient->buf;
	int & isconnect = pclient->isconnect;
	int & ftp = pclient->ftp;
	int & inbuf = pclient->inbuf;
	int & keepalive = pclient->keepalive;
	int & ckeepalive = pclient->ckeepalive;
	struct clientparam * param = pclient;
	int prefix = 0;
	int reqlen = 0;
	char *& newbuf = pclient->newbuf;
	uint64_t & contentlength64 = pclient->contentlength64;
	unsigned char username[1024];
	int bufsize = sizeof(pclient->buf);
	int reqsize, reqbufsize;
	int res = 0;

	while (true)
	{
		i = sockgetlinebuf(pclient, CLIENT, pclient->buf, sizeof(pclient->buf) - 1, '\n', conf.timeouts[STRING_L]);
		if (i <= 0)
		{
			ASSERT(FALSE);
			RETURN((pclient->keepalive) ? 555 : (i) ? 507 : 508);
		}

		if (i == 2 && pclient->buf[0] == '\r' && pclient->buf[1] == '\n')
		{
			continue;
		}

		pclient->buf[i] = 0;


		if (req)
		{
// 			if (!param->transparent && !param->srv->transparent && param->redirtype != R_HTTP && (i <= prefix || strncasecmp((char *)buf, (char *)req, prefix)))
// 			{
// 				ckeepalive = 0;
// 				if (param->remsock != INVALID_SOCKET){
// 					so._shutdown(param->remsock, SHUT_RDWR);
// 					so._closesocket(param->remsock);
// 				}
// 				param->sins.sin_addr.s_addr = 0;
// 				param->sins.sin_port = 0;
// 				param->remsock = INVALID_SOCKET;
// 				param->redirected = 0;
// 				param->redirtype = (REDIRTYPE)0;
// 			}
// 			myfree(req);
		}

		req = mystrdup((char *)pclient->buf);
		if (!req)
		{
			RETURN(510);
		}
		if (i < 10) {
			RETURN(511);
		}

		if (pclient->buf[i - 3] == '1') pclient->keepalive = 2;
		if ((isconnect = !strncasecmp((char *)pclient->buf, "CONNECT", 7))) pclient->keepalive = 2;

		if ((sb = strchr((char *)pclient->buf, ' ')) == NULL) { RETURN(512); }
		ss = ++sb;
		if (!isconnect) {
			printf("conn====>%s\n", ss);
			if (!strncasecmp((char *)sb, "http://", 7)) {
				sb += 7;
			}
			else if (!strncasecmp((char *)sb, "ftp://", 6)) {
				pclient->ftp = 1;
				sb += 6;
			}
			else if (*sb == '/') {
				pclient->transparent = 1;
			}
			else {
				RETURN(513);
			}
		}
		else {
			if ((se = strchr((char *)sb, ' ')) == NULL || sb == se) { RETURN(514); }
			*se = 0;
		}

		if (!param->transparent || isconnect) {
			if (!isconnect) {
				if ((se = strchr((char *)sb, '/')) == NULL
					|| sb == se
					|| !(sg = strchr((char *)sb, ' '))) {
					RETURN(515);
				}
				if (se > sg) se = sg;
				*se = 0;
			}
			prefix = (int)(se - buf);
			su = strrchr((char *)sb, '@');
			if (su) {
				su = mystrdup(sb);
				decodeurl(su, 0);
				parseconnusername((char *)su, (struct clientparam *)param, 1, (unsigned short)((ftp) ? 21 : 80));
				myfree(su);
			}
			else parsehostname((char *)sb, (struct clientparam *)param, (unsigned short)((ftp) ? 21 : 80));
			if (!isconnect){
				if (se == sg)*se-- = ' ';
				*se = '/';
				memmove(ss, se, i - (se - sb) + 1);
			}
		}
		reqlen = i = (int)strlen((char *)buf);
		if (!strncasecmp((char *)buf, "CONNECT", 7))param->operation = HTTP_CONNECT;
		else if (!strncasecmp((char *)buf, "GET", 3))param->operation = (ftp) ? FTP_GET : HTTP_GET;
		else if (!strncasecmp((char *)buf, "PUT", 3))param->operation = (ftp) ? FTP_PUT : HTTP_PUT;
		else if (!strncasecmp((char *)buf, "POST", 4) || !strncasecmp((char *)buf, "BITS_POST", 9))param->operation = HTTP_POST;
		else if (!strncasecmp((char *)buf, "HEAD", 4))param->operation = HTTP_HEAD;
		else param->operation = HTTP_OTHER;

		do {
			buf[inbuf + i] = 0;
			/*printf("Got: %s\n", buf+inbuf);*/
#ifndef WITHMAIN
			if (i > 25 && !param->srv->transparent && (!strncasecmp((char *)(buf + inbuf), "proxy-authorization", 19))){
				sb = (unsigned char *)strchr((char *)(buf + inbuf), ':');
				if (!sb)continue;
				++sb;
				while (isspace(*sb))sb++;
				if (!*sb) continue;
				if (!strncasecmp((char *)sb, "basic", 5)){
					sb += 5;
					while (isspace(*sb))sb++;
					i = de64(sb, username, 255);
					if (i <= 0)continue;
					username[i] = 0;
					sb = (unsigned char *)strchr((char *)username, ':');
					if (sb){
						*sb = 0;
						if (param->password)myfree(param->password);
						param->password = (unsigned char *)mystrdup((char *)sb + 1);
						param->pwtype = 0;
					}
					if (param->username)myfree(param->username);
					param->username = (unsigned char *)mystrdup((char *)username);
					continue;
				}
#ifndef NOCRYPT
				if (param->srv->usentlm && !strncasecmp((char *)sb, "ntlm", 4)){
					sb += 4;
					while (isspace(*sb))sb++;
					i = de64(sb, username, 1023);
					if (i <= 16)continue;
					username[i] = 0;
					if (strncasecmp((char *)username, "NTLMSSP", 8)) continue;
					if (username[8] == 1) {
						while ((i = sockgetlinebuf(param, CLIENT, buf, BUFSIZE - 1, '\n', conf.timeouts[STRING_S])) > 2){
							if (i > 15 && (!strncasecmp((char *)(buf), "content-length", 14))){
								buf[i] = 0;
								sscanf((char *)buf + 15, "%"PRINTF_INT64_MODIFIER"u", &contentlength64);
							}
						}
						while (contentlength64 > 0 && (i = sockgetlinebuf(param, CLIENT, buf, (BUFSIZE < contentlength64) ? BUFSIZE - 1 : (int)contentlength64, '\n', conf.timeouts[STRING_S])) > 0){
							if ((uint64_t)i > contentlength64) break;
							contentlength64 -= i;
						}
						contentlength64 = 0;
						if (param->password)myfree(param->password);
						param->password = myalloc(32);
						param->pwtype = 2;
						i = (int)strlen(proxy_stringtable[13]);
						memcpy(buf, proxy_stringtable[13], i);
						genchallenge(param, (char *)param->password, (char *)buf + i);
						memcpy(buf + strlen((char *)buf), "\r\n\r\n", 5);
						socksend(param->clisock, buf, (int)strlen((char *)buf), conf.timeouts[STRING_S]);
						ckeepalive = keepalive = 1;
						goto REQUESTEND;
					}
					if (username[8] == 3 && param->pwtype == 2 && i >= 80) {
						unsigned offset, len;

						len = username[20] + (((unsigned)username[21]) << 8);
						offset = username[24] + (((unsigned)username[25]) << 8);
						if (len != 24 || len + offset > (unsigned)i) continue;
						memcpy(param->password + 8, username + offset, 24);
						len = username[36] + (((unsigned)username[37]) << 8);
						offset = username[40] + (((unsigned)username[41]) << 8);
						if (len > 255 || len + offset > (unsigned)i) continue;
						if (param->username) myfree(param->username);
						unicode2text((char *)username + offset, (char *)username + offset, (len >> 1));
						param->username = (unsigned char *)mystrdup((char *)username + offset);
					}
					continue;
				}
#endif
			}
#endif
			if (!isconnect && (
				(i > 25 && !strncasecmp((char *)(buf + inbuf), "proxy-connection:", 17))
				||
				(i > 16 && (!strncasecmp((char *)(buf + inbuf), "connection:", 11)))
				)){
				sb = strchr((char *)(buf + inbuf), ':');
				if (!sb)continue;
				++sb;
				while (isspace(*sb))sb++;
				if (!strncasecmp((char *)sb, "keep-alive", 10))keepalive = 1;
				else keepalive = 0;
				continue;
			}
			if (i > 11 && !strncasecmp((char *)(buf + inbuf), "Expect: 100", 11)){
				keepalive = 1;
				do_local_write(param, (unsigned char *)proxy_stringtable[17], (int)strlen(proxy_stringtable[17]));
				continue;
			}
			if (param->transparent && i > 6 && !strncasecmp((char *)buf + inbuf, "Host:", 5)){
				sb = strchr((char *)(buf + inbuf), ':');
				if (!sb)continue;
				++sb;
				while (isspace(*sb))sb++;
				se = strchr((char *)sb, '\r');
				if (se) *se = 0;
				if (!param->hostname){
					parsehostname((char *)sb, param, 80);
				}
				newbuf = (char *)myalloc(strlen((char *)req) + strlen((char *)(buf + inbuf)) + 8);
				if (newbuf){
					sp = (char *)strchr((char *)req + 1, '/');
					memcpy(newbuf, req, (sp - req));
					sprintf((char*)newbuf + (sp - req), "http://%s%s", sb, sp);
					myfree(req);
					req = newbuf;
				}
				if (se)*se = '\r';
			}
			if (ftp && i > 19 && (!strncasecmp((char *)(buf + inbuf), "authorization", 19))){
				sb = (char *)strchr((char *)(buf + inbuf), ':');
				if (!sb)continue;
				++sb;
				while (isspace(*sb))sb++;
				if (!*sb) continue;
				if (!strncasecmp((char *)sb, "basic", 5)){
					sb += 5;
					while (isspace(*sb))sb++;
					i = de64(sb, username, 255);
					if (i <= 0)continue;
					username[i] = 0;
					sb = strchr((char *)username, ':');
					if (sb){
						*sb = 0;
						if (param->extpassword)myfree(param->extpassword);
						param->extpassword = (unsigned char *)mystrdup((char *)sb + 1);
					}
					if (param->extusername)myfree(param->extusername);
					param->extusername = (unsigned char *)mystrdup((char *)username);
					continue;
				}
			}
			if (i > 15 && (!strncasecmp((char *)(buf + inbuf), "content-length", 14))){
				sb = strchr((char *)(buf + inbuf), ':');
				if (!sb)continue;
				++sb;
				while (isspace(*sb))sb++;
				sscanf(sb, "%"PRINTF_INT64_MODIFIER"u", &contentlength64);
				if (param->maxtrafout64 && (param->maxtrafout64 < param->statscli64 || contentlength64 > param->maxtrafout64 - param->statscli64)){
					RETURN(10);
				}
				if (param->ndatfilterscli > 0 && contentlength64 > 0) continue;
			}
			inbuf += i;
			if ((bufsize - inbuf) < LINESIZE){
				if (bufsize > 20000){
					RETURN(516);
				}
				if (!(newbuf = (char *)myrealloc(buf, bufsize + CACHE_BUFSIZE))){ RETURN(21); }
				buf = newbuf;
				bufsize += CACHE_BUFSIZE;
			}
		} while ((i = sockgetlinebuf(param, CLIENT, buf + inbuf, LINESIZE - 2, '\n', conf.timeouts[STRING_S])) > 2);


		buf[inbuf] = 0;

		reqsize = (int)strlen(req);
		reqbufsize = reqsize + 1;

#ifndef WITHMAIN

		action = handlereqfilters(param, &req, &reqbufsize, 0, &reqsize);
		if (action == HANDLED){
			RETURN(0);
		}
		if (action != PASS) RETURN(517);
		action = handlehdrfilterscli(param, &buf, &bufsize, 0, &inbuf);
		if (action == HANDLED){
			RETURN(0);
		}
		if (action != PASS) RETURN(517);
		if (param->ndatfilterscli > 0 && contentlength64 > 0){
			uint64_t newlen64;
			newlen64 = sockfillbuffcli(param, (unsigned long)contentlength64, CONNECTION_S);
			if (newlen64 == contentlength64) {
				action = handledatfltcli(param, &param->clibuf, &param->clibufsize, 0, &param->cliinbuf);
				if (action == HANDLED){
					RETURN(0);
				}
				if (action != PASS) RETURN(517);
				contentlength64 = param->cliinbuf;
				param->ndatfilterscli = 0;
			}
			sprintf((char*)buf + strlen((char *)buf), "Content-Length: %"PRINTF_INT64_MODIFIER"u\r\n", contentlength64);
		}

#endif

		if ((res = (*param->srv->authfunc)(param))) { RETURN(res); }
		break;
	}

	return 0;
}


#ifdef WITHMAIN
struct proxydef childdef = {
	NULL,
	3128,
	0,
	S_PROXY,
	"-a - anonymous proxy\r\n"
	"-a1 - anonymous proxy with random client IP spoofing\r\n"
};
#include "proxymain.c"
#endif
