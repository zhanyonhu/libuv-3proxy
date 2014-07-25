/*
   3APA3A simpliest proxy server
   (c) 2002-2008 by ZARAZA <3APA3A@security.nnov.ru>

   please read License Agreement

   $Id: proxy.c,v 1.107 2014-04-07 20:35:07 vlad Exp $
*/


#include "proxy.h"

#define RETURN(xxx) { param->res = xxx; goto CLEANRET; }

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

static void logerr(struct clientparam * param, char * format, ...)
{
	fprintf(stderr, "%p>>", param);
	fprintf(stderr, format);
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
	struct clientparam * ptask = (struct clientparam *)handle->data;
	buf->len = sizeof(ptask->local_buf);
	buf->base = ptask->local_buf;
	memset(buf->base, 0, buf->len);
	ASSERT(buf->base != NULL);
}

static void remote_close_cb(uv_handle_t* handle)
{
	struct clientparam * pclient = (struct clientparam *)handle->data;
}

void do_close(struct clientparam * pclient)
{
	uv_close((uv_handle_t*)&pclient->local_conn, NULL);
	uv_close((uv_handle_t*)&pclient->remote_conn, remote_close_cb);
}

static void local_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
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
		LOG_ERR(pclient, "read_cb error: %s\n", uv_err_name((int)nread));
		ASSERT(nread == UV_ECONNRESET || nread == UV_EOF);

		do_close(pclient);
		return;
	}

}

int do_local_read(struct clientparam * pclient)
{
	int r = 0;
	r = uv_read_start((uv_stream_t*)&pclient->local_conn, local_alloc_cb, local_read_cb);
	ASSERT(r == 0);
	return r;
}

void procy_cb(uv_work_t* req)
{
	int r = 0;
	struct clientparam * pclient = (struct clientparam *)req->data;
	r = uv_tcp_init(pclient->srv->loop, &pclient->local_conn);

	pclient->local_conn.socket = pclient->local_conn.socket;
// 	pclient->local_conn.
// #ifndef NOIPV6
// 	struct sockaddr_in6	sincl, sincr;
// #else
// 	struct sockaddr_in	sincl, sincr;
// #endif

	ASSERT(r == 0);
	do_local_read(pclient);
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
