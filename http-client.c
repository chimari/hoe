//    HDS OPE file Editor
//      http-client.c : Access to HTTP
//                                           2012.10.22  A.Tajitsu
/* $Id: ftp-client.c,v 1.4 2004/05/29 05:36:31 68user Exp $ */

#include "main.h"
#include <sys/param.h>

#ifdef USE_WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/uio.h>
#endif


#include<fcntl.h>
#include "ssl.h"

#define BUF_LEN 4096             /* バッファのサイズ */


////////////////////// Global Args //////////////////////
extern gboolean flagChildDialog;
extern gboolean flagSkymon;
extern gboolean flagPlot;
extern gboolean flagFC;
extern gboolean flagPlan;
extern gboolean flagPAM;
extern gboolean flagService;
extern gboolean flag_getFCDB;
extern gboolean flag_getDSS;
extern gboolean flag_make_obj_tree;
extern gboolean flag_make_line_tree;
extern gboolean flag_make_etc_tree;
extern gboolean flag_nodraw;

extern int debug_flg;

#ifndef USE_WIN32
extern pid_t fc_pid;
#endif
extern pid_t fcdb_pid;
extern pid_t stddb_pid;


void check_msg_from_parent();
gchar *make_rand16();
static gint fd_check_io();
gint fd_recv();
gint fd_gets();
char *read_line();
void read_response();
gint fd_write();
void write_to_server();
void write_to_SSLserver();
void error();
void PortReq();

int http_c_fcdb_new();
int http_c_fc_new();
int http_c_std_new();

int curl_http_c_fcdb_ssl();

void unchunk();


gint ssl_gets();
gint ssl_read();
gint ssl_peek();
gint ssl_write();

int post_body_new();

int Connect();

//int month_from_string_short();

void HTTP_data_read();
void SSL_data_read();

void HTTP_data_read(int command_socket, FILE *fp_write,
		    gboolean http_nonblock){
  char buf[BUF_LEN];
  int size;
  int result, mode;
  
#ifdef USE_WIN32
  if(http_nonblock){
    mode = 1;
    result= ioctlsocket(command_socket, FIONBIO, &mode);
  }
#endif    
  do{ // data read
    memset(buf, 0, sizeof(buf));
    if(debug_flg){
      fprintf(stderr,".");
    }
#ifdef USE_WIN32
    size = recv(command_socket,buf,BUF_LEN-1,0);
#else
    size = recv(command_socket,buf,BUF_LEN-1,
		(http_nonblock)?(MSG_DONTWAIT):0);
#endif    
    if(size > 0){
      fwrite( &buf , size , 1 , fp_write );
    }
    else if (size < 0){
      switch(errno){
      case EINTR:
	break;
      case EAGAIN:
	if(debug_flg) perror("Timeout");
	usleep(1e5);
	size=1;
	break;
      default:
	if(debug_flg) perror("recv");
	break;
      }
    }
    if(debug_flg){
      fprintf(stderr,"%co",0x08);
    }
  }while(size>0);
  if(debug_flg){
    fprintf(stderr,"\n");
  }
}

void SSL_data_read(int command_socket, SSL *ssl, SSL_CTX *ctx,
		   FILE *fp_write, gboolean http_nonblock){
  char buf[BUF_LEN];
  int err, size, result,  mode;
  
    
  SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
  
  if(http_nonblock){
#ifdef USE_WIN32  
    mode = 1;
    result= ioctlsocket(command_socket, FIONBIO, &mode);
#else    
    result = fcntl(command_socket, F_SETFL, O_NONBLOCK);
#endif  
  }
  do{ // data read
    memset(buf, 0, sizeof(buf));
    if(debug_flg){
      fprintf(stderr,".");
    }
    size = SSL_read(ssl, buf, BUF_LEN-1);
    if(size > 0){
      fwrite( &buf , size , 1 , fp_write );
    }
    else if (size < 0){
      err = SSL_get_error(ssl, size);
      
      switch(err){
      case SSL_ERROR_ZERO_RETURN:
	// eof
	break;
      case SSL_ERROR_WANT_READ:
	if(debug_flg) perror("Timeout");
	usleep(1e5);
	size=1;
	break;
      }
    }
    if(debug_flg){
      fprintf(stderr,"%co",0x08);
    }
  }while(size>0);
  if(debug_flg){
    fprintf(stderr,"\n");
  }
}


void check_msg_from_parent(typHOE *hg){
  if(hg->pabort){
    //g_main_loop_quit(hg->ploop);
    g_thread_exit(NULL);
  }
}

gchar *make_rand16(){
  int i;
  gchar retc[17] ,*ret;
  gchar ch;

  srand ( time(NULL) );
  for ( i = 0 ; i < 16 ; i++ ) {
    ch = rand () % 62;
    if (ch>=52){
      retc[i]='0'+ch-52;
    }
    else if (ch>=26){
      retc[i]='A'+ch-26;
    }
    else{
      retc[i]='a'+ch;
    }
  }
  retc[i]=0x00;

  ret=g_strdup(retc);

  return(ret);
}

static gint fd_check_io(gint fd, GIOCondition cond)
{
	struct timeval timeout;
	fd_set fds;
	guint io_timeout=60;

	timeout.tv_sec  = io_timeout;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (cond == G_IO_IN) {
		select(fd + 1, &fds, NULL, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	} else {
		select(fd + 1, NULL, &fds, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	}

	if (FD_ISSET(fd, &fds)) {
		return 0;
	} else {
		g_warning("Socket IO timeout\n");
		return -1;
	}
}

gint fd_recv(gint fd, gchar *buf, gint len, gint flags)
{
  gint ret;
  
  if (fd_check_io(fd, G_IO_IN) < 0)
    return -1;

  ret = recv(fd, buf, len, flags);
#ifdef USE_WIN32
  if (ret == SOCKET_ERROR) {
    fprintf(stderr,"Recv() Error TimeOut...  %d\n",WSAGetLastError());
  }
#endif
  return ret;
}


gint fd_gets(gint fd, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = fd_recv(fd, bp, len, MSG_PEEK)) <= 0)
      return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = fd_recv(fd, bp, n, 0)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}


/*--------------------------------------------------
 * ソケットから1行読み込む
 */
char *read_line(int socket, char *p){
    char *org_p = p;

    while (1){
        if ( read(socket, p, 1) == 0 ) break;
        if ( *p == '\n' ) break;
        p++;
    }
    *(++p) = '\0';
    return org_p;
}


/*--------------------------------------------------
 * レスポンスを取得する。^\d\d\d- ならもう1行取得
 */
void read_response(int socket, char *p){
  int ret;
    do { 
      //read_line(socket, p);
    ret=fd_gets(socket,p,BUF_LEN);
        if ( debug_flg ){
	  fprintf(stderr, "<-- %s", p);fflush(stderr);
        }
    } while ( isdigit(p[0]) &&
	      isdigit(p[1]) && 
	      isdigit(p[2]) &&
	      p[3]=='-' );

}


gint fd_write(gint fd, const gchar *buf, gint len)
{
#ifdef USE_WIN32
  gint ret;
#endif
  if (fd_check_io(fd, G_IO_OUT) < 0)
    return -1;
  
#ifdef USE_WIN32
  ret = send(fd, buf, len, 0);
  if (ret == SOCKET_ERROR) {
    gint err;
    err = WSAGetLastError();
    switch (err) {
    case WSAEWOULDBLOCK:
      errno = EAGAIN;
      break;
    default:
      fprintf(stderr,"last error = %d\n", err);
      errno = 0;
      break;
    }
    if (err != WSAEWOULDBLOCK)
      g_warning("fd_write() failed with %d (errno = %d)\n",
		err, errno);
  }
  return ret;
#else
  return write(fd, buf, len);
#endif
}

/*--------------------------------------------------
 * 指定されたソケット socket に文字列 p を送信。
 * 文字列 p の終端は \0 で terminate されている
 * 必要がある
 */

void write_to_server(int socket, char *p){
    if ( debug_flg ){
        fprintf(stderr, "--> %s", p);fflush(stderr);
    }
    
    fd_write(socket, p, strlen(p));
}

void write_to_SSLserver(SSL *ssl, char *p){
  if ( debug_flg ){
    fprintf(stderr, "[SSL] <-- %s", p);fflush(stderr);
  }
  
  ssl_write(ssl, p, strlen(p));
}

void error( char *message ){
  fprintf(stderr, "%s\n", message);
    exit(1);
}

void PortReq(char *IPaddr , int *i1 , int *i2 , int *i3 , int *i4 , int *i5 , int *i6)
{
  int j ;
  char *ip ;
  IPaddr = IPaddr + 3 ;

  //printf("aaa %s \n",IPaddr);

  while( isdigit(*IPaddr) == 0 ) { IPaddr++ ; }

  ip = strtok(IPaddr,",");
  *i1 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i2 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i3 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i4 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i5 = atoi(ip) ;

  ip = strtok(NULL,",");

  j = 0 ;
  while ( isdigit(*(ip +j)) != 0 ) { j += 1 ; }
  ip[j] = '\0' ;
  *i6 = atoi(ip) ;
}




gpointer thread_get_dss(gpointer gdata){
  typHOE *hg=(typHOE *)gdata;
  //waitpid(fc_pid,0,WNOHANG);

  hg->psz=0;
  hg->pabort=FALSE;
  
  if(hg->proxy_flag){
    switch(hg->fc_mode){  // Now All FC images via HTTPS
    default:
      http_c_fc_new(hg, FALSE, TRUE);
      break;
    }
  }
  else{
    switch(hg->fc_mode){
    default:
      http_c_fc_new(hg, TRUE, FALSE);
      break;
    }
  }

  hg->fc_pid=1;
  if(hg->ploop) g_main_loop_quit(hg->ploop);

  return(NULL);
}


gpointer thread_get_stddb(gpointer gdata){ 
  typHOE *hg=(typHOE *)gdata;
  
  hg->psz=0;
  hg->pabort=FALSE;
  
  http_c_std_new(hg, FALSE, FALSE);

  if(hg->ploop) g_main_loop_quit(hg->ploop);

  return(NULL);
}

gpointer thread_get_fcdb(gpointer gdata){
  typHOE *hg=(typHOE *)gdata;

  hg->psz=0;
  hg->pabort=FALSE;

  switch(hg->fcdb_type){
  case FCDB_TYPE_GSC:
  case MAGDB_TYPE_GSC:
  case FCDB_TYPE_2MASS:
  case MAGDB_TYPE_2MASS:
  case MAGDB_TYPE_IRCS_GSC:
  case FCDB_TYPE_PS1:
  case MAGDB_TYPE_PS1:
  case MAGDB_TYPE_IRCS_PS1:
  case FCDB_TYPE_SMOKA:
  case TRDB_TYPE_SMOKA:
  case TRDB_TYPE_FCDB_SMOKA:
  case TRDB_TYPE_WWWDB_SMOKA:
  case FCDB_TYPE_HST:
  case FCDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_HST:
    //case TRDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_FCDB_HST:
  case FCDB_TYPE_KEPLER:
  case MAGDB_TYPE_KEPLER:
  case FCDB_TYPE_GEMINI:
  case TRDB_TYPE_GEMINI:
  case TRDB_TYPE_FCDB_GEMINI:
  case ADDOBJ_TYPE_TRANSIENT:
  case FCDB_TYPE_SDSS:
  case MAGDB_TYPE_SDSS:
    if(hg->proxy_flag){
      http_c_fcdb_new(hg, FALSE, TRUE);
    }
    else{
      http_c_fcdb_new(hg, TRUE, TRUE);
    }
    break;
    
  case DBACCESS_VER:
  case DBACCESS_HSCFIL:
  case DBACCESS_HDSCAMZ:
  case DBACCESS_IRCSSET:
  case DBACCESS_IRDSET:
    if(hg->proxy_flag){
      http_c_fcdb_new(hg, FALSE, FALSE);
    }
    else{
      http_c_fcdb_new(hg, TRUE, FALSE);
    }
    break;
    
  case FCDB_TYPE_SIMBAD:
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
      if(hg->proxy_flag){
	http_c_fcdb_new(hg, FALSE, FALSE);
      }
      else{
	http_c_fcdb_new(hg, TRUE, FALSE);
      }
    }
    else{
      http_c_fcdb_new(hg, FALSE,  FALSE);
    }
    break;
    
  default:
    http_c_fcdb_new(hg, FALSE,  FALSE);
    break;
  }

  if(hg->ploop) g_main_loop_quit(hg->ploop);

  return(NULL);
}

void unchunk(gchar *dss_tmp){
  FILE *fp_read, *fp_write;
  gchar *unchunk_tmp;
  gchar cbuf[BUFFSIZE];
  gchar *dbuf=NULL;
  gchar *cpp;
  gchar *chunkptr, *endptr;
  long chunk_size;
  gint i, read_size=0, crlf_size=0;
  
  if ( debug_flg ){
    fprintf(stderr, "Decoding chunked file \"%s\".\n", dss_tmp);fflush(stderr);
  }
  
  fp_read=fopen(dss_tmp,"r");
  unchunk_tmp=g_strconcat(dss_tmp,"_unchunked",NULL);
  fp_write=fopen(unchunk_tmp,"wb");
  
  while(!feof(fp_read)){
    if(fgets(cbuf,BUFFSIZE-1,fp_read)){
      cpp=cbuf;
      
      read_size=strlen(cpp);
      for(i=read_size;i>=0;i--){
	if(isalnum(cpp[i])){
	  crlf_size=read_size-i-1;
	  break;
	}
	else{
	  cpp[i]='\0';
	}
      }
      chunkptr=g_strdup_printf("0x%s",cpp);
      chunk_size=strtol(chunkptr, &endptr, 0);
      g_free(chunkptr);
      
      if(chunk_size==0) break;
      
      if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(chunk_size+crlf_size+1)))==NULL){
	fprintf(stderr, "!!! Memory allocation error in unchunk() \"%s\".\n", dss_tmp);
	fflush(stderr);
	break;
      }
      if(fread(dbuf,1, chunk_size+crlf_size, fp_read)){
	fwrite( dbuf , chunk_size , 1 , fp_write ); 
	if(dbuf) g_free(dbuf);
      }
      else{
	break;
      }
    }
  }
  
  fclose(fp_read);
  fclose(fp_write);
  
  unlink(dss_tmp);
  
  //rename(unchunk_tmp,dss_tmp);
  copy_file(unchunk_tmp,dss_tmp);
  unlink(unchunk_tmp);
  
  g_free(unchunk_tmp);
}

 gint ssl_gets(SSL *ssl, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  gint i;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = ssl_peek(ssl, bp, len)) <= 0)
	return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = ssl_read(ssl, bp, n)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}

 gint ssl_read(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_read(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	default:
		g_warning("SSL_read() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}

/* peek at the socket data without actually reading it */
gint ssl_peek(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_peek(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	case SSL_ERROR_SYSCALL:
	  // End of file
	  //printf("SSL_ERROR_SYSCALL ret=%d  %d\n",ret,(gint)strlen(buf));
	        return 0;
	default:
		g_warning("SSL_peek() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}

gint ssl_write(SSL *ssl, const gchar *buf, gint len)
{
	gint ret;

	ret = SSL_write(ssl, buf, len);

	switch (SSL_get_error(ssl, ret)) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	default:
		return -1;
	}
}


int http_c_std_new(typHOE *hg, gboolean SSL_flag, gboolean proxy_ssl){
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct sockaddr_in *addr_in;
  struct in_addr addr;
  int err, ret;

  gboolean chunked_flag=FALSE;
  gchar *cp;
   
  SSL *ssl;
  SSL_CTX *ctx;

  check_msg_from_parent(hg);

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo((hg->proxy_flag) ? hg->proxy_host :hg->std_host,
			 (SSL_flag) ? "https" : "http",
			 &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n",
	    (hg->proxy_flag) ? hg->proxy_host :hg->std_host);
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent(hg);

  if(hg->proxy_flag){
    addr_in = (struct sockaddr_in *)(res -> ai_addr);
    addr_in -> sin_port=htons(hg->proxy_port);
  }

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent(hg);

  /* サーバに接続 */
  if( Connect(command_socket, res->ai_addr, res->ai_addrlen, hg->http_timeout) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->std_host);
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }
  
  check_msg_from_parent(hg);

  if(SSL_flag){  // HTTPS
    SSL_load_error_strings();
    SSL_library_init();
    
    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, command_socket);

    SSL_set_tlsext_host_name(ssl, hg->std_host);
    
    while((ret=SSL_connect(ssl))!=1){
      err=SSL_get_error(ssl, ret);
      if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
	g_usleep(100000);
	g_warning("SSL_connect(): try again\n");
	continue;
      }
      g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
		err, ret, ERR_error_string(ERR_get_error(), NULL));
      return(HSKYMON_HTTP_ERROR_SSL);
    }
    
    check_msg_from_parent(hg);
  }

  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  if(SSL_flag){  // HTTPS
    sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->std_path);
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "Accept: application/xml\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "Host: %s\r\n", hg->std_host);
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "Connection: close\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "\r\n");
    write_to_SSLserver(ssl, send_mesg);
  }
  else{ // HTTP
    if(hg->proxy_flag){
      if(proxy_ssl){
	sprintf(send_mesg, "GET https://%s%s HTTP/1.1\r\n",
		hg->std_host,hg->std_path);
      }
      else{ 
	sprintf(send_mesg, "GET http://%s%s HTTP/1.1\r\n",
		hg->std_host,hg->std_path);
      }
    }
    else{ 
      sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->std_path);
      write_to_server(command_socket, send_mesg);
    }

    sprintf(send_mesg, "Accept: application/xml\r\n");
    write_to_server(command_socket, send_mesg);

    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_server(command_socket, send_mesg);

    sprintf(send_mesg, "Host: %s\r\n", hg->std_host);
    write_to_server(command_socket, send_mesg);

    sprintf(send_mesg, "Connection: close\r\n");
    write_to_server(command_socket, send_mesg);

    sprintf(send_mesg, "\r\n");
    write_to_server(command_socket, send_mesg);
  }
  
  if((fp_write=fopen(hg->std_file,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->std_file);
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }

  if(SSL_flag){
    while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"[SSL] --> Header: %s", buf);
      }
      if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
      if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	cp = buf + strlen("Content-Length: ");
	hg->psz=atol(cp);
      }
    }
    // data read
    SSL_data_read(command_socket, ssl, ctx, fp_write, hg->http_nonblock);
  }
  else{  // HTTP
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
      if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	cp = buf + strlen("Content-Length: ");
	hg->psz=atol(cp);
      }
    }
    // data read
    HTTP_data_read(command_socket, fp_write, hg->http_nonblock);
  }
    
  fclose(fp_write);

  check_msg_from_parent(hg);

  if(chunked_flag) unchunk(hg->std_file);

  if((chmod(hg->std_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->std_file);
  }
  
  if(SSL_flag){ // HTTPS
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
  }

  close(command_socket);

  return 0;
}

int post_body(typHOE *hg, gboolean wflag, int command_socket, 
	      gchar *rand16){
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char ins_mesg[BUF_LEN];
  gint ip, plen, i;
  gchar *send_buf1=NULL, *send_buf2=NULL;
  gboolean init_flag=FALSE;
  gboolean send_flag=TRUE;
  gchar *tmp;
  gint i_obj, grism, exp, repeat, filter, gain, frames;
  gboolean sh, pc, ag, nw, queue;
  gint pa;
  gdouble dexp;


  switch(hg->fcdb_type){
  case MAGDB_TYPE_SEIMEI_KOOLS:
  case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
    ip=0;
    plen=0;

    switch(hg->fcdb_type){
    case MAGDB_TYPE_SEIMEI_KOOLS:
      i_obj=hg->sh_i;
      exp=hg->obj[hg->sh_i].exp;
      repeat=hg->obj[hg->sh_i].repeat;
      
      grism=hg->obj[hg->sh_i].kools.grism;
      sh=hg->obj[hg->sh_i].kools.sh;
      pc=hg->obj[hg->sh_i].kools.pc;
      ag=hg->obj[hg->sh_i].kools.ag;
      nw=hg->obj[hg->sh_i].kools.nw;
      pa=hg->obj[hg->sh_i].pa;
      queue=hg->def_kools_queue;
      break;
      
    case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
      i_obj=hg->plan[hg->sh_i_plan].obj_i;
      exp=hg->plan[hg->sh_i_plan].exp;
      repeat=hg->plan[hg->sh_i_plan].repeat;
      
      grism=hg->plan[hg->sh_i_plan].setup;
      sh=hg->plan[hg->sh_i_plan].sh;
      pc=hg->plan[hg->sh_i_plan].pc;
      ag=hg->plan[hg->sh_i_plan].ag;
      nw=hg->plan[hg->sh_i_plan].nw;
      pa=hg->plan[hg->sh_i_plan].pa;
      queue=hg->plan_queue;
      break;
    }
    
    while(1){
      if(seimei_kools_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(seimei_kools_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		seimei_kools_post[ip].key);
	break;

      case POST_CONST:
	if(strcmp(seimei_kools_post[ip].key,"submit")==0){
	  sprintf(send_mesg,
		  "%s=%s",
		  seimei_kools_post[ip].key,
		  seimei_kools_post[ip].prm);
	}
	else{
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  seimei_kools_post[ip].prm);
	}
	break;
	
      case POST_INPUT:
	if(strcmp(seimei_kools_post[ip].key,"propid")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  (hg->prop_id) ? hg->prop_id : "Prop-ID");
	}
	else if(strcmp(seimei_kools_post[ip].key,"observer")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  (hg->observer) ? hg->observer : "Observer");
	}
	else if(strcmp(seimei_kools_post[ip].key,"object1")==0){
	  tmp=make_seimei_line(hg->obj[i_obj], FALSE);
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  tmp);
	  g_free(tmp);
	}
	else if(strcmp(seimei_kools_post[ip].key,"grism1")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  kools_grism_name[grism]);
	}
	else if(strcmp(seimei_kools_post[ip].key,"exptime1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  exp);
	}
	else if(strcmp(seimei_kools_post[ip].key,"nexp1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  repeat);
	}
	else if(strcmp(seimei_kools_post[ip].key,"sh1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (sh) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"ptc1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (pc) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"ag1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (ag) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"nw1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (nw) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"pa1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  pa);
	}
	else if(strcmp(seimei_kools_post[ip].key,"queue")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  (queue) ? "Y" : "N");
	}

	break;
      }


      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }


    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      write_to_server(command_socket, send_buf1);
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    // Seimei KOOLS

  case MAGDB_TYPE_SEIMEI_TRICCS:
  case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
    ip=0;
    plen=0;

    switch(hg->fcdb_type){
    case MAGDB_TYPE_SEIMEI_TRICCS:
      i_obj=hg->sh_i;
      dexp=hg->obj[hg->sh_i].dexp;
      repeat=hg->obj[hg->sh_i].repeat;
      
      filter=hg->obj[hg->sh_i].triccs.filter;
      gain=hg->obj[hg->sh_i].triccs.gain;
      frames=hg->obj[hg->sh_i].triccs.frames;
      sh=hg->obj[hg->sh_i].kools.sh;
      pc=hg->obj[hg->sh_i].kools.pc;
      ag=hg->obj[hg->sh_i].kools.ag;
      pa=hg->obj[hg->sh_i].pa;
      queue=hg->def_kools_queue;
      break;
      
    case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
      i_obj=hg->plan[hg->sh_i_plan].obj_i;
      dexp=hg->plan[hg->sh_i_plan].dexp;
      repeat=hg->plan[hg->sh_i_plan].repeat;
      
      filter=hg->plan[hg->sh_i_plan].setup;
      gain=hg->plan[hg->sh_i_plan].gain;
      frames=hg->plan[hg->sh_i_plan].frames;
      sh=hg->plan[hg->sh_i_plan].sh;
      pc=hg->plan[hg->sh_i_plan].pc;
      ag=hg->plan[hg->sh_i_plan].ag;
      pa=hg->plan[hg->sh_i_plan].pa;
      queue=hg->plan_queue;
      break;
    }
    
    while(1){
      if(seimei_triccs_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(seimei_triccs_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		seimei_triccs_post[ip].key);
	break;

      case POST_CONST:
	if(strcmp(seimei_triccs_post[ip].key,"submit")==0){
	  sprintf(send_mesg,
		  "%s=%s",
		  seimei_triccs_post[ip].key,
		  seimei_triccs_post[ip].prm);
	}else{
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  seimei_triccs_post[ip].prm);
	}
	break;
	
      case POST_INPUT:
	if(strcmp(seimei_triccs_post[ip].key,"propid")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  (hg->prop_id) ? hg->prop_id : "Prop-ID");
	}
	else if(strcmp(seimei_triccs_post[ip].key,"observer")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  (hg->observer) ? hg->observer : "Observer");
	}
	else if(strcmp(seimei_triccs_post[ip].key,"object1")==0){
	  tmp=make_seimei_line(hg->obj[i_obj], FALSE);
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  tmp);
	  g_free(tmp);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"filter1")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  triccs_filter_prm[filter]);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"gain1")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  triccs_gain_name[gain]);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"exptime1")==0){
	  sprintf(send_mesg,
		  "%s=%.3lf&",
		  seimei_triccs_post[ip].key,
		  dexp);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"nframe1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  frames);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"nexp1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  repeat);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"sh1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  (sh) ? 1 : 0);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"ptc1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  (pc) ? 1 : 0);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"ag1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  (ag) ? 1 : 0);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"pa1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  pa);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"queue")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  (queue) ? "Y" : "N");
	}

	break;
      }



      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }


    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      write_to_server(command_socket, send_buf1);
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    // Seimei TriCCS

  case FCDB_TYPE_LAMOST:
    ip=0;
    plen=0;

    switch(hg->fcdb_lamost_dr){
    case FCDB_LAMOST_DR7M:
    case FCDB_LAMOST_DR8M:
      while(1){
	if(lamost_med_post[ip].key==NULL) break;
	switch(lamost_med_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_med_post[ip].key);
	  break;

	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_med_post[ip].key,
		  lamost_med_post[ip].prm);
	  break;
	  
	case POST_INPUT:
	  if(strcmp(lamost_med_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->dss_arcmin*30.0);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  write_to_server(command_socket, send_mesg);
	}
	ip++;
      }
      break;

    default:
      while(1){
	if(lamost_post[ip].key==NULL) break;
	switch(lamost_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_post[ip].key);
	  break;

	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_post[ip].key,
		  lamost_post[ip].prm);
	  break;

	  
	case POST_INPUT:
	  if(strcmp(lamost_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->dss_arcmin*30.0);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  write_to_server(command_socket, send_mesg);
	}
	ip++;
      }
      break;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      write_to_server(command_socket, send_mesg);
    }

    break;

  case MAGDB_TYPE_LAMOST:
    ip=0;
    plen=0;

    switch(hg->fcdb_lamost_dr){
    case FCDB_LAMOST_DR7M:
    case FCDB_LAMOST_DR8M:
      while(1){
	if(lamost_med_post[ip].key==NULL) break;
	switch(lamost_med_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_med_post[ip].key);
	  break;
	  
	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_med_post[ip].key,
		  lamost_med_post[ip].prm);
	  break;
	  
	case POST_INPUT:
	  if(strcmp(lamost_med_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    (gdouble)hg->magdb_arcsec);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  write_to_server(command_socket, send_mesg);
	}
	ip++;
      }
      break;

    default:
      while(1){
	if(lamost_post[ip].key==NULL) break;
	switch(lamost_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_post[ip].key);
	  break;
	  
	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_post[ip].key,
		  lamost_post[ip].prm);
	  break;
	  
	case POST_INPUT:
	  if(strcmp(lamost_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    (gdouble)hg->magdb_arcsec);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  write_to_server(command_socket, send_mesg);
	}
	ip++;
      }
      break;
    }
      
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      write_to_server(command_socket, send_mesg);
    }

    break;


  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_ESO:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_ESO:
    ip=0;
    plen=0;

    while(1){
      if(eso_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(eso_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		eso_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		eso_post[ip].key,
		eso_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(eso_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  eso_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(eso_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  eso_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(eso_post[ip].key,"box")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d %02d\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->dss_arcmin/2,
		    hg->dss_arcmin*30-(hg->dss_arcmin/2)*60);
	    break;

	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 04 00\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d 00\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_arcmin);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d 00\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_arcmin_used);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"wdbo")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case TRDB_TYPE_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\nvotable/display\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case FCDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\nhtml/display\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"stime")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_stdate);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_stdate_used);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"etime")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_eddate);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_eddate_used);
	    break;
	  }
	}
	break;

      case POST_INST1:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_IMAGE;i++){
	    if(hg->fcdb_eso_image[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_image[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_IMAGE){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_image[hg->trdb_eso_image].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_IMAGE){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_image[hg->trdb_eso_image_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST2:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_SPEC;i++){
	    if(hg->fcdb_eso_spec[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_spec[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;
	  
	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SPEC){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_spec[hg->trdb_eso_spec].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_SPEC){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_spec[hg->trdb_eso_spec_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST3:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_VLTI;i++){
	    if(hg->fcdb_eso_vlti[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_vlti[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_VLTI){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_vlti[hg->trdb_eso_vlti].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_VLTI){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_vlti[hg->trdb_eso_vlti_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST4:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_POLA;i++){
	    if(hg->fcdb_eso_pola[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_pola[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_POLA){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_pola[hg->trdb_eso_pola].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_POLA){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_pola[hg->trdb_eso_pola_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST5:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_CORO;i++){
	    if(hg->fcdb_eso_coro[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_coro[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_CORO){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_coro[hg->trdb_eso_coro].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_CORO){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_coro[hg->trdb_eso_coro_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST6:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_OTHER;i++){
	    if(hg->fcdb_eso_other[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_other[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_OTHER){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_other[hg->trdb_eso_other].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	  
	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_OTHER){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_other[hg->trdb_eso_other_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST7:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_SAM;i++){
	    if(hg->fcdb_eso_sam[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_sam[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SAM){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_sam[hg->trdb_eso_sam].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_SAM){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_sam[hg->trdb_eso_sam_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_ADD:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n",
		rand16,
		eso_post[ip].key);
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  for(i=0;i<NUM_ESO_IMAGE;i++){
	    if(hg->fcdb_eso_image[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_image[i].add);
	    }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_image[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_SPEC;i++){
	    if(hg->fcdb_eso_spec[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_spec[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_spec[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_VLTI;i++){
	    if(hg->fcdb_eso_vlti[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_vlti[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_vlti[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_POLA;i++){
	    if(hg->fcdb_eso_pola[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_pola[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_pola[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_CORO;i++){
	    if(hg->fcdb_eso_coro[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_coro[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_coro[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_OTHER;i++){
	    if(hg->fcdb_eso_other[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_other[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_other[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_SAM;i++){
	    if(hg->fcdb_eso_sam[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_sam[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_sam[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  if(init_flag){
	    strcat(send_mesg,")\r\n");
	  }
	  else{
	    strcat(send_mesg,"\r\n");
	  }
	  break;

	case TRDB_TYPE_ESO:
	  strcat(send_mesg, "(");
	  switch(hg->trdb_eso_mode){
	  case TRDB_ESO_MODE_IMAGE:
	    strcat(send_mesg, eso_image[hg->trdb_eso_image].add);
	    break;
	  case TRDB_ESO_MODE_SPEC:
	    strcat(send_mesg, eso_spec[hg->trdb_eso_spec].add);
	    break;
	  case TRDB_ESO_MODE_VLTI:
	    strcat(send_mesg, eso_vlti[hg->trdb_eso_vlti].add);
	    break;
	  case TRDB_ESO_MODE_POLA:
	    strcat(send_mesg, eso_pola[hg->trdb_eso_pola].add);
	    break;
	  case TRDB_ESO_MODE_CORO:
	    strcat(send_mesg, eso_coro[hg->trdb_eso_coro].add);
	    break;
	  case TRDB_ESO_MODE_OTHER:
	    strcat(send_mesg, eso_other[hg->trdb_eso_other].add);
	    break;
	  case TRDB_ESO_MODE_SAM:
	    strcat(send_mesg, eso_sam[hg->trdb_eso_sam].add);
	    break;
	  }
	  strcat(send_mesg,")\r\n");
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  strcat(send_mesg, "(");
	  switch(hg->trdb_eso_mode_used){
	  case TRDB_ESO_MODE_IMAGE:
	    strcat(send_mesg, eso_image[hg->trdb_eso_image_used].add);
	    break;
	  case TRDB_ESO_MODE_SPEC:
	    strcat(send_mesg, eso_spec[hg->trdb_eso_spec_used].add);
	    break;
	  case TRDB_ESO_MODE_VLTI:
	    strcat(send_mesg, eso_vlti[hg->trdb_eso_vlti_used].add);
	    break;
	  case TRDB_ESO_MODE_POLA:
	    strcat(send_mesg, eso_pola[hg->trdb_eso_pola_used].add);
	    break;
	  case TRDB_ESO_MODE_CORO:
	    strcat(send_mesg, eso_coro[hg->trdb_eso_coro_used].add);
	    break;
	  case TRDB_ESO_MODE_OTHER:
	    strcat(send_mesg, eso_other[hg->trdb_eso_other_used].add);
	    break;
	  case TRDB_ESO_MODE_SAM:
	    strcat(send_mesg, eso_sam[hg->trdb_eso_sam_used].add);
	    break;
	  }
	  strcat(send_mesg,")\r\n");
	  break;
	}
	break;
      }
	
      if(send_flag){
	plen+=strlen(send_mesg);
	if(wflag){
	  write_to_server(command_socket, send_mesg);
	}
      }
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      write_to_server(command_socket, send_mesg);
    }

    break;
  }

  return(plen);
}


int http_c_fcdb(typHOE *hg){
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  // Calculate Content-Length
  if(hg->fcdb_post){
    rand16=make_rand16();
    plen=post_body_new(hg, FALSE, 0, NULL, rand16, FALSE);
  }
   
  check_msg_from_parent(hg);
   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->fcdb_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->fcdb_host);
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent(hg);
   
  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent(hg);
   
  /* サーバに接続 */
  if( Connect(command_socket, res->ai_addr, res->ai_addrlen, hg->http_timeout) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->fcdb_host);
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }
  
  check_msg_from_parent(hg);
   
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  if(hg->fcdb_post){
    sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hg->fcdb_path);
  }
  else{
    sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->fcdb_path);
  }
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept: application/xml\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hg->fcdb_host);
  write_to_server(command_socket, send_mesg);

  /*
  switch(hg->fcdb_type){
  case MAGDB_TYPE_SEIMEI_KOOLS:
  case MAGDB_TYPE_SEIMEI_TRICCS:
    sprintf(send_mesg, "Keep-Alive: timeout=5, max=100\r\n");
    write_to_server(command_socket, send_mesg);
    sprintf(send_mesg, "Connection: Keep-Alive\r\n");
    break;
      
  default:
    sprintf(send_mesg, "Connection: close\r\n");
    break;
    }*/
  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);

  if(hg->fcdb_post){
    sprintf(send_mesg, "Content-Length: %d\r\n", plen);
    write_to_server(command_socket, send_mesg);

    switch(hg->fcdb_type){
    case FCDB_TYPE_SDSS:
    case MAGDB_TYPE_SDSS:
    case FCDB_TYPE_LAMOST:
    case MAGDB_TYPE_LAMOST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_WWWDB_ESO:
    case TRDB_TYPE_ESO:
    case TRDB_TYPE_WWWDB_ESO:
    case TRDB_TYPE_FCDB_ESO:
      sprintf(send_mesg, "Content-Type:multipart/form-data; boundary=----WebKitFormBoundary%s\r\n", rand16);
      break;
    case MAGDB_TYPE_SEIMEI_KOOLS:
    case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
    case MAGDB_TYPE_SEIMEI_TRICCS:
    case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
      sprintf(send_mesg, "Content-Type: application/x-www-form-urlencoded\r\n");
      break;
    }
    write_to_server(command_socket, send_mesg);
  }

  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);

  // POST body
  if(hg->fcdb_post){
    plen=post_body(hg, TRUE, command_socket, rand16);
    if(rand16) g_free(rand16);
  }

  if((fp_write=fopen(hg->fcdb_file,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->fcdb_file);
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }

  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"--> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
    }
    if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
      cp = buf + strlen("Content-Length: ");
      hg->psz=atol(cp);
    }
  }
  // data read
  HTTP_data_read(command_socket, fp_write, hg->http_nonblock);
      
  fclose(fp_write);

  check_msg_from_parent(hg);

  if(chunked_flag) unchunk(hg->fcdb_file);
  // This is a bug fix for SDSS DR16 VOTable output
  if((hg->fcdb_type==FCDB_TYPE_SDSS)||(hg->fcdb_type==MAGDB_TYPE_SDSS)){ 
    str_replace(hg->fcdb_file, 
		"encoding=\"utf-16\"",
		" encoding=\"utf-8\"");
  }    
  

    if((chmod(hg->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
      g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->fcdb_file);
    }
  
  close(command_socket);

  return 0;
}



int month_from_string_short(const char *a_month)
{
  gint i;

  for(i=0;i<12;i++){
    if (strncmp(a_month, cal_month[i], 3) == 0)
      return i;
  }
  /* not a valid date */
  return -1;
}



int post_body_new(typHOE *hg,
		  gboolean wflag,
		  int command_socket,
		  SSL *ssl, 
		  gchar *rand16,
		  gboolean SSL_flag){
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char ins_mesg[BUF_LEN];
  gint ip, plen, i, i_inst;
  gint i_obj, grism, exp, repeat, filter, gain, frames;
  gboolean sh, pc, ag, nw, queue;
  gchar *send_buf1=NULL, *send_buf2=NULL;
  gchar* sci_instrume=NULL, *tmp_inst=NULL;
  gboolean init_flag=FALSE;
  gboolean send_flag=TRUE;
  gchar *tmp;
  gint pa;
  gdouble dexp;

  switch(hg->fcdb_type){
  case MAGDB_TYPE_SEIMEI_KOOLS:
  case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
    ip=0;
    plen=0;

    switch(hg->fcdb_type){
    case MAGDB_TYPE_SEIMEI_KOOLS:
      i_obj=hg->sh_i;
      exp=hg->obj[hg->sh_i].exp;
      repeat=hg->obj[hg->sh_i].repeat;
      
      grism=hg->obj[hg->sh_i].kools.grism;
      sh=hg->obj[hg->sh_i].kools.sh;
      pc=hg->obj[hg->sh_i].kools.pc;
      ag=hg->obj[hg->sh_i].kools.ag;
      nw=hg->obj[hg->sh_i].kools.nw;
      pa=hg->obj[hg->sh_i].pa;
      queue=hg->def_kools_queue;
      break;
      
    case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
      i_obj=hg->plan[hg->sh_i_plan].obj_i;
      exp=hg->plan[hg->sh_i_plan].exp;
      repeat=hg->plan[hg->sh_i_plan].repeat;
      
      grism=hg->plan[hg->sh_i_plan].setup;
      sh=hg->plan[hg->sh_i_plan].sh;
      pc=hg->plan[hg->sh_i_plan].pc;
      ag=hg->plan[hg->sh_i_plan].ag;
      nw=hg->plan[hg->sh_i_plan].nw;
      pa=hg->plan[hg->sh_i_plan].pa;
      queue=hg->plan_queue;
      break;
    }
    
    while(1){
      if(seimei_kools_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(seimei_kools_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		seimei_kools_post[ip].key);
	break;

      case POST_CONST:
	if(strcmp(seimei_kools_post[ip].key,"submit")==0){
	  sprintf(send_mesg,
		  "%s=%s",
		  seimei_kools_post[ip].key,
		  seimei_kools_post[ip].prm);
	}
	else{
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  seimei_kools_post[ip].prm);
	}
	break;
	
      case POST_INPUT:
	if(strcmp(seimei_kools_post[ip].key,"propid")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  (hg->prop_id) ? hg->prop_id : "Prop-ID");
	}
	else if(strcmp(seimei_kools_post[ip].key,"observer")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  (hg->observer) ? hg->observer : "Observer");
	}
	else if(strcmp(seimei_kools_post[ip].key,"object1")==0){
	  tmp=make_seimei_line(hg->obj[i_obj], FALSE);
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  tmp);
	  g_free(tmp);
	}
	else if(strcmp(seimei_kools_post[ip].key,"grism1")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  kools_grism_name[grism]);
	}
	else if(strcmp(seimei_kools_post[ip].key,"exptime1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  exp);
	}
	else if(strcmp(seimei_kools_post[ip].key,"nexp1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  repeat);
	}
	else if(strcmp(seimei_kools_post[ip].key,"sh1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (sh) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"ptc1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (pc) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"ag1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (ag) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"nw1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  (nw) ? 1 : 0);
	}
	else if(strcmp(seimei_kools_post[ip].key,"pa1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_kools_post[ip].key,
		  pa);
	}
	else if(strcmp(seimei_kools_post[ip].key,"queue")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_kools_post[ip].key,
		  (queue) ? "Y" : "N");
	}

	break;
      }


      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }


    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    // Seimei KOOLS

  case MAGDB_TYPE_SEIMEI_TRICCS:
  case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
    ip=0;
    plen=0;

    switch(hg->fcdb_type){
    case MAGDB_TYPE_SEIMEI_TRICCS:
      i_obj=hg->sh_i;
      dexp=hg->obj[hg->sh_i].dexp;
      repeat=hg->obj[hg->sh_i].repeat;
      
      filter=hg->obj[hg->sh_i].triccs.filter;
      gain=hg->obj[hg->sh_i].triccs.gain;
      frames=hg->obj[hg->sh_i].triccs.frames;
      sh=hg->obj[hg->sh_i].kools.sh;
      pc=hg->obj[hg->sh_i].kools.pc;
      ag=hg->obj[hg->sh_i].kools.ag;
      pa=hg->obj[hg->sh_i].pa;
      queue=hg->def_kools_queue;
      break;
      
    case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
      i_obj=hg->plan[hg->sh_i_plan].obj_i;
      dexp=hg->plan[hg->sh_i_plan].dexp;
      repeat=hg->plan[hg->sh_i_plan].repeat;
      
      filter=hg->plan[hg->sh_i_plan].setup;
      gain=hg->plan[hg->sh_i_plan].gain;
      frames=hg->plan[hg->sh_i_plan].frames;
      sh=hg->plan[hg->sh_i_plan].sh;
      pc=hg->plan[hg->sh_i_plan].pc;
      ag=hg->plan[hg->sh_i_plan].ag;
      pa=hg->plan[hg->sh_i_plan].pa;
      queue=hg->plan_queue;
      break;
    }
    
    while(1){
      if(seimei_triccs_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(seimei_triccs_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		seimei_triccs_post[ip].key);
	break;

      case POST_CONST:
	if(strcmp(seimei_triccs_post[ip].key,"submit")==0){
	  sprintf(send_mesg,
		  "%s=%s",
		  seimei_triccs_post[ip].key,
		  seimei_triccs_post[ip].prm);
	}else{
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  seimei_triccs_post[ip].prm);
	}
	break;
	
      case POST_INPUT:
	if(strcmp(seimei_triccs_post[ip].key,"propid")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  (hg->prop_id) ? hg->prop_id : "Prop-ID");
	}
	else if(strcmp(seimei_triccs_post[ip].key,"observer")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  (hg->observer) ? hg->observer : "Observer");
	}
	else if(strcmp(seimei_triccs_post[ip].key,"object1")==0){
	  tmp=make_seimei_line(hg->obj[i_obj], FALSE);
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  tmp);
	  g_free(tmp);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"filter1")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  triccs_filter_prm[filter]);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"gain1")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  triccs_gain_name[gain]);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"exptime1")==0){
	  sprintf(send_mesg,
		  "%s=%.3lf&",
		  seimei_triccs_post[ip].key,
		  dexp);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"nframe1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  frames);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"nexp1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  repeat);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"sh1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  (sh) ? 1 : 0);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"ptc1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  (pc) ? 1 : 0);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"ag1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  (ag) ? 1 : 0);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"pa1")==0){
	  sprintf(send_mesg,
		  "%s=%d&",
		  seimei_triccs_post[ip].key,
		  pa);
	}
	else if(strcmp(seimei_triccs_post[ip].key,"queue")==0){
	  sprintf(send_mesg,
		  "%s=%s&",
		  seimei_triccs_post[ip].key,
		  (queue) ? "Y" : "N");
	}

	break;
      }



      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }


    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    // Seimei TriCCS

  case FCDB_TYPE_LAMOST:
    ip=0;
    plen=0;

    switch(hg->fcdb_lamost_dr){
    case FCDB_LAMOST_DR7M:
    case FCDB_LAMOST_DR8M:
      while(1){
	if(lamost_med_post[ip].key==NULL) break;
	switch(lamost_med_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_med_post[ip].key);
	  break;

	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_med_post[ip].key,
		  lamost_med_post[ip].prm);
	  break;
	  
	case POST_INPUT:
	  if(strcmp(lamost_med_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->dss_arcmin*30.0);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  if(SSL_flag){
	    write_to_SSLserver(ssl, send_mesg);
	  }
	  else{
	    write_to_server(command_socket, send_mesg);
	  }
	}
	ip++;
      }
      break;

    default:
      while(1){
	if(lamost_post[ip].key==NULL) break;
	switch(lamost_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_post[ip].key);
	  break;

	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_post[ip].key,
		  lamost_post[ip].prm);
	  break;

	  
	case POST_INPUT:
	  if(strcmp(lamost_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->dss_arcmin*30.0);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  if(SSL_flag){
	    write_to_SSLserver(ssl, send_mesg);
	  }
	  else{
	    write_to_server(command_socket, send_mesg);
	  }
	}
	ip++;
      }
      break;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_mesg);
      }
      else{
	write_to_server(command_socket, send_mesg);
      }
    }

    break;

  case MAGDB_TYPE_LAMOST:
    ip=0;
    plen=0;

    switch(hg->fcdb_lamost_dr){
    case FCDB_LAMOST_DR7M:
    case FCDB_LAMOST_DR8M:
      while(1){
	if(lamost_med_post[ip].key==NULL) break;
	switch(lamost_med_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_med_post[ip].key);
	  break;
	  
	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_med_post[ip].key,
		  lamost_med_post[ip].prm);
	  break;
	  
	case POST_INPUT:
	  if(strcmp(lamost_med_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_med_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_med_post[ip].key,
		    (gdouble)hg->magdb_arcsec);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  if(SSL_flag){
	    write_to_SSLserver(ssl, send_mesg);
	  }
	  else{
	    write_to_server(command_socket, send_mesg);
	  }
	}
	ip++;
      }
      break;

    default:
      while(1){
	if(lamost_post[ip].key==NULL) break;
	switch(lamost_post[ip].flg){
	case POST_NULL:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		  rand16,
		  lamost_post[ip].key);
	  break;
	  
	case POST_CONST:
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  lamost_post[ip].key,
		  lamost_post[ip].prm);
	  break;
	  
	case POST_INPUT:
	  if(strcmp(lamost_post[ip].key,"pos.racenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_ra0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.deccenter")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    hg->fcdb_d_dec0);
	  }
	  else if(strcmp(lamost_post[ip].key,"pos.radius")==0){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		    rand16,
		    lamost_post[ip].key,
		    (gdouble)hg->magdb_arcsec);
	  }
	  break;
	}	
	plen+=strlen(send_mesg);
	if(wflag){
	  if(SSL_flag){
	    write_to_SSLserver(ssl, send_mesg);
	  }
	  else{
	    write_to_server(command_socket, send_mesg);
	  }
	}
	ip++;
      }
      break;
    }
      
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_mesg);
      }
      else{
	write_to_server(command_socket, send_mesg);
      }
    }

    break;


  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_ESO:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_ESO:
    ip=0;
    plen=0;

    while(1){
      if(eso_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(eso_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		eso_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		eso_post[ip].key,
		eso_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(eso_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  eso_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(eso_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  eso_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(eso_post[ip].key,"box")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d %02d\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->dss_arcmin/2,
		    hg->dss_arcmin*30-(hg->dss_arcmin/2)*60);
	    break;

	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 04 00\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d 00\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_arcmin);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d 00\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_arcmin_used);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"wdbo")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case TRDB_TYPE_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\nvotable/display\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case FCDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\nhtml/display\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"stime")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_stdate);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_stdate_used);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"etime")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_eddate);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_eddate_used);
	    break;
	  }
	}
	break;

      case POST_INST1:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_IMAGE;i++){
	    if(hg->fcdb_eso_image[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_image[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_IMAGE){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_image[hg->trdb_eso_image].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_IMAGE){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_image[hg->trdb_eso_image_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST2:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_SPEC;i++){
	    if(hg->fcdb_eso_spec[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_spec[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;
	  
	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SPEC){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_spec[hg->trdb_eso_spec].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_SPEC){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_spec[hg->trdb_eso_spec_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST3:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_VLTI;i++){
	    if(hg->fcdb_eso_vlti[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_vlti[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_VLTI){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_vlti[hg->trdb_eso_vlti].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_VLTI){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_vlti[hg->trdb_eso_vlti_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST4:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_POLA;i++){
	    if(hg->fcdb_eso_pola[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_pola[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_POLA){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_pola[hg->trdb_eso_pola].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_POLA){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_pola[hg->trdb_eso_pola_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST5:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_CORO;i++){
	    if(hg->fcdb_eso_coro[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_coro[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_CORO){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_coro[hg->trdb_eso_coro].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_CORO){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_coro[hg->trdb_eso_coro_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST6:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_OTHER;i++){
	    if(hg->fcdb_eso_other[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_other[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_OTHER){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_other[hg->trdb_eso_other].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	  
	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_OTHER){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_other[hg->trdb_eso_other_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST7:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_SAM;i++){
	    if(hg->fcdb_eso_sam[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_sam[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SAM){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_sam[hg->trdb_eso_sam].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_SAM){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_sam[hg->trdb_eso_sam_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_ADD:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n",
		rand16,
		eso_post[ip].key);
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  for(i=0;i<NUM_ESO_IMAGE;i++){
	    if(hg->fcdb_eso_image[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_image[i].add);
	    }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_image[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_SPEC;i++){
	    if(hg->fcdb_eso_spec[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_spec[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_spec[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_VLTI;i++){
	    if(hg->fcdb_eso_vlti[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_vlti[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_vlti[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_POLA;i++){
	    if(hg->fcdb_eso_pola[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_pola[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_pola[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_CORO;i++){
	    if(hg->fcdb_eso_coro[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_coro[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_coro[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_OTHER;i++){
	    if(hg->fcdb_eso_other[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_other[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_other[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_SAM;i++){
	    if(hg->fcdb_eso_sam[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_sam[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_sam[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  if(init_flag){
	    strcat(send_mesg,")\r\n");
	  }
	  else{
	    strcat(send_mesg,"\r\n");
	  }
	  break;

	case TRDB_TYPE_ESO:
	  strcat(send_mesg, "(");
	  switch(hg->trdb_eso_mode){
	  case TRDB_ESO_MODE_IMAGE:
	    strcat(send_mesg, eso_image[hg->trdb_eso_image].add);
	    break;
	  case TRDB_ESO_MODE_SPEC:
	    strcat(send_mesg, eso_spec[hg->trdb_eso_spec].add);
	    break;
	  case TRDB_ESO_MODE_VLTI:
	    strcat(send_mesg, eso_vlti[hg->trdb_eso_vlti].add);
	    break;
	  case TRDB_ESO_MODE_POLA:
	    strcat(send_mesg, eso_pola[hg->trdb_eso_pola].add);
	    break;
	  case TRDB_ESO_MODE_CORO:
	    strcat(send_mesg, eso_coro[hg->trdb_eso_coro].add);
	    break;
	  case TRDB_ESO_MODE_OTHER:
	    strcat(send_mesg, eso_other[hg->trdb_eso_other].add);
	    break;
	  case TRDB_ESO_MODE_SAM:
	    strcat(send_mesg, eso_sam[hg->trdb_eso_sam].add);
	    break;
	  }
	  strcat(send_mesg,")\r\n");
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  strcat(send_mesg, "(");
	  switch(hg->trdb_eso_mode_used){
	  case TRDB_ESO_MODE_IMAGE:
	    strcat(send_mesg, eso_image[hg->trdb_eso_image_used].add);
	    break;
	  case TRDB_ESO_MODE_SPEC:
	    strcat(send_mesg, eso_spec[hg->trdb_eso_spec_used].add);
	    break;
	  case TRDB_ESO_MODE_VLTI:
	    strcat(send_mesg, eso_vlti[hg->trdb_eso_vlti_used].add);
	    break;
	  case TRDB_ESO_MODE_POLA:
	    strcat(send_mesg, eso_pola[hg->trdb_eso_pola_used].add);
	    break;
	  case TRDB_ESO_MODE_CORO:
	    strcat(send_mesg, eso_coro[hg->trdb_eso_coro_used].add);
	    break;
	  case TRDB_ESO_MODE_OTHER:
	    strcat(send_mesg, eso_other[hg->trdb_eso_other_used].add);
	    break;
	  case TRDB_ESO_MODE_SAM:
	    strcat(send_mesg, eso_sam[hg->trdb_eso_sam_used].add);
	    break;
	  }
	  strcat(send_mesg,")\r\n");
	  break;
	}
	break;
      }
	
      if(send_flag){
	plen+=strlen(send_mesg);
	if(wflag){
	  if(SSL_flag){
	    write_to_SSLserver(ssl, send_mesg);
	  }
	  else{
	    write_to_server(command_socket, send_mesg);
	  }
	}
      }
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_mesg);
      }
      else{
	write_to_server(command_socket, send_mesg);
      }
    }

    break;

  case FCDB_TYPE_SDSS:
    ip=0;
    plen=0;

    while(1){
      if(sdss_post[ip].key==NULL) break;
      switch(sdss_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		sdss_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		sdss_post[ip].key,
		sdss_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(sdss_post[ip].key,"searchtool")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG) ? 
		  "Imaging" : "Spectro");
	}
	else if(strcmp(sdss_post[ip].key,"TaskName")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG) ? 
		  "Skyserver.Search.IQS" : "Skyserver.Search.SQS");
	}
	else if(strcmp(sdss_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(sdss_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(sdss_post[ip].key,"radius")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (hg->dss_arcmin < hg->fcdb_sdss_diam) ?
		  ((gdouble)hg->dss_arcmin/2.0) :
		  ((gdouble)hg->fcdb_sdss_diam/2.0));
	}
	else if(strcmp(sdss_post[ip].key,"magMin")==0){
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SDSS_BAND;i++){
	    if(hg->fcdb_sdss_fil[i]){
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMin\"\r\n\r\n%d\r\n",
		      rand16,
		      sdss_band[i],
		      hg->fcdb_sdss_magmin[i]);
	    }
	    else{
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMin\"\r\n\r\n\r\n",
		      rand16,
		      sdss_band[i]);
	    }
	    strcat(send_mesg,ins_mesg);
	  }
	}
	else if(strcmp(sdss_post[ip].key,"magMax")==0){
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SDSS_BAND;i++){
	    if(hg->fcdb_sdss_fil[i]){
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMax\"\r\n\r\n%d\r\n",
		      rand16,
		      sdss_band[i],
		      hg->fcdb_sdss_magmax[i]);
	    }
	    else{
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMax\"\r\n\r\n\r\n",
		      rand16,
		      sdss_band[i]);
	    }
	    strcat(send_mesg,ins_mesg);
	  }
	}
	break;
      }	
      plen+=strlen(send_mesg);
      if(wflag){
	if(SSL_flag){
	  write_to_SSLserver(ssl, send_mesg);
	}
	else{
	  write_to_server(command_socket, send_mesg);
	}
      }
	  
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_mesg);
      }
      else{
	write_to_server(command_socket, send_mesg);
      }
    }

    break;

  case MAGDB_TYPE_SDSS:
    ip=0;
    plen=0;

    while(1){
      if(sdss_post[ip].key==NULL) break;
      switch(sdss_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		sdss_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		sdss_post[ip].key,
		sdss_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(sdss_post[ip].key,"searchtool")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  sdss_post[ip].key,
		  "Imaging");
	}
	else if(strcmp(sdss_post[ip].key,"TaskName")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  sdss_post[ip].key,
		  "Skyserver.Search.IQS");
	}
	else if(strcmp(sdss_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(sdss_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.10lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(sdss_post[ip].key,"radius")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (gdouble)hg->magdb_arcsec/60.);
	}
	else if(strcmp(sdss_post[ip].key,"magMin")==0){
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SDSS_BAND;i++){
	    if(i==hg->magdb_sdss_band){
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMin\"\r\n\r\n%d\r\n",
		      rand16,
		      sdss_band[i],
		      0);
	    }
	    else{
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMin\"\r\n\r\n\r\n",
		      rand16,
		      sdss_band[i]);
	    }
	    strcat(send_mesg,ins_mesg);
	  }
	}
	else if(strcmp(sdss_post[ip].key,"magMax")==0){
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SDSS_BAND;i++){
	    if(i==hg->magdb_sdss_band){
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMax\"\r\n\r\n%d\r\n",
		      rand16,
		      sdss_band[i],
		      hg->magdb_mag);
	    }
	    else{
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMax\"\r\n\r\n\r\n",
		      rand16,
		      sdss_band[i]);
	    }
	    strcat(send_mesg,ins_mesg);
	  }
	}
	break;
      }	
      plen+=strlen(send_mesg);
      if(wflag){
	if(SSL_flag){
	  write_to_SSLserver(ssl, send_mesg);
	}
	else{
	  write_to_server(command_socket, send_mesg);
	}
      }
	  
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_mesg);
      }
      else{
	write_to_server(command_socket, send_mesg);
      }
    }

    break;

    
  case TRDB_TYPE_HST:
  case TRDB_TYPE_FCDB_HST:
    ip=0;
    plen=0;

    switch(hg->trdb_hst_mode){
    case TRDB_HST_MODE_OTHER:
      sci_instrume=g_strdup_printf("{\"sci_obstype\":\"%s\"},{\"sci_instrume\":\"%s\"}",
				   HST_mode[TRDB_HST_MODE_OTHER].prm,
				   HST_inst[hg->trdb_hst_other].prm);
      break;

    case TRDB_HST_MODE_SPEC:
      sci_instrume=g_strdup_printf("{\"sci_obstype\":\"%s\"},{\"sci_instrume\":\"%s\"}",
				   HST_mode[TRDB_HST_MODE_SPEC].prm,
				   HST_inst[hg->trdb_hst_spec].prm);
      break;
      
    case TRDB_HST_MODE_IMAGE:
      sci_instrume=g_strdup_printf("{\"sci_obstype\":\"%s\"},{\"sci_instrume\":\"%s\"}",
				   HST_mode[TRDB_HST_MODE_IMAGE].prm,
				   HST_inst[hg->trdb_hst_image].prm);
      break;
    }
    
    sprintf(send_mesg,"{\"target\":[\"%.5lf %.5lf\"],\"radius\":%.1lf,\"radius_units\":\"arcminutes\",\"conditions\":[%s,{\"sci_start_time\":\"%s 00:00:00..%s 23:59:59\"}],\"offset\":0,\"limit\":%d}",
	    hg->fcdb_d_ra0,
	    hg->fcdb_d_dec0,
	    (gdouble)hg->trdb_arcmin,
	    sci_instrume,
	    hg->trdb_hst_stdate,
	    hg->trdb_hst_eddate,
	    MAX_FCDB);
    g_free(sci_instrume);

    if(send_flag){
      plen+=strlen(send_mesg);
	
      if(send_buf1) g_free(send_buf1);
      if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
      else send_buf1=g_strdup(send_mesg);
      if(send_buf2) g_free(send_buf2);
      send_buf2=g_strdup(send_buf1);
    }

    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    

  case FCDB_TYPE_HST:
    //  case FCDB_TYPE_WWWDB_HST:
    ip=0;
    plen=0;

    for(i_inst=0;i_inst<NUM_HST_INST;i_inst++){
      if(hg->fcdb_hst_inst[i_inst]){
	if(!sci_instrume){
	  sci_instrume=g_strdup_printf(",{\"sci_instrume\":\"%s",
				       HST_inst[i_inst].prm);
	}
	else{
	  tmp_inst=g_strdup_printf("%s,%s",
				   sci_instrume,
				   HST_inst[i_inst].prm);
	  g_free(sci_instrume);
	  sci_instrume=g_strdup(tmp_inst);
	  g_free(tmp_inst);
	}
      }
    }
    if(sci_instrume){
      tmp_inst=g_strdup_printf("%s\"}]",
			       sci_instrume);
      g_free(sci_instrume);
      sci_instrume=g_strdup(tmp_inst);
      g_free(tmp_inst);
    }
    else{
      sci_instrume=g_strdup("]");
    }

    sprintf(send_mesg,"{\"target\":[\"%.5lf %.5lf\"],\"radius\":%.1lf,\"radius_units\":\"arcminutes\",\"conditions\":[{\"sci_obs_type\":\"%s\"}%s,\"offset\":0,\"limit\":%d}",
	    hg->fcdb_d_ra0,
	    hg->fcdb_d_dec0,
	    hg->dss_arcmin/2.0,
	    HST_mode[hg->fcdb_hst_mode].prm,
	    sci_instrume,
	    MAX_FCDB);
    g_free(sci_instrume);

    if(send_flag){
      plen+=strlen(send_mesg);
	
      if(send_buf1) g_free(send_buf1);
      if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
      else send_buf1=g_strdup(send_mesg);
      if(send_buf2) g_free(send_buf2);
      send_buf2=g_strdup(send_buf1);
    }

    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    
  case FCDB_TYPE_KEPLER:
  case MAGDB_TYPE_KEPLER:
    ip=0;
    plen=0;

    while(1){
      if(kepler_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(kepler_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		kepler_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"%s=%s&",
		kepler_post[ip].key,
		kepler_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(kepler_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "%s=%.5lf&",
		  kepler_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(kepler_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "%s=%.5lf&",
		  kepler_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(kepler_post[ip].key,"radius")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_KEPLER:
	    sprintf(send_mesg,
		    "%s=%.5lf&",
		    kepler_post[ip].key,
		    hg->dss_arcmin/2.0);
	    break;
	    
	  case MAGDB_TYPE_KEPLER:
	    sprintf(send_mesg,
		    "%s=%.5lf&",
		    kepler_post[ip].key,
		    (gdouble)hg->magdb_arcsec/30);
	    break;
	  }
	}
	else if(strcmp(kepler_post[ip].key,"extra_column_value_1")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_KEPLER:
	    if(hg->fcdb_kepler_fil){
	      sprintf(send_mesg,
		      "%s=%%3C+%d&",
		      kepler_post[ip].key,
		      hg->fcdb_kepler_mag);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=&",
		      kepler_post[ip].key);
	    }
	    break;

	  case MAGDB_TYPE_KEPLER:
	    sprintf(send_mesg,
		    "%s=%%3C+%d&",
		    kepler_post[ip].key,
		    hg->magdb_mag);
	  }
	}
	else if(strcmp(kepler_post[ip].key,"outputformat")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_KEPLER:
	  case MAGDB_TYPE_KEPLER:
	    sprintf(send_mesg,
		    "%s=VOTable&",
		    kepler_post[ip].key);
	    break;

	  default:
	    sprintf(send_mesg,
		    "%s=HTML_Table&",
		    kepler_post[ip].key);
	    break;
	  }
	}

	break;
      }

      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }

    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
    
  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_SMOKA:
  case TRDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_FCDB_SMOKA:
    ip=0;
    plen=0;

    while(1){
      if(smoka_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(smoka_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		smoka_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"%s=%s&",
		smoka_post[ip].key,
		smoka_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(smoka_post[ip].key,"longitudeC")==0){
	  sprintf(send_mesg,
		  "%s=%.10lf&",
		  smoka_post[ip].key,
		    hg->fcdb_d_ra0);
	}
	else if(strcmp(smoka_post[ip].key,"latitudeC")==0){
	  sprintf(send_mesg,
		  "%s=%.10lf&",
		  smoka_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(smoka_post[ip].key,"radius")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_SMOKA:
	    sprintf(send_mesg,
		    "%s=%.1lf&",
		    smoka_post[ip].key,
		    hg->dss_arcmin/2.0);
	    break;

	  case FCDB_TYPE_WWWDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=2.0&",
		    smoka_post[ip].key);
	    break;

	  case TRDB_TYPE_SMOKA:
	    sprintf(send_mesg,
		    "%s=%d&",
		    smoka_post[ip].key,
		    hg->trdb_arcmin);
	    break;

	  case TRDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=%d&",
		    smoka_post[ip].key,
		    hg->trdb_arcmin_used);
	    break;

	  }
	}
	else if(strcmp(smoka_post[ip].key,"asciitable")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_WWWDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=Table&",
		    smoka_post[ip].key);
	    break;

	  case FCDB_TYPE_SMOKA:
	  case TRDB_TYPE_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=Ascii&",
		    smoka_post[ip].key);
	    break;
	  }
	}
	else if(strcmp(smoka_post[ip].key,"frameorshot")==0){
	  switch(hg->fcdb_type){
	  case TRDB_TYPE_SMOKA:
	    if(((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)
		|| (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0))
	       && (hg->trdb_smoka_shot)) {
	      sprintf(send_mesg,
		      "%s=Shot&",
		      smoka_post[ip].key);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=Frame&",
		      smoka_post[ip].key);
	    }
	    break;
	  case TRDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    if(((strcmp(smoka_subaru[hg->trdb_smoka_inst_used].prm,"SUP")==0)
		|| (strcmp(smoka_subaru[hg->trdb_smoka_inst_used].prm,"HSC")==0))
	       && (hg->trdb_smoka_shot_used)) {
	      sprintf(send_mesg,
		      "%s=Shot&",
		      smoka_post[ip].key);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=Frame&",
		      smoka_post[ip].key);
	    }
	    break;

	  case FCDB_TYPE_SMOKA:
	  case FCDB_TYPE_WWWDB_SMOKA:
	    if(hg->fcdb_smoka_shot){
	      sprintf(send_mesg,
		      "%s=Shot&",
		      smoka_post[ip].key);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=Frame&",
		      smoka_post[ip].key);
	    }
	    break;
	  }
	}
	else if(strcmp(smoka_post[ip].key,"date_obs")==0){
	  switch(hg->fcdb_type){
	  case TRDB_TYPE_SMOKA:
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    hg->trdb_smoka_date);
	    break;

	  case TRDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    hg->trdb_smoka_date_used);
	    break;

	  case FCDB_TYPE_SMOKA:
	  case FCDB_TYPE_WWWDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=&",
		    smoka_post[ip].key);
	    break;
	  }
	}
	break;

      case POST_INST1:
	switch(hg->fcdb_type){
	case TRDB_TYPE_SMOKA:
	  sprintf(send_mesg, "%s=%s&", 
		  smoka_post[ip].key, 
		  smoka_subaru[hg->trdb_smoka_inst].prm);
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  sprintf(send_mesg, "%s=%s&", 
		  smoka_post[ip].key, 
		  smoka_subaru[hg->trdb_smoka_inst_used].prm);
	  break;

	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_SUBARU;i++){
	    if(hg->fcdb_smoka_subaru[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_subaru[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;
	}
	break;

      case POST_INST2:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_KISO;i++){
	    if(hg->fcdb_smoka_kiso[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_kiso[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_INST3:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_OAO;i++){
	    if(hg->fcdb_smoka_oao[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_oao[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_INST4:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_MTM;i++){
	    if(hg->fcdb_smoka_mtm[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_mtm[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_INST5:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_KANATA;i++){
	    if(hg->fcdb_smoka_kanata[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_kanata[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  for(i=0;i<NUM_SMOKA_NAYUTA;i++){
	    if(hg->fcdb_smoka_nayuta[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_nayuta[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_IMAG:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  sprintf(send_mesg,
		  "%s=%s&",
		  smoka_post[ip].key,
		  smoka_post[ip].prm);
	  break;

	case TRDB_TYPE_SMOKA:
	  if(hg->trdb_smoka_imag){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  if(hg->trdb_smoka_imag_used){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_SPEC:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  sprintf(send_mesg,
		  "%s=%s&",
		  smoka_post[ip].key,
		  smoka_post[ip].prm);
	  break;

	case TRDB_TYPE_SMOKA:
	  if(hg->trdb_smoka_spec){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  if(hg->trdb_smoka_spec_used){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_IPOL:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  sprintf(send_mesg,
		  "%s=%s&",
		  smoka_post[ip].key,
		  smoka_post[ip].prm);
	  break;

	case TRDB_TYPE_SMOKA:
	  if(hg->trdb_smoka_ipol){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  if(hg->trdb_smoka_ipol_used){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;
      }

      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }

    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);

    plen+=strlen(send_mesg);

    if(wflag){
      if(SSL_flag){
	write_to_SSLserver(ssl, send_buf1);
      }
      else{
	write_to_server(command_socket, send_buf1);
      }
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;
  }

  return(plen);
}



int http_c_fcdb_new(typHOE *hg, gboolean SSL_flag, gboolean proxy_ssl){
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct addrinfo dhints, *dres;
  struct sockaddr_in *addr_in;
  struct in_addr addr;
  int err, ret;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  SSL *ssl;
  SSL_CTX *ctx;

  // Calculate Content-Length
  if(hg->fcdb_post){
    rand16=make_rand16();
    plen=post_body_new(hg, FALSE, 0, NULL, rand16, FALSE);
  }
   
  check_msg_from_parent(hg);

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo((hg->proxy_flag) ? hg->proxy_host : hg->fcdb_host,
			 (SSL_flag) ? "https" : "http",
			 &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n",
	    (hg->proxy_flag) ? hg->proxy_host : hg->fcdb_host);
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent(hg);

  if(hg->proxy_flag){
    addr_in = (struct sockaddr_in *)(res -> ai_addr);
    addr_in -> sin_port=htons(hg->proxy_port);
  }

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }

  check_msg_from_parent(hg);

  /* サーバに接続 */
  if( Connect(command_socket, res->ai_addr, res->ai_addrlen, hg->http_timeout) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->fcdb_host);
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent(hg);

  if(SSL_flag){  // HTTPS
    SSL_load_error_strings();
    SSL_library_init();

    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, command_socket);

    SSL_set_tlsext_host_name(ssl, hg->fcdb_host);
    
    while((ret=SSL_connect(ssl))!=1){
      err=SSL_get_error(ssl, ret);
      if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
	g_usleep(100000);
	g_warning("SSL_connect(): try again\n");
	continue;
      }
      g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
		err, ret, ERR_error_string(ERR_get_error(), NULL));
      return (HSKYMON_HTTP_ERROR_CONNECT);
    }

    check_msg_from_parent(hg);
  }
  
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  if(SSL_flag){  // HTTPS
    if(hg->fcdb_post){
      sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hg->fcdb_path);
    }
    else{
      sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->fcdb_path);
    }
    write_to_SSLserver(ssl, send_mesg);

    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_SSLserver(ssl, send_mesg);

    sprintf(send_mesg, "Host: %s\r\n", hg->fcdb_host);
    write_to_SSLserver(ssl, send_mesg);

  }
  else{ // HTTP
    if(hg->proxy_flag){
      if(proxy_ssl){
	if(hg->fcdb_post){
	  sprintf(send_mesg, "POST https://%s%s HTTP/1.1\r\n",
		  hg->fcdb_host,hg->fcdb_path);
	}
	else{
	  sprintf(send_mesg, "GET https://%s%s HTTP/1.1\r\n",
		  hg->fcdb_host,hg->fcdb_path);
	}
      }
      else{ 
	if(hg->fcdb_post){
	  sprintf(send_mesg, "POST http://%s%s HTTP/1.1\r\n",
		  hg->fcdb_host,hg->fcdb_path);
	}
	else{
	  sprintf(send_mesg, "GET http://%s%s HTTP/1.1\r\n",
		  hg->fcdb_host,hg->fcdb_path);
	}
      }
    }
    else{
      if(hg->fcdb_post){
	sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hg->fcdb_path);
      }
      else{
	sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->fcdb_path);
      }
    }
    write_to_server(command_socket, send_mesg);

    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_server(command_socket, send_mesg);
    
    sprintf(send_mesg, "Host: %s\r\n", hg->fcdb_host);
    write_to_server(command_socket, send_mesg);
  }

  switch(hg->fcdb_type){
  case FCDB_TYPE_HST:
  case FCDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_HST:
  case TRDB_TYPE_FCDB_HST:
    //case FCDB_TYPE_KEPLER:
    sprintf(send_mesg, "Accept: */*\r\n");
    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }
    break;

  case DBACCESS_VER:
  case DBACCESS_HSCFIL:
  case DBACCESS_HDSCAMZ:
  case DBACCESS_IRCSSET:
  case DBACCESS_IRDSET:
    break;
    
  default:
    sprintf(send_mesg, "Accept: application/xml, application/json\r\n");
    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }
    break;
  }

  sprintf(send_mesg, "Connection: close\r\n");
  if(SSL_flag){  // HTTPS
    write_to_SSLserver(ssl, send_mesg);
  }
  else{  // HTTP
    write_to_server(command_socket, send_mesg);
  }

  /////////// POST
  if(hg->fcdb_post){
    sprintf(send_mesg, "Content-Length: %d\r\n", plen);
    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }

    switch(hg->fcdb_type){
    case FCDB_TYPE_SDSS:
    case MAGDB_TYPE_SDSS:
    case FCDB_TYPE_LAMOST:
    case MAGDB_TYPE_LAMOST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_WWWDB_ESO:
    case TRDB_TYPE_ESO:
    case TRDB_TYPE_WWWDB_ESO:
    case TRDB_TYPE_FCDB_ESO:
      sprintf(send_mesg, "Content-Type:multipart/form-data; boundary=----WebKitFormBoundary%s\r\n", rand16);
      break;

    case MAGDB_TYPE_SEIMEI_KOOLS:
    case MAGDB_TYPE_SEIMEI_PLAN_KOOLS:
    case MAGDB_TYPE_SEIMEI_TRICCS:
    case MAGDB_TYPE_SEIMEI_PLAN_TRICCS:
      sprintf(send_mesg, "Content-Type: application/x-www-form-urlencoded\r\n");
      break;

    case FCDB_TYPE_HST:
    case FCDB_TYPE_WWWDB_HST:
    case TRDB_TYPE_HST:
    case TRDB_TYPE_FCDB_HST:
      sprintf(send_mesg, "Content-Type: application/json\r\n");
      break;
      
    default:
      sprintf(send_mesg, "Content-Type: application/x-www-form-urlencoded\r\n");
      break;
    }

    if(SSL_flag){  // HTTPS
      write_to_SSLserver(ssl, send_mesg);
    }
    else{  // HTTP
      write_to_server(command_socket, send_mesg);
    }
  }

  sprintf(send_mesg, "\r\n");
  if(SSL_flag){  // HTTPS
    write_to_SSLserver(ssl, send_mesg);
  }
  else{  // HTTP
    write_to_server(command_socket, send_mesg);
  }

  // POST body
  if(hg->fcdb_post){
    if(SSL_flag){
      plen=post_body_new(hg, TRUE, 0, ssl, rand16, TRUE);
    }
    else{
      plen=post_body_new(hg, TRUE, command_socket, NULL, rand16, FALSE);
    }
    if(rand16) g_free(rand16);
  }

  // Download a file
  if((fp_write=fopen(hg->fcdb_file,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->fcdb_file);
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }

  if(SSL_flag){  // HTTPS
    while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"[SSL] --> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
      if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	cp = buf + strlen("Content-Length: ");
	hg->psz=atol(cp);
      }
    }
    // data read
    SSL_data_read(command_socket, ssl, ctx, fp_write, hg->http_nonblock);
  }
  else{  // HTTP
    if((fp_write=fopen(hg->fcdb_file,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->fcdb_file);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
      if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	cp = buf + strlen("Content-Length: ");
	hg->psz=atol(cp);
      }
    }
    // data read
    HTTP_data_read(command_socket, fp_write, hg->http_nonblock);
  }
  
  fclose(fp_write);

  check_msg_from_parent(hg);

  if(chunked_flag) unchunk(hg->fcdb_file);
  // This is a bug fix for SDSS DR16 VOTable output
  if((hg->fcdb_type==FCDB_TYPE_SDSS)||(hg->fcdb_type==MAGDB_TYPE_SDSS)){ 
    str_replace(hg->fcdb_file, 
		"encoding=\"utf-16\"",
		" encoding=\"utf-8\"");
  }    

  if((chmod(hg->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->fcdb_file);
  }

  if(SSL_flag){
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
  }
  
  close(command_socket);

  return 0;
}


int http_c_fc_new(typHOE *hg, gboolean SSL_flag, gboolean proxy_ssl){
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  
  gchar *tmp_file=NULL;

  gchar *tmp, *tmp_scale;
  gfloat sdss_scale=SDSS_SCALE;
  gint xpix,ypix,i_bin;
  static char cbuf[BUFFSIZE];
  gchar *cp, *cpp, *cp2, *cp3=NULL,  *c_test=NULL;
  FILE *fp_read;

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  struct addrinfo hints, *res;
  struct sockaddr_in *addr_in;
  struct in_addr addr;
  int err, ret;

  gboolean chunked_flag=FALSE;

  SSL *ssl;
  SSL_CTX *ctx;

  hg->psz=0;

  check_msg_from_parent(hg);
   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo((hg->proxy_flag) ? hg->proxy_host : hg->dss_host,
			 (SSL_flag) ? "https" : "http",
			 &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->dss_host);
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent(hg);

  if(hg->proxy_flag){
    addr_in = (struct sockaddr_in *)(res -> ai_addr);
    addr_in -> sin_port=htons(hg->proxy_port);
  }
  
  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent(hg);
 
  /* サーバに接続 */
  if( Connect(command_socket, res->ai_addr, res->ai_addrlen, hg->http_timeout) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent(hg);

  if(SSL_flag){  // HTTPS
    SSL_load_error_strings();
    SSL_library_init();
    
    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, command_socket);
    
    SSL_set_tlsext_host_name(ssl, hg->dss_host);
    
    while((ret=SSL_connect(ssl))!=1){
      err=SSL_get_error(ssl, ret);
      if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
	g_usleep(100000);
	g_warning("SSL_connect(): try again\n");
	continue;
      }
      g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
		err, ret, ERR_error_string(ERR_get_error(), NULL));
      return(HSKYMON_HTTP_ERROR_SSL);
    }
  
    check_msg_from_parent(hg);
  }

  
  // bin mode
  object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);

  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_epoch(hg->obj[hg->dss_i].equinox),
		    JD2000, &object_prec);
  ln_equ_to_hequ (&object_prec, &hobject_prec);


  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src,
			hobject_prec.ra.hours,hobject_prec.ra.minutes,
			hobject_prec.ra.seconds,
			(hobject_prec.dec.neg) ? "-" : "+", 
			hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			hobject_prec.dec.seconds,
			hg->dss_arcmin,hg->dss_arcmin);
    break;
    
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
    tmp=g_strdup_printf(hg->dss_path,
			hobject_prec.ra.hours,hobject_prec.ra.minutes,
			hobject_prec.ra.seconds,
			(hobject_prec.dec.neg) ? "-" : "%2B", 
			hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			hobject_prec.dec.seconds,
			hg->dss_arcmin,hg->dss_arcmin,hg->dss_src);
    break;

  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
    switch(hg->dss_scale){
    case FC_SCALE_LOG:
      tmp_scale=g_strdup("Log");
      break;
    case FC_SCALE_SQRT:
      tmp_scale=g_strdup("Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      tmp_scale=g_strdup("HistEq");
      break;
    case FC_SCALE_LOGLOG:
      tmp_scale=g_strdup("LogLog");
      break;
    default:
      tmp_scale=g_strdup("Linear");
    }
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src, 2000.0,
			tmp_scale,
			(hg->dss_invert) ? "&invert=on&" : "&",
			(gdouble)hg->dss_arcmin/60.,
			(gdouble)hg->dss_arcmin/60.,
			hg->dss_pix,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec));
    if(tmp_scale) g_free(tmp_scale);
    break;

  case FC_SDSS:
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1000)||(ypix>1000));
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			sdss_scale,
			xpix,
			ypix,
			(hg->sdss_photo) ? "P" : "",
			(hg->sdss_spec) ? "S" : "",
			(hg->sdss_photo) ? "&PhotoObjs=on" : "",
			(hg->sdss_spec) ? "&SpecObjs=on" : "");
    break;
    
  case FC_SDSS13:
    xpix=1500;
    ypix=1500;
    sdss_scale=((gfloat)hg->dss_arcmin*60.)/(gfloat)xpix;
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			sdss_scale,
			xpix,
			ypix,
			(hg->sdss_photo) ? "P" : "",
			(hg->sdss_spec) ? "S" : "",
			(hg->sdss_photo) ? "&PhotoObjs=on" : "",
			(hg->sdss_spec) ? "&SpecObjs=on" : "");
    break;


  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if(hg->dss_arcmin>PANSTARRS_MAX_ARCMIN){
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
    }
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			hg->dss_arcmin*240);
    break;

  }

  hg->psz=0;

  if(SSL_flag){  // HTTPS
    sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
    write_to_SSLserver(ssl, send_mesg);
    if(tmp) g_free(tmp);
    
    sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "Connection: close\r\n");
    write_to_SSLserver(ssl, send_mesg);
    
    sprintf(send_mesg, "\r\n");
    write_to_SSLserver(ssl, send_mesg);
  }
  else{  // HTTP
    if(hg->proxy_flag){
      if(proxy_ssl){
	sprintf(send_mesg, "GET https://%s%s HTTP/1.1\r\n", hg->dss_host,tmp);
      }
      else{
	sprintf(send_mesg, "GET http://%s%s HTTP/1.1\r\n", hg->dss_host,tmp);
      }
    }
    else{
      sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
    }
    write_to_server(command_socket, send_mesg);
    if(tmp) g_free(tmp);

    sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
    write_to_server(command_socket, send_mesg);
    
    sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
    write_to_server(command_socket, send_mesg);
    
    sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
    write_to_server(command_socket, send_mesg);
    
    sprintf(send_mesg, "Connection: close\r\n");
    write_to_server(command_socket, send_mesg);
    
    sprintf(send_mesg, "\r\n");
    write_to_server(command_socket, send_mesg);
  }

  switch(hg->fc_mode){
    //case FC_ESO_DSS1R:
    //case FC_ESO_DSS2R:
    //case FC_ESO_DSS2B:
    //case FC_ESO_DSS2IR:
  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if((fp_write=fopen(hg->dss_tmp,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_tmp);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }
    
    check_msg_from_parent(hg);

    if(SSL_flag){  // HTTPS
      while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"[SSL] --> Header: %s", buf);
	}
	if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
	if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	  cp = buf + strlen("Content-Length: ");
	  hg->psz=atol(cp);
	}
      }
      // data read
      SSL_data_read(command_socket, ssl, ctx, fp_write, hg->http_nonblock);
    }
    else{  // HTTP
      while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"--> Header: %s", buf);
	}
	if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
	if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	  cp = buf + strlen("Content-Length: ");
	  hg->psz=atol(cp);
	}
      }
      // data read
      HTTP_data_read(command_socket, fp_write, hg->http_nonblock);
    }

    fclose(fp_write);

    check_msg_from_parent(hg);
    
    if((chmod(hg->dss_tmp,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
      g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_tmp);
    }

    if(chunked_flag) unchunk(hg->dss_tmp);
    
    fp_read=fopen(hg->dss_tmp,"r");
    
    switch(hg->fc_mode){
      /*
    case FC_ESO_DSS1R:
    case FC_ESO_DSS2R:
    case FC_ESO_DSS2B:
    case FC_ESO_DSS2IR:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  //if(NULL != (cp = my_strcasestr(cpp, "<A HREF="))){
	  if(NULL != (cp = strstr(cpp, "<A HREF="))){
	    cpp=cp+strlen("<A HREF=");
	    
	    if(NULL != (cp2 = my_strcasestr(cp, ">"))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;
      */
    case FC_SKYVIEW_GALEXF:
    case FC_SKYVIEW_GALEXN:
    case FC_SKYVIEW_DSS1R:
    case FC_SKYVIEW_DSS1B:
    case FC_SKYVIEW_DSS2R:
    case FC_SKYVIEW_DSS2B:
    case FC_SKYVIEW_DSS2IR:
    case FC_SKYVIEW_SDSSU:
    case FC_SKYVIEW_SDSSG:
    case FC_SKYVIEW_SDSSR:
    case FC_SKYVIEW_SDSSI:
    case FC_SKYVIEW_SDSSZ:
    case FC_SKYVIEW_2MASSJ:
    case FC_SKYVIEW_2MASSH:
    case FC_SKYVIEW_2MASSK:
    case FC_SKYVIEW_WISE34:
    case FC_SKYVIEW_WISE46:
    case FC_SKYVIEW_WISE12:
    case FC_SKYVIEW_WISE22:
    case FC_SKYVIEW_AKARIN60:
    case FC_SKYVIEW_AKARIWS:
    case FC_SKYVIEW_AKARIWL:
    case FC_SKYVIEW_AKARIN160:
    case FC_SKYVIEW_NVSS:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = my_strcasestr(cpp, "x['_output']='../.."))){
	    cpp=cp+strlen("x['_output']='../..");
	    
	    if(NULL != (cp2 = strstr(cp, "'"))){
	      cp3=g_strndup(cpp,strlen(cpp)-2);
	    }
	    
	    break;
	  }
	}
      }
      break;

    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANZ:
    case FC_PANY:
      
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = my_strcasestr(cpp, "<img src=\"//" FC_HOST_PANCOL))){
	    cpp=cp+strlen("<img src=\"//" FC_HOST_PANCOL);
	    
	    if(NULL != (cp2 = my_strcasestr(cp, "\" width="))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    }
    
    fclose(fp_read);

    check_msg_from_parent(hg);

    if(SSL_flag){  // HTTPS
      SSL_shutdown(ssl);
      SSL_free(ssl);
      SSL_CTX_free(ctx);
      ERR_free_strings();
    }
    close(command_socket);

    chunked_flag=FALSE;
    
    /* サーバに接続 */
    if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }

    check_msg_from_parent(hg);

    if( Connect(command_socket, res->ai_addr, res->ai_addrlen, hg->http_timeout) != 0){
      fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
      return(HSKYMON_HTTP_ERROR_CONNECT);
    }

    check_msg_from_parent(hg);

    // AddrInfoの解放
    freeaddrinfo(res);

    if(SSL_flag){  // HTTPS
      SSL_load_error_strings();
      SSL_library_init();

      ctx = SSL_CTX_new(SSLv23_client_method());
      ssl = SSL_new(ctx);
      err = SSL_set_fd(ssl, command_socket);

      SSL_set_tlsext_host_name(ssl, hg->dss_host);
      
      while((ret=SSL_connect(ssl))!=1){
	err=SSL_get_error(ssl, ret);
	if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
	  g_usleep(100000);
	  g_warning("SSL_connect(): try again\n");
	  continue;
	}
	g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
		  err, ret, ERR_error_string(ERR_get_error(), NULL));
	return(HSKYMON_HTTP_ERROR_SSL);
      }
    
      check_msg_from_parent(hg);
    }

    hg->psz=0;
    if(cp3){

      if(hg->proxy_flag){
	if(strncasecmp(cp3, "HTTP", strlen("HTTP"))!=0){
	  if(proxy_ssl){
	    c_test=g_strdup_printf("https://%s%s",hg->dss_host,cp3);
	  }
	  else{
	    c_test=g_strdup_printf("https://%s%s",hg->dss_host,cp3);
	  }
	  g_free(cp3);
	  cp3=g_strdup(c_test);
	  g_free(c_test);
	}
      }
      
      switch(hg->fc_mode){
	//case FC_ESO_DSS1R:
	//case FC_ESO_DSS2R:
	//case FC_ESO_DSS2B:
	//case FC_ESO_DSS2IR:
      case FC_PANCOL:
      case FC_PANG:
      case FC_PANR:
      case FC_PANI:
      case FC_PANZ:
      case FC_PANY:
	sprintf(send_mesg, "GET %s HTTP/1.1\r\n", cp3);
	break;

      case FC_SKYVIEW_GALEXF:
      case FC_SKYVIEW_GALEXN:
      case FC_SKYVIEW_DSS1R:
      case FC_SKYVIEW_DSS1B:
      case FC_SKYVIEW_DSS2R:
      case FC_SKYVIEW_DSS2B:
      case FC_SKYVIEW_DSS2IR:
      case FC_SKYVIEW_SDSSU:
      case FC_SKYVIEW_SDSSG:
      case FC_SKYVIEW_SDSSR:
      case FC_SKYVIEW_SDSSI:
      case FC_SKYVIEW_SDSSZ:
      case FC_SKYVIEW_2MASSJ:
      case FC_SKYVIEW_2MASSH:
      case FC_SKYVIEW_2MASSK:
      case FC_SKYVIEW_WISE34:
      case FC_SKYVIEW_WISE46:
      case FC_SKYVIEW_WISE12:
      case FC_SKYVIEW_WISE22:
      case FC_SKYVIEW_AKARIN60:
      case FC_SKYVIEW_AKARIWS:
      case FC_SKYVIEW_AKARIWL:
      case FC_SKYVIEW_AKARIN160:
      case FC_SKYVIEW_NVSS:
	sprintf(send_mesg, "GET %s.jpg HTTP/1.1\r\n", cp3);
	break;
      }

      if(SSL_flag){  // HTTPS
	write_to_SSLserver(ssl, send_mesg);
      
	sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
	write_to_SSLserver(ssl, send_mesg);
	
	sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
	write_to_SSLserver(ssl, send_mesg);

	sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
	write_to_SSLserver(ssl, send_mesg);
	
	sprintf(send_mesg, "Connection: close\r\n");
	write_to_SSLserver(ssl, send_mesg);

	sprintf(send_mesg, "\r\n");
	write_to_SSLserver(ssl, send_mesg);
      }
      else{  // HTTP
	write_to_server(command_socket, send_mesg);
	
	sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
	write_to_server(command_socket, send_mesg);
	
	sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
	write_to_server(command_socket, send_mesg);
	
	sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
	write_to_server(command_socket, send_mesg);
	
	sprintf(send_mesg, "Connection: close\r\n");
	write_to_server(command_socket, send_mesg);
	
	sprintf(send_mesg, "\r\n");
	write_to_server(command_socket, send_mesg);
      }
  
      if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
	fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
	return(HSKYMON_HTTP_ERROR_TEMPFILE);
      }

      if(SSL_flag){  // HTTPS
	while((size = ssl_gets(ssl, buf,BUF_LEN)) > 2 ){
	  // header lines
	  if(debug_flg){
	    fprintf(stderr,"[SSL] --> Header: %s", buf);
	  }
	  if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	    chunked_flag=TRUE;
	  }
	  if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	    cp = buf + strlen("Content-Length: ");
	    hg->psz=atol(cp);
	  }
	}
	// data read
	SSL_data_read(command_socket, ssl, ctx, fp_write, hg->http_nonblock);
      }
      else{  // HTTP
	while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
	  // header lines
	  if(debug_flg){
	    fprintf(stderr,"--> Header: %s", buf);
	  }
	  if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	    chunked_flag=TRUE;
	  }
	  if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	    cp = buf + strlen("Content-Length: ");
	    hg->psz=atol(cp);
	  }
	}
	// data read
	HTTP_data_read(command_socket, fp_write, hg->http_nonblock);
      }
	
      fclose(fp_write);

      check_msg_from_parent(hg);
    }
      
    break;

  default:
    if ( debug_flg ){
      fprintf(stderr," File Writing...  \"%s\" \n", hg->dss_file);
    }
    if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }

    check_msg_from_parent(hg);

    if(SSL_flag){  // HTTP
      while((size = ssl_gets(ssl, buf,BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"--> Header: %s", buf);
	}
	if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
	if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	  cp = buf + strlen("Content-Length: ");
	  hg->psz=atol(cp);
	}
      }
      // data read
      SSL_data_read(command_socket, ssl, ctx, fp_write, hg->http_nonblock);
    }
    else{  // HTTP
      while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"--> Header: %s", buf);
	}
	if(NULL != (cp = my_strcasestr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
	if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
	  cp = buf + strlen("Content-Length: ");
	  hg->psz=atol(cp);
	}
      }
      // data read
      HTTP_data_read(command_socket, fp_write, hg->http_nonblock);
    }
    
    
    fclose(fp_write);

    check_msg_from_parent(hg);
    
    if ( debug_flg ){
      fprintf(stderr," Done.\n");
    }    
    break;
  }
  
  check_msg_from_parent(hg);

  if(chunked_flag) unchunk(hg->dss_file);

  if((chmod(hg->dss_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_file);
  }

  if(SSL_flag){  // HTTPS
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
  }
  
  close(command_socket);

  return 0;
}

#ifdef USE_WIN32
int Connect(int socket, struct sockaddr * name, socklen_t namelen, gint timeout_sec)
{
  unsigned long mode;
  fd_set readFd, writeFd, errFd;
  int result;
  int errono;
  int sockNum;
  struct timeval timeout;

  timeout.tv_sec=timeout_sec;
  timeout.tv_usec=0;
  
  //接続前にいったん非同期に変更
  mode = 1;
  result= ioctlsocket(socket, FIONBIO, &mode);
  if(NO_ERROR != result)
    {
      return -1;
    }
  
  //接続
  result= connect(socket, name, namelen);
  if(result == -1)
    {
      errono = WSAGetLastError();
      if(WSAEWOULDBLOCK == errono)
        {
	  //非同期接続成功だとここに入る。select()で完了を待つ。
	  errno = 0;
        }
      else
        {
	  //接続失敗
	  //同期型に戻す。
	  mode = 0;
	  ioctlsocket(socket, FIONBIO, &mode);
	  return -1;
        }
    }
  
  //同期型に戻す。
  mode = 0;
  result= ioctlsocket(socket, FIONBIO, &mode);
  if(0 < result)
    {
      //error
      return -1;
    }
  
  //セレクトで待つ
  FD_ZERO(&readFd);
  FD_ZERO(&writeFd);
  FD_ZERO(&errFd);
  FD_SET(socket, &readFd);
  FD_SET(socket, &writeFd);
  FD_SET(socket, &errFd);
  sockNum = select(socket + 1, &readFd, &writeFd, &errFd, &timeout);
  if(0 == sockNum)
    {
      //timeout
      return -1;
    }
  else if(FD_ISSET(socket, &readFd) || FD_ISSET(socket, &writeFd) )
    {
      //読み書きできる状態
    }
  else
    {
      //error
      return -1;
    }

  
  return 0;
}

#else
//タイムアウト付きコネクト(非同期コネクト)
int Connect(int socket, struct sockaddr * name, socklen_t namelen, gint timeout_sec)
{
  struct timeval timeout;
  int result, flags;
  fd_set readFd, writeFd, errFd;
  int sockNum;

  timeout.tv_sec=timeout_sec;
  timeout.tv_usec=0;
  
  //接続前に一度非同期に変更
  flags = fcntl(socket, F_GETFL);
  if(-1 == flags)
    {
      return -1;
    }
  result = fcntl(socket, F_SETFL, flags | O_NONBLOCK);
  if(-1 == result)
    {
        return -1;
    }
  
  //接続
  result = connect(socket, name, namelen);
  if(result == -1)
    {
      if(EINPROGRESS == errno)
	{
	  //非同期接続成功だとここに入る。select()で完了を待つ。
	  errno = 0;
        }
      else
        {
	  //接続失敗 同期に戻す。
	  fcntl(socket, F_SETFL, flags );
	  return -1;
        }
    }
  
  //同期に戻す。
  result = fcntl(socket, F_SETFL, flags );
  if(-1 == result)
    {
      //error
      return -1;
    }
  
  //セレクトで待つ
  FD_ZERO(&readFd);
  FD_ZERO(&writeFd);
  FD_ZERO(&errFd);
  FD_SET(socket, &readFd);
  FD_SET(socket, &writeFd);
  FD_SET(socket, &errFd);
  sockNum = select(socket + 1, &readFd, &writeFd, &errFd, &timeout);
  if(0 == sockNum)
    {
      //timeout error
      return -1;
    }
  else if(FD_ISSET(socket, &readFd) || FD_ISSET(socket, &writeFd) )
    {
      //読み書きできる状態
    }
  else
    {
      //error
      return -1;
    }

    //ソケットエラー確認
    int optval = 0;
    socklen_t optlen = (socklen_t)sizeof(optval);
    errno = 0;
    result = getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)&optval, &optlen);
    if(result < 0)
    {
        //error
    }
    else if(0 != optval)
    {
        //error
    }

    return 0;
}

#endif
