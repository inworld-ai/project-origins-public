// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: greet.proto
#ifndef GRPC_greet_2eproto__INCLUDED
#define GRPC_greet_2eproto__INCLUDED

#include "greet.pb.h"

#include <functional>
#include <grpc/impl/codegen/port_platform.h>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace whisper {

class Whisper final {
 public:
  static constexpr char const* service_full_name() {
    return "whisper.Whisper";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>> speech(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>>(speechRaw(context));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>> Asyncspeech(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>>(AsyncspeechRaw(context, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>> PrepareAsyncspeech(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>>(PrepareAsyncspeechRaw(context, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      virtual void speech(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::whisper::Packet,::whisper::Packet>* reactor) = 0;
      #else
      virtual void speech(::grpc::ClientContext* context, ::grpc::experimental::ClientBidiReactor< ::whisper::Packet,::whisper::Packet>* reactor) = 0;
      #endif
    };
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    typedef class experimental_async_interface async_interface;
    #endif
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    async_interface* async() { return experimental_async(); }
    #endif
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>* speechRaw(::grpc::ClientContext* context) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>* AsyncspeechRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::whisper::Packet, ::whisper::Packet>* PrepareAsyncspeechRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    std::unique_ptr< ::grpc::ClientReaderWriter< ::whisper::Packet, ::whisper::Packet>> speech(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriter< ::whisper::Packet, ::whisper::Packet>>(speechRaw(context));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>> Asyncspeech(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>>(AsyncspeechRaw(context, cq, tag));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>> PrepareAsyncspeech(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>>(PrepareAsyncspeechRaw(context, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      void speech(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::whisper::Packet,::whisper::Packet>* reactor) override;
      #else
      void speech(::grpc::ClientContext* context, ::grpc::experimental::ClientBidiReactor< ::whisper::Packet,::whisper::Packet>* reactor) override;
      #endif
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientReaderWriter< ::whisper::Packet, ::whisper::Packet>* speechRaw(::grpc::ClientContext* context) override;
    ::grpc::ClientAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>* AsyncspeechRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) override;
    ::grpc::ClientAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>* PrepareAsyncspeechRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_speech_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status speech(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::whisper::Packet, ::whisper::Packet>* stream);
  };
  template <class BaseClass>
  class WithAsyncMethod_speech : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_speech() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_speech() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status speech(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::whisper::Packet, ::whisper::Packet>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestspeech(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::whisper::Packet, ::whisper::Packet>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncBidiStreaming(0, context, stream, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_speech<Service > AsyncService;
  template <class BaseClass>
  class ExperimentalWithCallbackMethod_speech : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithCallbackMethod_speech() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodCallback(0,
          new ::grpc::internal::CallbackBidiHandler< ::whisper::Packet, ::whisper::Packet>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context) { return this->speech(context); }));
    }
    ~ExperimentalWithCallbackMethod_speech() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status speech(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::whisper::Packet, ::whisper::Packet>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerBidiReactor< ::whisper::Packet, ::whisper::Packet>* speech(
      ::grpc::CallbackServerContext* /*context*/)
    #else
    virtual ::grpc::experimental::ServerBidiReactor< ::whisper::Packet, ::whisper::Packet>* speech(
      ::grpc::experimental::CallbackServerContext* /*context*/)
    #endif
      { return nullptr; }
  };
  #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
  typedef ExperimentalWithCallbackMethod_speech<Service > CallbackService;
  #endif

  typedef ExperimentalWithCallbackMethod_speech<Service > ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_speech : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_speech() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_speech() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status speech(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::whisper::Packet, ::whisper::Packet>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_speech : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_speech() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_speech() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status speech(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::whisper::Packet, ::whisper::Packet>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void Requestspeech(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncBidiStreaming(0, context, stream, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class ExperimentalWithRawCallbackMethod_speech : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithRawCallbackMethod_speech() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackBidiHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context) { return this->speech(context); }));
    }
    ~ExperimentalWithRawCallbackMethod_speech() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status speech(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::whisper::Packet, ::whisper::Packet>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerBidiReactor< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* speech(
      ::grpc::CallbackServerContext* /*context*/)
    #else
    virtual ::grpc::experimental::ServerBidiReactor< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* speech(
      ::grpc::experimental::CallbackServerContext* /*context*/)
    #endif
      { return nullptr; }
  };
  typedef Service StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef Service StreamedService;
};

}  // namespace whisper


#endif  // GRPC_greet_2eproto__INCLUDED
