/*
 ============================================================================
 Name        : Webserver.c
 Author      : Mike
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ---------------------------------------- Defines for protocol */

#define HTTP_HOST "Host: "
#define HTTP_CONTENT_LENGTH "Content-Length: "
#define HTTP_CONTENT_TYPE "Content-Type: "

#define HTTP_MAX_COMMAND_LENGTH 7
#define HTTP_MAX_URI_LENGTH 255
#define HTTP_MAX_HTTP_VERSION_LENGTH 9
#define HTTP_MAX_HOST_LENGTH 255
#define HTTP_MAX_CONTENT_LENGTH_LENGTH 10
#define HTTP_MAX_CONTENT_TYPE_LENGTH 50

#define GET_REQUEST "GET /infotext.html HTTP/1.1\n\
Host: www.example.net"

#define POST_REQUEST "POST /contact_form.php HTTP/1.1\n\
Host: developer.mozilla.org\n\
Content-Length: 64\n\
Content-Type: application/x-www-form-urlencoded\n\
\n\n\
name=Joe%20User&request=Send%20me%20one%20of%20your%20catalogue"

/* ---------------------------------------- Struct for request and response*/

typedef struct {
	char* command;
	char* uri;
	char* httpVersion;
	char* host;
	int contentLength;
	char* contentType;
	char* data;
} HTTP_REQUEST;

/* ---------------------------------------- Forward declarations */

HTTP_REQUEST* parseRequest(const char* request, int len);
char * parseCommand(const char* request);
char * parseURI(const char * request, int pos);
char * parseChars(const char * string, int pos, int length);
char * parseHTTPVersion(const char * request, int pos);
char * parseSpecificAttrFromRequest(const char * request, char * attr,
		int maxlength);
char * parseData(const char * request, int length);

bool isRequestValid(HTTP_REQUEST * request);

/* ---------------------------------------- */

int main(void) {

	char* request = GET_REQUEST;
//	printf("%s\n\n", GET_REQUEST);
	puts("Parsing request now....\n");

	HTTP_REQUEST * req = parseRequest(request, strlen(request));

	if (req != NULL) {

		puts("Parsed following data: \n");

		printf("Command: %s\n", req->command);
		printf("File: %s\n", req->uri);
		printf("HTTP: %s\n", req->httpVersion);
		printf("Host: %s", req->host);
	} else {
		puts("Parsing failed! \n\n");
	}

	free(req);
	req = NULL;

	char* request2 = POST_REQUEST;
//	printf("\n\n%s\n\n", POST_REQUEST);
	puts("Parsing request now....\n");

	HTTP_REQUEST * req2 = parseRequest(request2, strlen(request2));

	if (req2 != NULL) {

		puts("Parsed following data: \n");

		printf("Command: %s\n", req2->command);
		printf("File: %s\n", req2->uri);
		printf("HTTP: %s\n", req2->httpVersion);
		printf("Host: %s\n", req2->host);
		printf("Content Length: %d\n", req2->contentLength);
		printf("Content Type: %s\n", req2->contentType);

		printf("Data: %s", req2->data);
	} else {
		puts("Parsing failed! \n\n");
	}

	free(req2);
	req2 = NULL;

	return EXIT_SUCCESS;
}

/**
 * Triggers the parse actions and initialzes the validation at the end
 */
HTTP_REQUEST* parseRequest(const char* request, int len) {

	int currentPosition = 0;
	HTTP_REQUEST * req = malloc(sizeof(HTTP_REQUEST));

	if (req != NULL) {
		req->command = parseCommand(request);
		currentPosition = strlen(req->command) + 1; // blank

		req->uri = parseURI(request, currentPosition);
		currentPosition += strlen(req->uri) + 1; // blank

		req->httpVersion = parseHTTPVersion(request, currentPosition);
		currentPosition += strlen(req->httpVersion) + 7; // new line + host label + blank

		req->host = parseSpecificAttrFromRequest(request, HTTP_HOST,
		HTTP_MAX_HOST_LENGTH);
		req->contentLength = atoi(
				parseSpecificAttrFromRequest(request, HTTP_CONTENT_LENGTH,
				HTTP_MAX_CONTENT_LENGTH_LENGTH));
		req->contentType = parseSpecificAttrFromRequest(request,
		HTTP_CONTENT_TYPE, HTTP_MAX_CONTENT_TYPE_LENGTH);

		req->data = parseData(request, req->contentLength);

		if (isRequestValid(req)) {
			// TODO emit event for app
			puts("event for app should be emitted");
		} else {
			// TODO respond with error message
			puts("request is invalid");
		}
	} else {
		// out of memory
	}

	return req;
}

/**
 * Gets the content of the request
 */
char * parseData(const char * request, int length) {
	char * delemiter = "\n\n";
	char * startPos = strstr(request, delemiter);
	if (startPos != NULL) {
		startPos += strlen(delemiter);
		int pos = startPos - request + 1;
		return parseChars(request, pos, length);
	} else {
		return NULL;
	}
}

/**
 * Parses specific fields from the request
 */
char * parseSpecificAttrFromRequest(const char * request, char * attr,
		int maxlength) {
	char * startPos = strstr(request, attr);
	if (startPos != NULL) {
		startPos += strlen(attr);
		int pos = startPos - request;
		return parseChars(request, pos, maxlength);
	} else {
		return NULL;
	}
}

/**
 * Parses the HTTP version
 */
char * parseHTTPVersion(const char * request, int pos) {
	return parseChars(request, pos, HTTP_MAX_HTTP_VERSION_LENGTH);
}

/**
 * Parses the requested filename
 */
char * parseURI(const char * request, int pos) {
	return parseChars(request, pos, HTTP_MAX_URI_LENGTH);
}

/**
 * Parses the HTTP command
 */
char * parseCommand(const char* request) {
	return parseChars(request, 0, HTTP_MAX_COMMAND_LENGTH);
}

/**
 * Parses chars of a string starting from pos and going on for length chars
 * Function stops when reaching a blank or new line and fills the rest of the array with
 * end of array ('\0')
 */
char * parseChars(const char * string, int pos, int length) {
	int i = pos;
	int j = 0;
	int stringLength = strlen(string);
	char * tmp = malloc(length);

	while (i < stringLength) {
		if (string[i] != ' ' && string[i] != '\n') {
			tmp[j] = string[i];
			i++;
			j++;
		} else {
			break;
		}
	}

	// fill the rest of the array with end of array
	if (j < length) {
		while (j < length) {
			tmp[j] = '\0';
			j++;
		}
	}

	return tmp;
}

/**
 * Validates http request according to rfc
 */
bool isRequestValid(HTTP_REQUEST * request) {

	// command
	// 405 method not allowed
	// 501 not implemented
	// https://tools.ietf.org/html/rfc2616#section-5.1.1
	if (!strcmp(request->command,"GET") &&!strcmp(request->command,"POST")) {
		return false;
	}

	// host
	// 400 missing host
	// https://tools.ietf.org/html/rfc2616#section-14.23

	// uri
	// 414 uri too long
	// https://tools.ietf.org/html/rfc2616#section-5.1.2
	// https://tools.ietf.org/html/rfc2616#section-3.2.1
	if (request->uri == NULL || request->host == NULL || request->httpVersion == NULL) {
		return false;
	}

	return true;
}
