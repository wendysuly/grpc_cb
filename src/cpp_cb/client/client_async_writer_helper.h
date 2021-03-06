// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CLIENT_CLIENT_ASYNC_WRITER_HELPER_H
#define GRPC_CB_CLIENT_CLIENT_ASYNC_WRITER_HELPER_H

#include <functional>  // for function<>
#include <memory>  // for enable_shared_from_this<>

#include <grpc_cb/impl/call_sptr.h>         // for CallSptr
#include <grpc_cb/impl/message_queue.h>     // for MessageQueue
#include <grpc_cb/impl/message_sptr.h>      // for MessageSptr
#include <grpc_cb/status.h>                 // for Status
#include <grpc_cb/support/config.h>         // for GRPC_FINAL

namespace grpc_cb {

// Cache messages and write one by one.
// Used by ClientAsyncWriter and ClientAsyncReaderWriter.
// Differ from ClientAsyncReaderHelper:
//  ReaderHelper is ended by the peer, while WriterHelper is ended by Writer.
//  When Writer is destructed, WriterHelper must be informed that
//    there are no more writing.
//  And WriterHelper must live longer than Writer.
class ClientAsyncWriterHelper GRPC_FINAL
    : public std::enable_shared_from_this<ClientAsyncWriterHelper> {
 public:
  using OnEnd = std::function<void()>;
  ClientAsyncWriterHelper(const CallSptr& call_sptr,
                          const OnEnd& on_end);
  ~ClientAsyncWriterHelper();

 public:
  bool Queue(const MessageSptr& msg_sptr);

  // Set the end of messages. Differ with the close.
  // Do not queue further. May trigger on_end().
  void QueueEnd();

  void Abort() { aborted_ = true; }  // Abort writing. Stop sending.
  const Status& GetStatus() const { return status_; }

 public:
  void OnWritten(bool success);

 private:
  bool WriteNext();

 private:
  const CallSptr call_sptr_;
  bool aborted_ = false;  // to abort writer
  const OnEnd on_end_;  // callback on the end
  Status status_;

  MessageQueue msg_queue_;  // Cache messages to write.

  // Grpc only allows to write one by one.
  // When the last msg is writing, the queue is empty, so we need it.
  bool is_writing_ = false;

  // no more msg to queue after QueueEnd()
  bool is_queue_ended_ = false;
};  // class ClientAsyncWriterHelper

}  // namespace grpc_cb
#endif  // GRPC_CB_CLIENT_CLIENT_ASYNC_WRITER_HELPER_H
