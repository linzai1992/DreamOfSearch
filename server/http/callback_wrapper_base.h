/**
 *
 */
#ifndef SERVER_HTTP_CALLBACK_WRAPPER_H_
#define SERVER_HTTP_CALLBACK_WRAPPER_H_
#include "common/basics.h"
#include "server/http/http_client.h"

class CallBackWrapperBase {
 public:
  CallBackWrapperBase() {}
  virtual ~CallBackWrapperBase() {}
  virtual void Process(const HttpClient& client,
                       evhtp_request_t* req) {
    string resp;
    client.GetResponse(req, &resp);
    Process(resp, req);
  }
  virtual void Process(const string& response,
                       evhtp_request_t* req) = 0;

 private:

  DO_NOT_COPY_AND_ASSIGN(CallBackWrapperBase);
};

#endif  // SERVER_HTTP_CALLBACK_WRAPPER_H_