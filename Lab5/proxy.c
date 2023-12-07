#include <stdio.h>
#include <string.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// default headers
static const char *user_agent_def = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";

static const char *user_agent_key = "User-Agent: ";
static const char *connection_key = "Connection: ";
static const char *proxy_connection_key = "Proxy-Connection: ";
static const char *host_key = "Host: ";

int has_key(char *header, const char *key) {
    int len = strlen(key);
    return strncmp(header, key, len) == 0;
}

void parse_uri(char *uri, char *hostname, unsigned int *port, char *query) {
    char *ptr;
    if (has_key(uri, "http://")) {
        ptr = uri + 7;  // skip characters of http://
    } else {
        ptr = uri;
    }

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

int is_other_header(char *header) {
    return !(has_key(header, user_agent_key) || has_key(header, connection_key) ||
        has_key(header, proxy_connection_key));
}

void create_header_string (rio_t rio, char *headers, char *hostname, char *query) {
    char buf[MAXLINE], host_hdr[MAXLINE] = "";
    int host_flag = 1, user_agent_flag = 1;

    // HTTP request header
    sprintf(headers, "GET %s HTTP/1.0\r\n", query);

    // all default value headers
    strcat(headers, connection_hdr);
    strcat(headers, proxy_connection_hdr);

    // check if the client sent any headers, readlineb will hang otherwise
    if (strcmp(rio.rio_bufptr, "") != 0) {
        // iterate over existing headers of request to the proxy
        while (Rio_readlineb(&rio, buf, MAXLINE) != 0) {
            if (strcmp(buf, "\r\n") == 0) {
                // reached end of all headers
                break;
            } else if (has_key(buf, host_key)) {
                // browser overrides Host header
                strcat(headers, buf);
                host_flag = 0;
            } else if (has_key(buf, user_agent_key)) {
                // browser overrides User Agent header
                strcat(headers, buf);
                user_agent_flag = 0;
            } else if (is_other_header(buf)) {
                // any other headers that a browser might include, like Cookies
                strcat(headers, buf);
            }
        }
    }

    // if the Host or User Agent were not set by existing headers
    if (host_flag) {
        sprintf(host_hdr, "Host: %s\r\n", hostname);
        strcat(headers, host_hdr);
    }
    if (user_agent_flag) {
        strcat(headers, user_agent_def);
    }

    // CRLF indicate end of request
    strcat(headers, "\r\n");
}

void forward_request(int clientfd, rio_t rio, char *uri) {
    int serverfd;
    char hostname[MAXLINE], port_str[8], query[MAXLINE];
    char headers[MAXLINE] = "", response[MAXLINE] = "";
    unsigned int port;
    size_t len;

    parse_uri(uri, hostname, &port, query);
    sprintf(port_str, "%d", port);

    // open connection to destination server
    if ((serverfd = open_clientfd(hostname, port_str)) < 0) {
        return;
    }

    create_header_string(rio, headers, hostname, query);

    // write to server socket
    Rio_readinitb(&rio, serverfd);
    Rio_writen(serverfd, headers, strlen(headers));

    // read server response
    while((len = Rio_readlineb(&rio, response, MAXLINE)) != 0){
        Rio_writen(clientfd, response, len);  // write server response back to client
    }

    Close(serverfd);
}

void handle_proxy_request(int clientfd) {
    char buf[MAXLINE], method[8], uri[MAXLINE], version[8];
    rio_t rio;

    Rio_readinitb(&rio, clientfd);
    memset(rio.rio_buf, 0, 8192);  // rio object persists some data between requests

    // receive HTTP request
    if (!Rio_readlineb(&rio, buf, MAXLINE)) {
        return;
    }
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET") != 0) {
        // this will also discard HTTPS requests, which use CONNECT
        printf("Proxy can only handle HTTP GET requests\n");
        return;
    }

    forward_request(clientfd, rio, uri);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    if ((listenfd = open_listenfd(argv[1])) < 0) {
        printf("Unable to open port %s\n", argv[1]);
        return 1;
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
