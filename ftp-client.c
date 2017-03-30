/* $Id: ftp-client.c,v 1.4 2004/05/29 05:36:31 68user Exp $ */

#include "main.h"
#include <sys/param.h>
#include <ctype.h>

#ifdef USE_WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/uio.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <signal.h>
#include <strings.h>

#ifdef USE_SSL
#include<fcntl.h>
#  include "ssl.h"
#endif

#ifdef USE_WIN32
#define BUF_LEN 65535             /* バッファのサイズ */
#else
#define BUF_LEN 1023             /* バッファのサイズ */
#endif

#ifdef USE_WIN32
unsigned __stdcall http_c_fc();
#ifdef USE_SSL
unsigned __stdcall http_c_fc_ssl();
#endif
#else
int http_c_fc();
extern pid_t fc_pid;
#ifdef USE_SSL
int http_c_fc_ssl();
#endif
#endif
int get_dss();

void unchunk();

#ifdef USE_SSL
gint ssl_read();
gint ssl_peek();
gint ssl_gets();
gint ssl_write();
#endif

int debug_flg = 0;      /* -d オプションを付けると turn on する */

extern void ext_play();
extern gdouble get_julian_day_of_epoch();


static gint fd_check_io(gint fd, GIOCondition cond)
{
	struct timeval timeout;
	fd_set fds;
	guint io_timeout=60;
	//SockInfo *sock;

	//sock = sock_find_from_fd(fd);
	//if (sock && !SOCK_IS_CHECK_IO(sock->flags))
	//	return 0;

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

#ifdef USE_SSL
void write_to_SSLserver(SSL *ssl, char *p){
  if ( debug_flg ){
    fprintf(stderr, "[SSL] <-- %s", p);fflush(stderr);
  }
  
  ssl_write(ssl, p, strlen(p));
}
#endif

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


int sftp_c(typHOE *hg){
  gchar *cmdline;
  FILE *fp;
  long sz;
  gchar buf[BUF_LEN];
  int ret;
  gchar *sftp_py, *sftp_log;
  
  sftp_py=g_strconcat(hg->temp_dir,
		      G_DIR_SEPARATOR_S,
		      SFTP_PY,
		      NULL);
  sftp_log=g_strconcat(hg->temp_dir,
		       G_DIR_SEPARATOR_S,
		       SFTP_LOG,
		       NULL);

  if((fp=fopen(sftp_py,"w"))==NULL){
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: cannot create temporary_file.",
		  " ",
		  sftp_py,
		  NULL);
    g_free(sftp_py);
    g_free(sftp_log);
    return(-1);
  }

  fprintf(fp,"#!/usr/bin/env python\n");
  fprintf(fp,"# -*- coding:UTF-8 -*-\n\n");
  fprintf(fp,"import paramiko\n\n");

  fprintf(fp,"HOST = \'%s\'\n", SOSS_HOSTNAME);
  fprintf(fp,"USER = \'%s\'\n", hg->prop_id);
  fprintf(fp,"PSWD = r\'%s\'\n\n", hg->prop_pass);
  
  fprintf(fp,"LOCAL_PATH = r\"%s\"\n", hg->filename_read);
  fprintf(fp,"REMOTE_PATH = \"/home/%s/%s/%s\"\n\n", 
	  hg->prop_id,SOSS_PATH,g_path_get_basename(hg->filename_read));

  fprintf(fp,"ssh = paramiko.SSHClient()\n");
  fprintf(fp,"ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())\n");
  fprintf(fp,"ssh.connect(HOST, username=USER, password=PSWD)\n\n");
 
  fprintf(fp,"sftp = ssh.open_sftp()\n");
  fprintf(fp,"sftp.put(LOCAL_PATH, REMOTE_PATH)\n");
  fprintf(fp,"sftp.close()\n\n");
 
  fprintf(fp,"ssh.close()\n");

  fclose(fp);

  cmdline=g_strconcat(PY_COM,
		      " ",
		      sftp_py,
		      " 2> ",
		      sftp_log,
		      NULL);

  ret=system(cmdline);
  g_free(cmdline);

  remove(sftp_py);

  if((fp=fopen(sftp_log,"r"))==NULL){
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: cannot create temporary log file.",
		  " ",
		  sftp_log,
		  NULL);
    g_free(sftp_py);
    g_free(sftp_log);
    return(-2);
  }
  fseek(fp,0,SEEK_END);
  sz=ftell(fp);

  if(sz>0){
    GtkWidget *dialog, *label, *button, *pixmap, *vbox0, *vbox, *hbox,
      *text, *scr;
    GtkTextBuffer *text_buffer;
    gchar *msg;

    dialog = gtk_dialog_new_with_buttons("HOE : Sftp failed",
					 NULL,
					 GTK_DIALOG_MODAL,
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
					 NULL);

    vbox0 = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox0), 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		       vbox0,FALSE, FALSE, 0);

    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(vbox0),
		       hbox,FALSE, FALSE, 0);
    
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
				     GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);


    vbox = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);
    
    label = gtk_label_new ("Failed to upload via sftp!!");
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    msg=g_strconcat("   Host : ",SOSS_HOSTNAME, NULL);
    label = gtk_label_new (msg);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    g_free(msg);

    msg=g_strconcat("   User : ",hg->prop_id, NULL);
    label = gtk_label_new (msg);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    g_free(msg);

    msg=g_strconcat("   Password : ",hg->prop_pass, NULL);
    label = gtk_label_new (msg);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    g_free(msg);

    label = gtk_label_new (" =================== <error message> =================== ");
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox0),label,FALSE, FALSE, 0);

    scr = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scr),
				   GTK_POLICY_AUTOMATIC, 
				   GTK_POLICY_AUTOMATIC);

    text_buffer = gtk_text_buffer_new(NULL);

    text = gtk_text_view_new_with_buffer (text_buffer);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text), FALSE);
    
    gtk_container_add(GTK_CONTAINER(scr), text);
    
    gtk_box_pack_start(GTK_BOX(vbox0),scr,TRUE, TRUE, 0);


    fseek(fp,0,SEEK_SET);
    while(fgets(buf,BUF_LEN-1,fp)!=NULL){
	//fprintf(stderr," : %s",buf);
      gtk_text_buffer_insert_at_cursor(text_buffer,
				       buf,
				       strlen(buf));
    }

    gtk_widget_set_usize(scr,480,200);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      gtk_widget_destroy(dialog);
    }

    remove(sftp_log);
    g_free(sftp_py);
    g_free(sftp_log);
    return(-3);
  }
  fclose(fp);

  g_free(sftp_py);
  g_free(sftp_log);
  remove(sftp_log);
  
  popup_message(GTK_STOCK_OK, POPUP_TIMEOUT,
		"The OPE file has been successfully uploaded to Gen2.",
		NULL);
  return(0);
}


int sftp_get_c(typHOE *hg){
  gchar *cmdline;
  FILE *fp;
  long sz;
  gchar buf[BUF_LEN];
  int ret;
  gchar *sftp_py, *sftp_log, *log_file;
  
  sftp_py=g_strconcat(hg->temp_dir,
		      G_DIR_SEPARATOR_S,
		      SFTP_PY,
		      NULL);
  sftp_log=g_strconcat(hg->temp_dir,
		       G_DIR_SEPARATOR_S,
		       SFTP_LOG,
		       NULL);

  if((fp=fopen(sftp_py,"w"))==NULL){
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: cannot create temporary_file.",
		  " ",
		  sftp_py,
		  NULL);
    g_free(sftp_py);
    g_free(sftp_log);
    return(-1);
  }

  fprintf(fp,"#!/usr/bin/env python\n");
  fprintf(fp,"# -*- coding:UTF-8 -*-\n\n");
  fprintf(fp,"import paramiko\n\n");

  fprintf(fp,"HOST = \'%s\'\n", SOSS_HOSTNAME);
  fprintf(fp,"USER = \'%s\'\n", hg->prop_id);
  fprintf(fp,"PSWD = r\'%s\'\n\n", hg->prop_pass);
  
  fprintf(fp,"LOCAL_PATH = r\"%s\"\n", hg->filename_log);
  fprintf(fp,"REMOTE_PATH = \"/home/%s/hdslog-%04d%02d%02d.txt\"\n\n", 
	  hg->prop_id,
	  hg->fr_year,hg->fr_month,hg->fr_day);

  fprintf(fp,"ssh = paramiko.SSHClient()\n");
  fprintf(fp,"ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())\n");
  fprintf(fp,"ssh.connect(HOST, username=USER, password=PSWD)\n\n");
 
  fprintf(fp,"sftp = ssh.open_sftp()\n");
  fprintf(fp,"sftp.get(REMOTE_PATH,LOCAL_PATH)\n");
  fprintf(fp,"sftp.close()\n\n");
 
  fprintf(fp,"ssh.close()\n");

  fclose(fp);

  cmdline=g_strconcat(PY_COM,
		      " ",
		      sftp_py,
		      " 2> ",
		      sftp_log,
		      NULL);

  ret=system(cmdline);
  g_free(cmdline);

  remove(sftp_py);

  if((fp=fopen(sftp_log,"r"))==NULL){
    popup_message(GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: cannot create temporary log file.",
		  " ",
		  sftp_log,
		  NULL);
    g_free(sftp_py);
    g_free(sftp_log);
    return(-2);
  }
  fseek(fp,0,SEEK_END);
  sz=ftell(fp);

  if(sz>0){
    GtkWidget *dialog, *label, *button, *pixmap, *vbox0, *vbox, *hbox,
      *text, *scr;
    GtkTextBuffer *text_buffer;
    gchar *msg;

    dialog = gtk_dialog_new_with_buttons("HOE : Sftp failed",
					 NULL,
					 GTK_DIALOG_MODAL,
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
					 NULL);

    vbox0 = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox0), 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		       vbox0,FALSE, FALSE, 0);

    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(vbox0),
		       hbox,FALSE, FALSE, 0);
    
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
				     GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);


    vbox = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);
    
    label = gtk_label_new ("Failed to download via sftp!!");
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    msg=g_strconcat("   Host : ",SOSS_HOSTNAME, NULL);
    label = gtk_label_new (msg);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    g_free(msg);

    msg=g_strconcat("   User : ",hg->prop_id, NULL);
    label = gtk_label_new (msg);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    g_free(msg);

    msg=g_strconcat("   Password : ",hg->prop_pass, NULL);
    label = gtk_label_new (msg);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    g_free(msg);

    label = gtk_label_new (" =================== <error message> =================== ");
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox0),label,FALSE, FALSE, 0);

    scr = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scr),
				   GTK_POLICY_AUTOMATIC, 
				   GTK_POLICY_AUTOMATIC);

    text_buffer = gtk_text_buffer_new(NULL);

    text = gtk_text_view_new_with_buffer (text_buffer);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text), FALSE);
    
    gtk_container_add(GTK_CONTAINER(scr), text);
    
    gtk_box_pack_start(GTK_BOX(vbox0),scr,TRUE, TRUE, 0);


    fseek(fp,0,SEEK_SET);
    while(fgets(buf,BUF_LEN-1,fp)!=NULL){
	//fprintf(stderr," : %s",buf);
      gtk_text_buffer_insert_at_cursor(text_buffer,
				       buf,
				       strlen(buf));
    }

    gtk_widget_set_usize(scr,480,200);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      gtk_widget_destroy(dialog);
    }

    remove(sftp_log);
    remove(hg->filename_log);
    g_free(sftp_py);
    g_free(sftp_log);
    return(-3);
  }
  fclose(fp);

  g_free(sftp_py);
  g_free(sftp_log);
  remove(sftp_log);
  
  popup_message(GTK_STOCK_OK, POPUP_TIMEOUT*5,
		"The LOG file has been successfully downloaded.",
		NULL);

  return(0);
}


int ftp_c(typHOE *hg, gchar *user, gchar *passwd, gchar *host, gchar *opefile, gchar *path){
    int command_socket;           /* コマンド用ソケット */
    int data_socket;              /* データ用ソケット */
    //int data_waiting_socket;          /* データコネクションの待ち受け用ソケット */
    struct hostent *servhost;         /* ホスト名とIPアドレスを扱うための構造体 */
    struct sockaddr_in server;        /* ソケットを扱うための構造体 */
    struct sockaddr_in sin;
    int len, result, size;

    struct sockaddr_in trans_address ;
    
    char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
    char buf[BUF_LEN+1];

    FILE *fp_read;
    char ip[20];
    int i1 ,i2 ,i3 ,i4 , i5 , i6 , sport ;

                                /* ホストの情報 (IP アドレスなど) を取得 */
    servhost = gethostbyname(host);
    if ( servhost == NULL ){
        fprintf(stderr, "Bad hostname [%s]\n", host);
        exit(1);
    }

                                /* IP アドレスを示す構造体をコピー */
    bzero((char*)&server, sizeof(server));
    server.sin_family = AF_INET;
                /* 構造体をゼロクリア */
    //bcopy(servhost->h_addr, (char *)&server.sin_addr, servhost->h_length);
    memmove((char *)&server.sin_addr, servhost->h_addr, servhost->h_length);

                /* ポート番号取得 */
    server.sin_port = (getservbyname("ftp", "tcp"))->s_port;
                /* ソケット生成 */
    command_socket = socket(AF_INET, SOCK_STREAM, 0);

                /* サーバに接続 */
    connect(command_socket, (struct sockaddr *)&server, sizeof(server));

                /* welcome response を取得 */
    read_response(command_socket, buf);

                /* USER・PASS を送信 */
    sprintf(send_mesg, "USER %s\n", user);
    write_to_server(command_socket, send_mesg);
    read_response(command_socket, buf);

    sprintf(send_mesg, "PASS %s\n", passwd);
    write_to_server(command_socket, send_mesg);
    read_response(command_socket, buf);

    if(buf[0]=='5' && buf[1]=='3' && buf[2]=='0'){
      write_to_server(command_socket, "QUIT\n");
      read_response(command_socket, buf);

#ifdef USE_WIN32
      closesocket(command_socket);
#else
      close(command_socket);
#endif

      return -1;
    }

 
                /* データコネクション用ソケットを作成し、
                 * bind・listen する
                 */
    /*
    data_waiting_socket = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if ( bind(data_waiting_socket, (struct sockaddr *)&sin, sizeof sin) < 0 ){
        error("bind failed.\n");
    }
    if ( listen(data_waiting_socket, SOMAXCONN) == -1 ){
        error("listen failed.\n");
    }
    */
    /* まだ accept はしない。PORT・LIST を送ってから */


    /* ----------------------------------------- */
    {
        u_long local_ip;

        /* localhost の IP アドレスを取得。既に ESTABLISHED である
         * command_socket から取得していることに注意。
         */

        len = sizeof(sin);
        if ( getsockname(command_socket,
                         (struct sockaddr *)&sin, &len) < 0 ){
            error("getsockname failed.\n");
        }
        local_ip = ntohl(sin.sin_addr.s_addr);

                /* ポート番号を取得 */
	/*
        if ( getsockname(data_waiting_socket,
                         (struct sockaddr *)&sin, &len) < 0 ){
            error("getsockname failed.\n");
        }
	*/

        sprintf(send_mesg, "PORT %d,%d,%d,%d,%d,%d\n",
                (int)(local_ip >> 24) & 0xff,
                (int)(local_ip >> 16) & 0xff,
                (int)(local_ip >>  8) & 0xff,
                (int)(local_ip)       & 0xff,
                /*
                 * ↑は inet_ntoa(local_ip) でもいいんだけど、
                 * その場合はピリオドをカンマに変換しないといけない。
                 */
                (ntohs(sin.sin_port) >>  8) & 0xff,
                 ntohs(sin.sin_port)        & 0xff);

                /* PORT・RETR を送信 */
        write_to_server(command_socket, send_mesg);
        read_response(command_socket, buf);

	// ~/Procedure/ にディレクトリを変更
        sprintf(send_mesg, "CWD %s\n", path);
        write_to_server(command_socket, send_mesg);

	read_response(command_socket, buf);

	if(buf[0]=='5' && buf[1]=='5' && buf[2]=='0'){
	  write_to_server(command_socket, "QUIT\n");
	  read_response(command_socket, buf);
      
#ifdef USE_WIN32
	  closesocket(command_socket);
#else
	  close(command_socket);
#endif
	  return -2;  // No such file or Directory
	}


	// ASCII mode
        sprintf(send_mesg, "TYPE A\n");
        write_to_server(command_socket, send_mesg);

	read_response(command_socket, buf);

	// PASV mode
        sprintf(send_mesg, "PASV\n");
        write_to_server(command_socket, send_mesg);

	read_response(command_socket, buf);
	PortReq(buf,&i1,&i2,&i3,&i4,&i5,&i6);
	memset(ip,'\0',20);
	sprintf(ip,"%d.%d.%d.%d",i1,i2,i3,i4);
	sport = i5 * 256 + i6 ;


	// OPE fileをput
        sprintf(send_mesg, "STOR %s\n", g_path_get_basename(opefile));
        write_to_server(command_socket, send_mesg);
    }


    /* ファイルの内容送信 */
    if((fp_read=fopen(opefile,"r"))==NULL){
      fprintf(stderr," File Read Error  \"%s\" \n", opefile);
      exit(1);
    }

    data_socket = socket(AF_INET,SOCK_STREAM,0);

#ifdef USE_WIN32
    {
      long x=0;
      setsockopt(data_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&x, sizeof(x));
      setsockopt(data_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&x, sizeof(x));
    }
#endif

    trans_address.sin_family = AF_INET ;
    trans_address.sin_addr.s_addr = inet_addr(ip);
    trans_address.sin_port = htons(sport) ;
    len = sizeof(trans_address);

    result = connect(data_socket , (struct sockaddr *)&trans_address, len);
    if ( result == -1 )
      {
	fprintf(stderr,"Failed Connect ftp-server\n");
	exit(1);
      }
    
    memset(buf,'\0',BUF_LEN);
    
    /*
    while (!feof(fp_read)){
      if(fgets(buf,BUF_LEN-1,fp_read)){
	write(data_socket, buf, BUF_LEN);
      }
      else{
	break;
      }
    }
    */
    while((size = fread(buf,1,BUF_LEN,fp_read)) > 0 )
      {
        fd_write(data_socket,buf,size);
      }  
    fclose(fp_read);
#ifdef USE_WIN32
    closesocket(data_socket);
#else
    close(data_socket);
#endif

    /* データコネクションの確立  
    len = sizeof(sin);
    data_socket = accept(data_waiting_socket, (struct sockaddr *)&sin, &len);
    if ( data_socket == -1 ){
        error("accept failed.\n");
    }
    
    

    while (!feof(fp_read)){
      if(fgets(buf,BUF_LEN-1,fp_read)){
	write(data_socket, buf, BUF_LEN);
      }
      else{
	break;
      }
    }
    */
    /*
    { 
        int read_size;
        read_size = read(data_socket, buf, BUF_LEN);
        if ( read_size > 0 ){
            write(1, buf, read_size);
        } else {
            break;
        }
    }
    */
                /* 150 Opening ASCII mode data connection ... 
                 * のようなレスポンスを受け取る
                 */
    read_response(command_socket, buf);
    
    if(buf[0]=='5' && buf[1]=='5' && buf[2]=='3'){
      write_to_server(command_socket, "QUIT\n");
      read_response(command_socket, buf);
      
#ifdef USE_WIN32
      closesocket(command_socket);
#else
      close(command_socket);
#endif
      return -3;  // Permission Denied
    }

                /* 226 Transfer complete. のようなレスポンスを受け取る */
    read_response(command_socket, buf);

    // ascii mode
    sprintf(send_mesg, "TYPE A\n");
    write_to_server(command_socket, send_mesg);
    
    read_response(command_socket, buf);

                /* QUIT 送って終了 */
    write_to_server(command_socket, "QUIT\n");
    read_response(command_socket, buf);

    //close(data_waiting_socket);
#ifdef USE_WIN32
    closesocket(command_socket);
#else
    close(command_socket);
#endif

    return 0;
}


#ifdef USE_WIN32
unsigned __stdcall http_c_fc(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_fc(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  
  gchar *tmp_file=NULL;

  gdouble ra_0, dec_0;
  gchar tmp[2048], tmp_scale[10];
  gfloat sdss_scale=SDSS_SCALE;
  gint xpix,ypix,i_bin;
  //struct ln_hms ra_hms;
  //struct ln_dms dec_dms;
  static char cbuf[BUFFSIZE];
  gchar *cp, *cpp, *cp2, *cp3=NULL;
  FILE *fp_read;

  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  const char *cause=NULL;

  gboolean chunked_flag=FALSE;

   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->dss_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  // bin mode
  ra_0=hg->obj[hg->dss_i].ra;
  hobject.ra.hours=(gint)(ra_0/10000);
  ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
  hobject.ra.minutes=(gint)(ra_0/100);
  hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;
 
  if(hg->obj[hg->dss_i].dec<0){
    hobject.dec.neg=1;
    dec_0=-hg->obj[hg->dss_i].dec;
  }
  else{
    hobject.dec.neg=0;
    dec_0=hg->obj[hg->dss_i].dec;
  }
  hobject.dec.degrees=(gint)(dec_0/10000);
  dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
  hobject.dec.minutes=(gint)(dec_0/100);
  hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;


  ln_hequ_to_equ (&hobject, &object);
  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_epoch(hg->obj[hg->dss_i].epoch),
		    JD2000, &object_prec);
  ln_equ_to_hequ (&object_prec, &hobject_prec);


  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
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
    switch(hg->dss_scale){
    case FC_SCALE_LOG:
      sprintf(tmp_scale,"Log");
      break;
    case FC_SCALE_SQRT:
      sprintf(tmp_scale,"Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      sprintf(tmp_scale,"HistEq");
      break;
    case FC_SCALE_LOGLOG:
      sprintf(tmp_scale,"LogLog");
      break;
    default:
      sprintf(tmp_scale,"Linear");
    }
    sprintf(tmp,hg->dss_path,
	    hg->dss_src, hg->obj[hg->dss_i].epoch,
	    tmp_scale,
	    //	    (hg->dss_log) ? "Log" : "Linear",
	    (hg->dss_invert) ? "&invert=on&" : "&",
	    (gdouble)hg->dss_arcmin/60.,
	    (gdouble)hg->dss_arcmin/60.,
	    hg->dss_pix,
	    ln_hms_to_deg(&hobject_prec.ra),
	    ln_dms_to_deg(&hobject_prec.dec));
    break;

  case FC_SDSS:
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1000)||(ypix>1000));
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
	    ln_hms_to_deg(&hobject_prec.ra),
	    ln_dms_to_deg(&hobject_prec.dec),
	    hg->dss_arcmin*240);
    break;

  }

  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
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

  switch(hg->fc_mode){
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
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
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do { // data read
      size = recv(command_socket, buf, BUF_LEN, 0);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size>0);

    fclose(fp_write);

#ifndef USE_WIN32
    if((chmod(hg->dss_tmp,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_tmp);
  }
#endif

    if(chunked_flag) unchunk(hg->dss_tmp);
    
    fp_read=fopen(hg->dss_tmp,"r");
    
    switch(hg->fc_mode){
    case FC_ESO_DSS1R:
    case FC_ESO_DSS2R:
    case FC_ESO_DSS2B:
    case FC_ESO_DSS2IR:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  if(NULL != (cp = strstr(cpp, "<A HREF="))){
	    cpp=cp+strlen("<A HREF=");
	    
	    if(NULL != (cp2 = strstr(cp, ">"))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
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
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = strstr(cpp, "x['_output']='../.."))){
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
	  
	  if(NULL != (cp = strstr(cpp, "<img src=\"//" FC_HOST_PANCOL))){
	    cpp=cp+strlen("<img src=\"//" FC_HOST_PANCOL);
	    
	    if(NULL != (cp2 = strstr(cp, "\" width="))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    }
    
    fclose(fp_read);
    
    close(command_socket);

    chunked_flag=FALSE;
    
    /* サーバに接続 */
    if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }
    if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
      fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
      return(HSKYMON_HTTP_ERROR_CONNECT);
    }

    // AddrInfoの解放
    freeaddrinfo(res);

    if(cp3){
      switch(hg->fc_mode){
      case FC_ESO_DSS1R:
      case FC_ESO_DSS2R:
      case FC_ESO_DSS2B:
      case FC_ESO_DSS2IR:
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
	sprintf(send_mesg, "GET %s.jpg HTTP/1.1\r\n", cp3);
	break;
      }

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
  
      if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
	fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
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
      }
      do { // data read
	size = recv(command_socket, buf, BUF_LEN, 0);
	fwrite( &buf , size , 1 , fp_write ); 
      }while(size>0);
      
      
      fclose(fp_write);
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
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do { // data read
      size = recv(command_socket, buf, BUF_LEN, 0);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size>0);
    
    
    fclose(fp_write);
    if ( debug_flg ){
      fprintf(stderr," Done.\n");
    }
    
    break;
  }

  if(chunked_flag) unchunk(hg->dss_file);

#ifndef USE_WIN32
    if((chmod(hg->dss_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_file);
  }
#endif
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}

#ifdef USE_SSL
#ifdef USE_WIN32
unsigned __stdcall http_c_fc_ssl(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_fc_ssl(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  
  gchar *tmp_file=NULL;

  gdouble ra_0, dec_0;
  gchar tmp[2048], tmp_scale[10];
  gfloat sdss_scale=SDSS_SCALE;
  gint xpix,ypix,i_bin;
  //struct ln_hms ra_hms;
  //struct ln_dms dec_dms;
  static char cbuf[BUFFSIZE];
  gchar *cp, *cpp, *cp2, *cp3=NULL;
  FILE *fp_read;

  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  const char *cause=NULL;

  gboolean chunked_flag=FALSE;

  SSL *ssl;
  SSL_CTX *ctx;

   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->dss_host, "https", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  ssl = SSL_new(ctx);
  err = SSL_set_fd(ssl, command_socket);
  if( SSL_connect(ssl) !=1) {
    fprintf(stderr, "SSL connection failed.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SSL);
  }
  
  // bin mode
  ra_0=hg->obj[hg->dss_i].ra;
  hobject.ra.hours=(gint)(ra_0/10000);
  ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
  hobject.ra.minutes=(gint)(ra_0/100);
  hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;
 
  if(hg->obj[hg->dss_i].dec<0){
    hobject.dec.neg=1;
    dec_0=-hg->obj[hg->dss_i].dec;
  }
  else{
    hobject.dec.neg=0;
    dec_0=hg->obj[hg->dss_i].dec;
  }
  hobject.dec.degrees=(gint)(dec_0/10000);
  dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
  hobject.dec.minutes=(gint)(dec_0/100);
  hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;


  ln_hequ_to_equ (&hobject, &object);
  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_epoch(hg->obj[hg->dss_i].epoch),
		    JD2000, &object_prec);
  ln_equ_to_hequ (&object_prec, &hobject_prec);


  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
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
    switch(hg->dss_scale){
    case FC_SCALE_LOG:
      sprintf(tmp_scale,"Log");
      break;
    case FC_SCALE_SQRT:
      sprintf(tmp_scale,"Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      sprintf(tmp_scale,"HistEq");
      break;
    case FC_SCALE_LOGLOG:
      sprintf(tmp_scale,"LogLog");
      break;
    default:
      sprintf(tmp_scale,"Linear");
    }
    sprintf(tmp,hg->dss_path,
	    hg->dss_src, hg->obj[hg->dss_i].epoch,
	    tmp_scale,
	    //	    (hg->dss_log) ? "Log" : "Linear",
	    (hg->dss_invert) ? "&invert=on&" : "&",
	    (gdouble)hg->dss_arcmin/60.,
	    (gdouble)hg->dss_arcmin/60.,
	    hg->dss_pix,
	    ln_hms_to_deg(&hobject_prec.ra),
	    ln_dms_to_deg(&hobject_prec.dec));
    break;


  case FC_SDSS:
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1000)||(ypix>1000));
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
	    ln_hms_to_deg(&hobject_prec.ra),
	    ln_dms_to_deg(&hobject_prec.dec),
	    hg->dss_arcmin*240);
    break;

  }

  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
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

  switch(hg->fc_mode){
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
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
    
    while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"[SSL] --> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do{ // data read
      size = SSL_read(ssl, buf, BUF_LEN);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size >0);
    

    fclose(fp_write);

#ifndef USE_WIN32
    if((chmod(hg->dss_tmp,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_tmp);
  }
#endif

    if(chunked_flag) unchunk(hg->dss_tmp);
    
    fp_read=fopen(hg->dss_tmp,"r");
    
    switch(hg->fc_mode){
    case FC_ESO_DSS1R:
    case FC_ESO_DSS2R:
    case FC_ESO_DSS2B:
    case FC_ESO_DSS2IR:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  if(NULL != (cp = strstr(cpp, "<A HREF="))){
	    cpp=cp+strlen("<A HREF=");
	    
	    if(NULL != (cp2 = strstr(cp, ">"))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
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
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = strstr(cpp, "x['_output']='../.."))){
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
	  
	  if(NULL != (cp = strstr(cpp, "<img src=\"//" FC_HOST_PANCOL))){
	    cpp=cp+strlen("<img src=\"//" FC_HOST_PANCOL);
	    
	    if(NULL != (cp2 = strstr(cp, "\" width="))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    }
    
    fclose(fp_read);
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    close(command_socket);

    chunked_flag=FALSE;
    
    /* サーバに接続 */
    if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }
    if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
      fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
      return(HSKYMON_HTTP_ERROR_CONNECT);
    }

    // AddrInfoの解放
    freeaddrinfo(res);

    SSL_load_error_strings();
    SSL_library_init();

    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, command_socket);
    if( SSL_connect(ssl) !=1) {
      fprintf(stderr, "SSL connection failed.\n");
#ifdef USE_WIN32
      gtk_main_quit();
      _endthreadex(0);
#endif
      return(HSKYMON_HTTP_ERROR_SSL);
    }
    
    if(cp3){
      switch(hg->fc_mode){
      case FC_ESO_DSS1R:
      case FC_ESO_DSS2R:
      case FC_ESO_DSS2B:
      case FC_ESO_DSS2IR:
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
	sprintf(send_mesg, "GET %s.jpg HTTP/1.1\r\n", cp3);
	break;
      }
      
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
  
      if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
	fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
	return(HSKYMON_HTTP_ERROR_TEMPFILE);
      }

      while((size = ssl_gets(ssl, buf,BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"[SSL] --> Header: %s", buf);
	}
	if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
      }
      do{ // data read
	size = SSL_read(ssl, buf, BUF_LEN);
	fwrite( &buf , size , 1 , fp_write ); 
      }while(size >0);
      
      fclose(fp_write);
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
    
    while((size = ssl_gets(ssl, buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do{ // data read
      size = SSL_read(ssl, buf, BUF_LEN);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size >0);
    
    fclose(fp_write);
    if ( debug_flg ){
      fprintf(stderr," Done.\n");
    }
    
    break;
  }
  
  if(chunked_flag) unchunk(hg->dss_file);

#ifndef USE_WIN32
  if((chmod(hg->dss_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_file);
  }
#endif
  
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  ERR_free_strings();
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}
#endif  //USE_SSL

int get_dss(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;
  unsigned int dwThreadID;
  HANDLE hThread;
  
#ifdef USE_SSL
  if((hg->fc_mode<FC_SKYVIEW_GALEXF)||(hg->fc_mode>FC_SKYVIEW_RGB)){
    hThread = (HANDLE)_beginthreadex(NULL,0,
				     http_c_fc,
				     (LPVOID)hg, 0, &dwThreadID);
  }
  else{
    hThread = (HANDLE)_beginthreadex(NULL,0,
				     http_c_fc_ssl,
				     (LPVOID)hg, 0, &dwThreadID);
  }
#else
  hThread = (HANDLE)_beginthreadex(NULL,0,
				   http_c_fc,
				   (LPVOID)hg, 0, &dwThreadID);
#endif
  if (hThread == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hThread);
  }

#else
  waitpid(fc_pid,0,WNOHANG);

  if( (fc_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(fc_pid ==0) {
#ifdef USE_SSL
    if((hg->fc_mode<FC_SKYVIEW_GALEXF)||(hg->fc_mode>FC_SKYVIEW_WISE22)){
      http_c_fc(hg);
    }
    else{
      http_c_fc_ssl(hg);
    }
#else
    http_c_fc(hg);
#endif
    kill(getppid(), SIGUSR1);  //calling dss_signal
    _exit(1);
  }
#endif

  return 0;
}


void unchunk(gchar *dss_tmp){
   FILE *fp_read, *fp_write;
   gchar *unchunk_tmp;
   static char cbuf[BUFFSIZE];
   gchar *cpp;
   gchar *chunkptr, *endptr;
   long chunk_size;
   gint i, read_size=0, crlf_size=0;

   if ( debug_flg ){
     fprintf(stderr, "Decoding chunked file \"%s\".\n", dss_tmp);fflush(stderr);
   }

   fp_read=fopen(dss_tmp,"r");
   unchunk_tmp=g_strconcat(dss_tmp,"_unchunked",NULL);
   fp_write=fopen(unchunk_tmp,"w");

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
       if(chunk_size>BUFFSIZE-crlf_size){
	 fprintf(stderr, "!!! Buffer size overflow. Stopped to convert\"%s\".\n", dss_tmp);
	 fflush(stderr);
	 break;
       }


       if(fread(cbuf,1, chunk_size+crlf_size, fp_read)){
	 cpp=cbuf;
	 fwrite( &cbuf , chunk_size , 1 , fp_write ); 
       }
       else{
	 break;
       }
     }
   }

   fclose(fp_read);
   fclose(fp_write);

   unlink(dss_tmp);

   rename(unchunk_tmp,dss_tmp);

   g_free(unchunk_tmp);
 }


#ifdef USE_SSL
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
#endif

#ifdef USE_SSL
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
#endif

/* peek at the socket data without actually reading it */
#ifdef USE_SSL
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
#endif

#ifdef USE_SSL
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
#endif

