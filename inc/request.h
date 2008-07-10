/** **************************************************************************
 * request.h
 * 
 * Copyright 2008 Bryan Ischo <bryan@ischo.com>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the
 *
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ************************************************************************** **/

#ifndef REQUEST_H
#define REQUEST_H

#include "libs3.h"
#include "error_parser.h"
#include "response_headers_handler.h"
#include "util.h"

/**
 * Any return value other than S3StatusOK will stop the request processing
 * immediately
 **/
typedef S3Status (RequestHeadersCallback)
    (const S3ResponseHeaders *responseHeaders, void *callbackData);

/**
 * As a 'read' function:
 *   - Fill [buffer] with up to [bufferSize] bytes, return the number of
 *     bytes actually filled; returning 0 means EOF.
 *
 * As a 'write' function:
 *   - [bufferSize] bytes are supplied in [buffer].  Return anything other
 *     than [bufferSize] to stop the request processing immediately
 **/
typedef int (RequestDataCallback)
    (char *buffer, int bufferSize, void *callbackData);

/**
 **/
typedef void (RequestCompleteCallback)
    (S3Status requestStatus, int httpResponseCode, 
     const S3ErrorDetails *s3ErrorDetails, void *callbackData);
                                          

// Describes a type of HTTP request (these are our supported HTTP "verbs")
typedef enum
{
    HttpRequestTypeGET,
    HttpRequestTypeHEAD,
    HttpRequestTypePUT,
    HttpRequestTypeCOPY,
    HttpRequestTypeDELETE
} HttpRequestType;


// This completely describes a request.  A RequestParams is not required to be
// allocated from the heap and its lifetime is not assumed to extend beyond
// the lifetime of the function to which it has been passed.
typedef struct RequestParams
{
    // Request type, affects the HTTP verb used
    HttpRequestType httpRequestType;

    // Protocol to use for request
    S3Protocol protocol;

    // URI style to use for request
    S3UriStyle uriStyle;

    // Bucket name, if any
    const char *bucketName;

    // Key, if any
    const char *key;

    // Query params - ready to append to URI (i.e. ?p1=v1?p2=v2)
    const char *queryParams;

    // sub resource, like ?acl, ?location, ?torrent
    const char *subResource;

    // AWS Access Key ID
    const char *accessKeyId;

    // AWS Secret Access Key
    const char *secretAccessKey;

    // Request headers
    const S3RequestHeaders *requestHeaders;

    // Callback to be made when headers are available.  May not be called.
    RequestHeadersCallback *headersCallback;

    // Callback to be made to supply data to send to S3.  May not be called.
    RequestDataCallback *toS3Callback;

    // Number of bytes total that readCallback will supply
    int64_t toS3CallbackTotalSize;

    // Callback to be made that supplies data read from S3.  May not be called.
    RequestDataCallback *fromS3Callback;

    // Callback to be made when request is complete.  This will *always* be
    // called.
    RequestCompleteCallback *completeCallback;

    // Data passed to the callbacks
    void *callbackData;
} RequestParams;


// This is the stuff associated with a request that needs to be on the heap
// (and thus live while a curl_multi is in use).
typedef struct Request
{
    // The status of this Request, as will be reported to the user via the
    // complete callback
    S3Status status;

    // The HTTP headers to use for the curl request
    struct curl_slist *headers;

    // The CURL structure driving the request
    CURL *curl;

    // libcurl requires that the uri be stored outside of the curl handle
    char uri[MAX_URI_SIZE + 1];

    // The HTTP response code that S3 sent back for this request
    int httpResponseCode;

    // Callback to be made when headers are available.  May not be called.
    RequestHeadersCallback *headersCallback;

    // Callback to be made to supply data to send to S3.  May not be called.
    RequestDataCallback *toS3Callback;

    // Callback to be made that supplies data read from S3.  May not be called.
    RequestDataCallback *fromS3Callback;

    // Callback to be made when request is complete.  This will *always* be
    // called.
    RequestCompleteCallback *completeCallback;

    // Data passed to the callbacks
    void *callbackData;

    // Handler of response headers
    ResponseHeadersHandler responseHeadersHandler;

    // This is set to nonzero after the haders callback has been made
    int headersCallbackMade;

    // Parser of errors
    ErrorParser errorParser;
} Request;


// Request functions
// ----------------------------------------------------------------------------

// Initialize the API
S3Status request_api_initialize(const char *userAgentInfo);

// Deinitialize the API
void request_api_deinitialize();

// Perform a request; if context is 0, performs the request immediately;
// otherwise, sets it up to be performed by context.
void request_perform(const RequestParams *params, S3RequestContext *context);

// Called by the internal request code or internal request context code when a
// curl has finished the request
void request_finish(Request *request);


#endif /* REQUEST_H */
