#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#define MAX_LINE 4096

void download(int s){
  char buf[MAX_LINE];
  printf("Enter file to download: ");
  fgets(buf, sizeof(buf), stdin);
  short int len = htons(strlen(buf));
  if (send(s, &len, sizeof(len), 0) == -1){
    perror("Client send error\n");
    exit(1);
  }
  if (send(s, buf, sizeof(buf), 0) == -1){
    perror("Client send error\n");
    exit(1);
  }
  int fileSize, l;
  if ((l = recv(s, &fileSize, sizeof(fileSize), 0)) == -1){
    perror("Receive error\n");
    exit(1);
  }
  fileSize = ntohl(fileSize);
  if (fileSize < 0){
    printf("File does not exist on server\n");
  } else {
    FILE *fp;
    buf[strlen(buf) - 1] = '\0';
    if ((fp = fopen(buf, "a")) == NULL){
      perror("Error creating file");
      exit(1);
    }
    printf("%s\n", buf);
    int received_bytes, total_bytes = 0;
    char recv_buf[MAX_LINE];
    while ((received_bytes = recv(s, recv_buf, MAX_LINE, 0)) > 0){
      printf("%s\n", recv_buf);
      fprintf(fp, recv_buf);
      total_bytes += received_bytes;
      printf("%d:%d\n", total_bytes, fileSize);
      if (total_bytes >= fileSize)
        break;
    }
    printf("LLLL\n");
    close(fp);
  }
}

int main(int argc, char * argv[]){
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin, server_addr;
  char *host, *port, *text;
  char in_buf[MAX_LINE], en_buf[MAX_LINE + 1], key_buf[MAX_LINE + 1], buf[MAX_LINE + 1], initial[100] = "DWLD";
  int s, len, addr_len;
  
  if (argc == 3){
    host = argv[1];
    port = argv[2];
  }
  else {
    fprintf(stderr, "usage: ftp-client host port\n");
    exit(1);
  }
  /*fp = fopen(text, "r");
  if (fp != NULL){
    size_t bufLen = fread(buf, sizeof(char), MAX_LINE, fp);
    if (ferror(fp) != 0){
      perror("Error reading file\n");
      exit(1);
    } else {
      buf[bufLen++] = '\0';
    }
  } else {
    strncpy(buf, text, strlen(text));
  }*/

  hp = gethostbyname(host);
  if (!hp){
    fprintf(stderr, "ftp-client: unknown host: %s\n", host);
    exit(1);
  }
  bzero((char*)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(atoi(port));

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    perror("ftp-client: socket\n");
    exit(1);
  }

  if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0){
    perror("ftp-client: connect\n");
    close(s);
    exit(1);
  }
  
  while (1){
    printf("Enter Command: ");
    fgets(in_buf, sizeof(in_buf), stdin);
    len = strlen(in_buf) + 1;
    if (send(s, in_buf, len, 0) == -1){
      perror("Client initial send error\n");
      exit(1);
    }
    if (!strncmp(in_buf, "QUIT", 4))
      break;
    else if (!strncmp(in_buf, "DWLD", 4))
      download(s);
    /*else if (!strncmp(in_buf, "UPLD", 4))
      upload();
    else if (!strncmp(in_buf, "DELF", 4))
      delete_file();
    else if (!strncmp(in_buf, "LIST", 4))
      list();
    else if (!strncmp(in_buf, "MDIR", 4))
      make_directory();
    else if (!strncmp(in_buf, "RDIR", 4))
      remove_directory();
    else if (!strncmp(in_buf, "CDIR", 4))
      change_directory();*/
    else
      printf("Bad Command\n");
  }
  
  close(s);
}
