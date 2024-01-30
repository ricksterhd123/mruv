// Simple HTTP/1.1 library
// - Uses picohttpparser for parsing HTTP request/responses
// - Uses my own code for writing HTTP request/responses

#pragma once

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <picohttpparser.h>

#include "utils.h"

#define MAX_HTTP_HEADERS 100
#define HTTP_MINOR_VERSION 1

typedef struct
{
    char *name;
    size_t name_len;
    char *value;
    size_t value_len;
} http_header_t;

http_header_t *get_n_http_header(const char *name, size_t name_len, const char *value, size_t value_len)
{

    // if (name[name_len] != '\0' || value[value_len] != '\0') {
    //     fprintf(stderr, "FATAL2: Copied non null-terminated string - %s %s\n", name, value);
    //     return NULL;
    // }

    http_header_t *header = (http_header_t *)malloc(sizeof(http_header_t));
    if (header == NULL)
    {
        fprintf(stderr, "FATAL: Failed to allocate memory\n");
        return NULL;
    }

    header->name = (char *)malloc(sizeof(char) * name_len);
    header->name_len = name_len;
    header->value = (char *)malloc(sizeof(char) * value_len);
    header->value_len = value_len;

    memcpy(header->name, name, name_len);
    memcpy(header->value, value, value_len);

    return header;
}

http_header_t *get_http_header(const char *name, const char *value)
{
    return get_n_http_header(name, strlen(name) + 1, value, strlen(value) + 1);
}

void free_http_header(http_header_t *header)
{
    free(header->name);
    free(header->value);
}

char *get_status_reason_code(unsigned short status_code)
{
    char status_code_str[4];
    itoa(status_code, status_code_str);

    char *reason;

    if (status_code == 200)
    {
        reason = "OK";
    }
    else if (status_code == 400)
    {
        reason = "Bad Request";
    }
    else
    {
        reason = "Unknown";
    }

    size_t status_reason_code_len = strlen(status_code_str) + strlen(reason) + 2;
    char *status_reason_code = (char *)malloc(sizeof(char) * status_reason_code_len);
    snprintf(status_reason_code, status_reason_code_len, "%s %s", status_code_str, reason);

    return status_reason_code;
}

typedef struct
{
    size_t num_headers;
    struct phr_header headers[MAX_HTTP_HEADERS];
    const char *method;
    size_t method_len;
    const char *path;
    size_t path_len;
} http_request_t;

typedef struct
{
    unsigned int status_code;

    char *http_date;
    size_t http_date_len;

    char *http_version;
    size_t http_version_len;

    char *status_reason;
    size_t status_reason_len;

    http_header_t **headers;
    size_t num_headers;

    char *body;
    size_t body_len;
} http_response_t;

http_request_t *parse_http_request(const char *buf, size_t len)
{
    int minor_version;

    http_request_t *req = (http_request_t *)malloc(sizeof(http_request_t));
    int pret = phr_parse_request(buf, len, &req->method, &req->method_len, &req->path, &req->path_len, &minor_version, req->headers, &req->num_headers, 0);

    if (pret == -1)
    {
        free(req);
        return NULL;
    }

    return req;
}

http_response_t *generate_http_response(unsigned int status_code, const char *body, size_t body_len)
{
    http_response_t *response = (http_response_t *)malloc(sizeof(http_response_t));

    if (response == NULL)
    {
        return NULL;
    }

    time_t result = time(NULL);
    response->http_version = new_str("HTTP/1.1");
    response->http_version_len = strlen(response->http_version) + 1;

    response->http_date = new_str(asctime(gmtime(&result)));
    response->http_date_len = strlen(response->http_date) + 1;

    response->status_code = status_code;
    response->status_reason = get_status_reason_code(status_code);
    response->status_reason_len = strlen(response->status_reason) + 1;

    response->body_len = body_len;
    response->body = (char *)malloc(sizeof(char) * body_len);
    memcpy(response->body, body, body_len);

    // Calculate Content-Length header
    size_t content_length_str_len = ceil(log10(body_len)) + 1;
    char *content_length_str = (char *)malloc(sizeof(char) * content_length_str_len);
    itoa(strlen(body), content_length_str);

    http_header_t *content_length = get_http_header("Content-Length", content_length_str);
    http_header_t *date = get_http_header("Date", response->http_date);
    http_header_t *server = get_http_header("Server", "mruv");
    http_header_t *last_modified = get_http_header("Last-Modified", response->http_date);
    http_header_t *accept_ranges = get_http_header("Accept-Ranges", "none");
    http_header_t *vary = get_http_header("Vary", "Accept-Encoding");
    http_header_t *content_type = get_http_header("Content-Type", "text/plain");

    response->headers = (http_header_t **)malloc(sizeof(http_header_t *) * 100);
    response->headers[0] = date;
    response->headers[1] = server;
    response->headers[2] = last_modified;
    response->headers[3] = accept_ranges;
    response->headers[4] = content_length;
    response->headers[5] = vary;
    response->headers[6] = content_type;
    response->num_headers = 7;

    free(content_length_str);

    return response;
}

void free_http_response(http_response_t *response)
{
    for (size_t i = 0; i < response->num_headers; i++)
    {
        free_http_header(response->headers[i]);
    }

    if (response->body != NULL)
    {
        free(response->body);
    }

    free(response->http_date);
    free(response->http_version);
    free(response->status_reason);

    free(response);
}

char *http_response_to_str(http_response_t *response)
{
    // Calculate Status line length first
    // [version_length] [status_reason]\r\n\0
    size_t status_line_len = response->http_version_len + response->status_reason_len;
    char* status_line = (char*) calloc(status_line_len, sizeof(char));
    snprintf(status_line, status_line_len, "%s %s", response->http_version, response->status_reason);

    // Pre-calulate total header string length, allocate and build string
    size_t headers_str_len = 0;
    for (size_t i = 0; i < response->num_headers; i++) {
        headers_str_len += response->headers[i]->name_len + 1 + response->headers[i]->value_len + 2;
    }

    char* headers_str = (char*) calloc(headers_str_len, sizeof(char));
    char* headers_str_tmp = headers_str;

    for (size_t i = 0; i < response->num_headers; i++) {
        if (response->headers[i] == NULL) {
            continue;
        }
        size_t headers_str_line_len = response->headers[i]->name_len + 1 + response->headers[i]->value_len + 2;
        char* headers_str_line = (char*) calloc(headers_str_line_len, sizeof(char));
        snprintf(headers_str_line, headers_str_line_len, "%s: %s\r\n", response->headers[i]->name, response->headers[i]->value);
        // printf("%s: %s\r\n", response->headers[i]->name, response->headers[i]->value);
        strncat(headers_str, headers_str_line, headers_str_line_len);
        free(headers_str_line);
    }

    // Add [status_line]\r\n[header...]\r\n[header...]\r\n[header...]\r\n[body]\0 terminated character
    size_t total_response_len = status_line_len + 2 + headers_str_len + 2 + response->body_len;

    char* response_str = (char*) calloc(total_response_len, sizeof(char));
    snprintf(response_str, total_response_len, "%s\r\n%s", headers_str, response->body);

    free(headers_str);
    free(status_line);

    return response_str;
}

/**
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Accept-Ranges: none
Content-Length: 51
Vary: Accept-Encoding
Content-Type: text/plain

Hello World! My content includes a trailing CRLF.
*/
