#include <stdio.h>
#include <string.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";

void parse_uri(char *uri, char *hostname, unsigned int *port, char *query) {
    char *ptr = uri + 7;  // skip characters of http://

    // iterate until port or query section
    while (*ptr != ':' && *ptr != '/') {
        *hostname = *ptr;
        ptr++;
        hostname++;
    }
    *hostname = '\0';  // terminate string

    // get port and query
    if (*ptr == ':') {
        sscanf(ptr + 1, "%d%s", port, query);
    } else {
        *port = 80;  // default 80 if not defined
        strcpy(query, ptr);
    }
}

void create_header_string(char *headers, char *hostname, char *query) {
    char request_hdr[MAXLINE], host_hdr[MAXLINE], std_hdr[MAXLINE];
    
    sprintf(request_hdr, "GET %s HTTP/1.0\r\n", query);
    sprintf(host_hdr, "Host: %s\r\n", hostname);

    // all default value headers
    sprintf(std_hdr, "%s%s%s",
        user_agent_hdr,
        connection_hdr,
        proxy_connection_hdr
    );

    // combine defaults with request and host
    sprintf(headers, "%s%s%s",
        request_hdr,
        host_hdr,
        std_hdr
    );
}

int forward_request(char *uri) {
    int clientfd, serverfd;
    char hostname[MAXLINE], port_str[8], query[MAXLINE];
    char headers[MAXLINE];
    char response[MAXLINE];
    unsigned int port;
    rio_t rio;
    size_t len, object_size;

    parse_uri(uri, hostname, &port, query);
    sprintf(port_str, "%d", port);

    if ((serverfd = open_clientfd(hostname, port_str)) < 0) {
        return -1;
    }

    create_header_string(headers, hostname, query);

    // write to server socket
    Rio_readinitb(&rio, serverfd);
    Rio_writen(serverfd, headers, strlen(headers));

    // read server response
    while((len = Rio_readlineb(&rio, response, MAXLINE)) != 0){
        continue;
    }

    printf("%s", response);

    return 0;
}

void handle_proxy_request(int fd) {
    struct stat sbuf;
    char buf[MAXLINE], method[8], uri[MAXLINE], version[8];
    rio_t rio;

    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) {
        return;
    }
    printf("%s\n", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (!strcasecmp(method, "GET")) {
        // this will also discard HTTPS requests, which use CONNECT
        printf("Proxy can only handle HTTP GET requests");
        return;
    }

    forward_request(uri);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    if ((listenfd = open_listenfd(argv[1])) < 0) {
        printf("Unable to open port %s", argv[1]);
        return -1;
    }

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
            port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        handle_proxy_request(connfd);
        Close(connfd);
    }

    return 0;
}
