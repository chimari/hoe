//    HDS OPE file Editor
//      scp-client.c : scp client for Gen2 sumda
//      imported from example codes of libssh2 1.4.3
//                                           2018.2.27  A.Tajitsu

#include "main.h"
#include "config.h"

#ifdef USE_SSL
#include "libssh2_config.h"
#include <libssh2.h>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <sys/param.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#ifndef USE_WIN32
#include <netdb.h>
#endif

#ifdef USE_WIN32
#define BUF_LEN 65535             /* バッファのサイズ */
#else
#define BUF_LEN 1023             /* バッファのサイズ */
#endif

int scp_w_main();
int scp_g_main();

int scp_write(typHOE *hg){
  int ret;
  FILE *fp;
  gchar *sftp_log;
  GtkWidget *dialog, *label, *button, *pixmap, *vbox0, *vbox, *hbox,
    *text, *scr;
  GtkTextBuffer *text_buffer;
  gchar *msg;
  gchar buf[BUF_LEN];

  ret=scp_w_main(hg);

  sftp_log=g_strconcat(hg->temp_dir,
		       G_DIR_SEPARATOR_S,
		       SFTP_LOG,
		       NULL);

  if(ret<0){
    if((fp=fopen(sftp_log,"r"))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: cannot read a temporary_file for scp log.",
		    " ",
		    sftp_log,
		    NULL);
      g_free(sftp_log);
      return(-1);
    }

    dialog = gtk_dialog_new_with_buttons("HOE : Sftp failed",
					 GTK_WINDOW(hg->w_top),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);

    vbox0 = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox0), 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       vbox0,FALSE, FALSE, 0);
    
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(vbox0),
		       hbox,FALSE, FALSE, 0);
    
#ifdef USE_GTK3
    pixmap=gtk_image_new_from_icon_name ("dialog-warning",
					 GTK_ICON_SIZE_DIALOG);
#else
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
				     GTK_ICON_SIZE_DIALOG);
#endif
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


    while(fgets(buf,BUF_LEN-1,fp)!=NULL){
      gtk_text_buffer_insert_at_cursor(text_buffer,
				       buf,
				       strlen(buf));
    }
    
    gtk_widget_set_size_request(scr,480,200);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      gtk_widget_destroy(dialog);
    }
    
    fclose(fp);
    unlink(sftp_log);
    if(sftp_log) g_free(sftp_log);
    return(-1);
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "network-transmit", 
#else
		  GTK_STOCK_OK,
#endif
		  POPUP_TIMEOUT,
		  "The OPE file has been successfully uploaded to Gen2.",
		  NULL);
    unlink(sftp_log);
    if(sftp_log) g_free(sftp_log);
    return(0);
  }
}


int scp_get(typHOE *hg){
  int ret;
  FILE *fp;
  gchar *sftp_log;
  GtkWidget *dialog, *label, *button, *pixmap, *vbox0, *vbox, *hbox,
    *text, *scr;
  GtkTextBuffer *text_buffer;
  gchar *msg;
  gchar buf[BUF_LEN];

  ret=scp_g_main(hg);

  sftp_log=g_strconcat(hg->temp_dir,
		       G_DIR_SEPARATOR_S,
		       SFTP_LOG,
		       NULL);

  if(ret<0){
    if((fp=fopen(sftp_log,"r"))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: cannot read a temporary_file for scp log.",
		    " ",
		    sftp_log,
		    NULL);
      g_free(sftp_log);
      return(-1);
    }

    dialog = gtk_dialog_new_with_buttons("HOE : Sftp failed",
					 GTK_WINDOW(hg->w_top),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);

    vbox0 = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox0), 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       vbox0,FALSE, FALSE, 0);
    
    hbox = gtk_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(vbox0),
		       hbox,FALSE, FALSE, 0);
    
#ifdef USE_GTK3
    pixmap=gtk_image_new_from_icon_name ("dialog-warning",
					 GTK_ICON_SIZE_DIALOG);
#else
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
				     GTK_ICON_SIZE_DIALOG);
#endif
    gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);
    
    
    vbox = gtk_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);
    
    label = gtk_label_new ("Failed to dwonload via sftp!!");
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


    while(fgets(buf,BUF_LEN-1,fp)!=NULL){
      gtk_text_buffer_insert_at_cursor(text_buffer,
				       buf,
				       strlen(buf));
    }
    
    gtk_widget_set_size_request(scr,480,200);
    
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      gtk_widget_destroy(dialog);
    }
    
    fclose(fp);
    unlink(sftp_log);
    if(sftp_log) g_free(sftp_log);
    return(-1);
  }
  else{
    popup_message(hg->w_top, 
#ifdef USE_GTK3
		  "network-receive", 
#else
		  GTK_STOCK_OK, 
#endif
		  POPUP_TIMEOUT,
		  "The Log file has been successfully downloaded from Gen2.",
		  NULL);
    unlink(sftp_log);
    if(sftp_log) g_free(sftp_log);
    return(0);
  }
}

int scp_w_main(typHOE *hg)
{
    struct hostent *servhost;         /* ホスト名とIPアドレスを扱うための構造体 */
    int sock, i, auth_pw = 1;
    struct sockaddr_in sin;
    const char *fingerprint;
    LIBSSH2_SESSION *session = NULL;
    LIBSSH2_CHANNEL *channel;
    gchar *username=g_strdup(hg->prop_id);
    gchar *password=g_strdup(hg->prop_pass);
    gchar *loclfile=g_strdup(hg->filename_read);
    gchar *scppath=g_strconcat("/home/",hg->prop_id,
			       G_DIR_SEPARATOR_S,SOSS_PATH,
			       G_DIR_SEPARATOR_S,
			       g_path_get_basename(hg->filename_read),
			       NULL);
    FILE *local;
    int rc;
    char mem[1024];
    size_t nread;
    char *ptr;
    struct stat fileinfo;
    gboolean err_flag=FALSE;

#ifdef USE_WIN32
    WSADATA wsadata;
    int err;
#endif
    FILE *fp;
    gchar *sftp_log;
  
    sftp_log=g_strconcat(hg->temp_dir,
			 G_DIR_SEPARATOR_S,
			 SFTP_LOG,
			 NULL);
    
    if((fp=fopen(sftp_log,"w"))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: cannot create a temporary_file for scp log.",
		    " ",
		    sftp_log,
		    NULL);
      g_free(sftp_log);
      return(-1);
    }
    g_free(sftp_log);


#ifdef USE_WIN32
    err = WSAStartup(MAKEWORD(2,0), &wsadata);
    if (err != 0) {
        fprintf(fp, "WSAStartup failed with error: %d\n", err);
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }
#endif

    servhost = gethostbyname(SOSS_HOSTNAME);
    if ( servhost == NULL ){
        fprintf(fp, "Bad hostname [%s]\n", SOSS_HOSTNAME);
	err_flag=TRUE;
	fclose(fp);
	return -1;
    }
    memcpy(&sin.sin_addr, servhost->h_addr_list[0], servhost->h_length);

    rc = libssh2_init (0);
    if (rc != 0) {
        fprintf (fp, "libssh2 initialization failed (%d)\n", rc);
	fclose(fp);
        return -1;
    }

    local = fopen(loclfile, "rb");
    if (!local) {
        fprintf(fp, "Can't open local file %s\n", loclfile);
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }

    stat(loclfile, &fileinfo);

    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sock) {
        fprintf(fp, "failed to create socket!\n");
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    //sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),
            sizeof(struct sockaddr_in)) != 0) {
        fprintf(fp, "failed to connect!\n");
	fclose(fp);
        return -1;
    }

    /* Create a session instance
     */
    session = libssh2_session_init();
    if(!session){
      fprintf(fp, "Failed to initialize SSH session.\n");
      err_flag=TRUE;
      fclose(fp);
      return -1;
    }
    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    rc = libssh2_session_handshake(session, sock);
    if(rc) {
        fprintf(fp, "Failure establishing SSH session: %d\n", rc);
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }

    /* At this point we havn't yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     */
    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    fprintf(fp, "Fingerprint: ");
    for(i = 0; i < 20; i++) {
        fprintf(fp, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(fp, "\n");

    if (auth_pw) {
        /* We could authenticate via password */
        if (libssh2_userauth_password(session, username, password)) {
            fprintf(fp, "Authentication by password failed.\n");
	    err_flag=TRUE;
            goto shutdown;
        }
    } else {
        /* Or by public key */
        if (libssh2_userauth_publickey_fromfile(session, username,
                            "/home/username/.ssh/id_rsa.pub",
                            "/home/username/.ssh/id_rsa",
                            password)) {
            fprintf(fp, "\tAuthentication by public key failed\n");
	    err_flag=TRUE;
            goto shutdown;
        }
    }

    /* Send a file via scp. The mode parameter must only have permissions! */
    channel = libssh2_scp_send(session, scppath, fileinfo.st_mode & 0777,
                               (unsigned long)fileinfo.st_size);

    if (!channel) {
        char *errmsg;
        int errlen;
        int err = libssh2_session_last_error(session, &errmsg, &errlen, 0);
        fprintf(fp, "Unable to open a session: (%d) %s\n", err, errmsg);
	err_flag=TRUE;
        goto shutdown;
    }

    fprintf(fp, "SCP session waiting to send file\n");
    do {
        nread = fread(mem, 1, sizeof(mem), local);
        if (nread <= 0) {
            /* end of file */
            break;
        }
        ptr = mem;

        do {
            /* write the same data over and over, until error or completion */
            rc = libssh2_channel_write(channel, ptr, nread);
            if (rc < 0) {
                fprintf(fp, "ERROR %d\n", rc);
                break;
            }
            else {
                /* rc indicates how many bytes were written this time */
                ptr += rc;
                nread -= rc;
            }
        } while (nread);

    } while (1);

    fprintf(fp, "Sending EOF\n");
    libssh2_channel_send_eof(channel);

    fprintf(fp, "Waiting for EOF\n");
    libssh2_channel_wait_eof(channel);

    fprintf(fp, "Waiting for channel to close\n");
    libssh2_channel_wait_closed(channel);

    libssh2_channel_free(channel);
    channel = NULL;

 shutdown:

    if(session) {
        libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
        libssh2_session_free(session);
    }
#ifdef USE_WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    if (local)
        fclose(local);
    fprintf(fp, "all done\n");
    fclose(fp);

    libssh2_exit();

    if(username) g_free(username);
    if(password) g_free(password);
    if(loclfile) g_free(loclfile);
    if(scppath) g_free(scppath);
    
    return ((err_flag)? (-2) : (0));
}


int scp_g_main(typHOE *hg)
{
    struct hostent *servhost;         /* ホスト名とIPアドレスを扱うための構造体 */
    int sock, i, auth_pw = 1;
    struct sockaddr_in sin;
    const char *fingerprint;
    LIBSSH2_SESSION *session = NULL;
    LIBSSH2_CHANNEL *channel;
    gchar *username=g_strdup(hg->prop_id);
    gchar *password=g_strdup(hg->prop_pass);
    gchar *scppath=g_strdup_printf("/home/%s/hdslog-%04d%02d%02d.txt", 
				   hg->prop_id,
				   hg->fr_year,hg->fr_month,hg->fr_day);
    FILE *local;
    int rc;
    char mem[1024];
    size_t nread;
    char *ptr;
    struct stat fileinfo;
    gboolean err_flag=FALSE;
    off_t got=0;

#ifdef USE_WIN32
    WSADATA wsadata;
    int err;
#endif
    FILE *fp;
    gchar *sftp_log;
  
    sftp_log=g_strconcat(hg->temp_dir,
			 G_DIR_SEPARATOR_S,
			 SFTP_LOG,
			 NULL);
    
    if((fp=fopen(sftp_log,"w"))==NULL){
      popup_message(hg->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    POPUP_TIMEOUT*2,
		    "Error: cannot create a temporary_file for scp log.",
		    " ",
		    sftp_log,
		    NULL);
      g_free(sftp_log);
      return(-1);
    }
    g_free(sftp_log);


#ifdef USE_WIN32
    err = WSAStartup(MAKEWORD(2,0), &wsadata);
    if (err != 0) {
        fprintf(fp, "WSAStartup failed with error: %d\n", err);
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }
#endif

    servhost = gethostbyname(SOSS_HOSTNAME);
    if ( servhost == NULL ){
        fprintf(fp, "Bad hostname [%s]\n", SOSS_HOSTNAME);
	err_flag=TRUE;
	fclose(fp);
	return -1;
    }
    memcpy(&sin.sin_addr, servhost->h_addr_list[0], servhost->h_length);

    rc = libssh2_init (0);
    if (rc != 0) {
        fprintf (fp, "libssh2 initialization failed (%d)\n", rc);
	fclose(fp);
        return -1;
    }

    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sock) {
        fprintf(fp, "failed to create socket!\n");
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    //sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),
            sizeof(struct sockaddr_in)) != 0) {
        fprintf(fp, "failed to connect!\n");
	fclose(fp);
        return -1;
    }

    /* Create a session instance
     */
    session = libssh2_session_init();
    if(!session){
      fprintf(fp, "Failed to initialize SSH session.\n");
      err_flag=TRUE;
      fclose(fp);
      return -1;
    }
    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    rc = libssh2_session_handshake(session, sock);
    if(rc) {
        fprintf(fp, "Failure establishing SSH session: %d\n", rc);
	err_flag=TRUE;
	fclose(fp);
        return -1;
    }

    /* At this point we havn't yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     */
    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    fprintf(fp, "Fingerprint: ");
    for(i = 0; i < 20; i++) {
        fprintf(fp, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(fp, "\n");

    if (auth_pw) {
        /* We could authenticate via password */
        if (libssh2_userauth_password(session, username, password)) {
            fprintf(fp, "Authentication by password failed.\n");
	    err_flag=TRUE;
            goto shutdown;
        }
    } else {
        /* Or by public key */
        if (libssh2_userauth_publickey_fromfile(session, username,
                            "/home/username/.ssh/id_rsa.pub",
                            "/home/username/.ssh/id_rsa",
                            password)) {
            fprintf(fp, "\tAuthentication by public key failed\n");
	    err_flag=TRUE;
            goto shutdown;
        }
    }

    /* Request a file via SCP */
    channel = libssh2_scp_recv(session, scppath, &fileinfo);

    if (!channel) {
        char *errmsg;
        int errlen;
        int err = libssh2_session_last_error(session, &errmsg, &errlen, 0);
        fprintf(fp, "Unable to open a session: (%d) %s\n", err, errmsg);
	err_flag=TRUE;
        goto shutdown;
    }

    if((local=fopen(hg->filename_log,"wb"))==NULL){
      fprintf(fp," File Write Error  \"%s\" \n", hg->filename_log);
      err_flag=TRUE;
      goto shutdown;
    }

    fprintf(fp, "SCP session waiting to get file\n");
    while(got < fileinfo.st_size) {
        char mem[1024];
        int amount=sizeof(mem);

        if((fileinfo.st_size -got) < amount) {
            amount = fileinfo.st_size -got;
        }

        rc = libssh2_channel_read(channel, mem, amount);
        if(rc > 0) {
	  fwrite( &mem , amount , 1 , local ); 
        }
        else if(rc < 0) {
            fprintf(fp, "libssh2_channel_read() failed: %d\n", rc);
            break;
        }
        got += rc;
    }

    fprintf(fp, "Sending EOF\n");
    libssh2_channel_send_eof(channel);

    fprintf(fp, "Waiting for EOF\n");
    libssh2_channel_wait_eof(channel);

    fprintf(fp, "Waiting for channel to close\n");
    libssh2_channel_wait_closed(channel);

    libssh2_channel_free(channel);
    channel = NULL;

 shutdown:

    if(session) {
        libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
        libssh2_session_free(session);
    }
#ifdef USE_WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    if (local)
        fclose(local);
    fprintf(fp, "all done\n");
    fclose(fp);

    libssh2_exit();

    if(username) g_free(username);
    if(password) g_free(password);
    if(scppath) g_free(scppath);
    
    return ((err_flag)? (-2) : (0));
}
#endif // USE_SSL
