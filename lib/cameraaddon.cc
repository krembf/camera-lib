#include <fstream>
#include <napi.h>
#include "camera.hpp"
#include <vector>
#include <sstream>
#include <thread>
#include "napi-thread-safe-callback.hpp"

// Namespace for using cout.
using namespace std;

using namespace camerautils;

static std::ofstream logFile("logfile.txt", std::ofstream::out);

static void log(std::string text)
{
  logFile << text << endl;
  logFile.flush();
}

static Napi::Buffer<uint8_t> napiBuffer;

Napi::String MethodGetCameraName(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  log("Method GetCameraName enter");
  Camera mycam;
  auto name = mycam.getName();
  log(name);
  return Napi::String::New(env, name);
}

Napi::Buffer<uint8_t> MethodSnap(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  log("Method Snap enter");
  Camera mycam;

  // The image dimensions
  const auto width = 960;
  const auto height = 600;
  const auto byteDepth = 4; //RGBA
  const auto bufferSize = width * height * byteDepth;

  std::vector<uint8_t> buffer(bufferSize);
  mycam.Init();
  mycam.Snap(buffer.data(), bufferSize);
  std::ostringstream oss;
  oss << "First pixel: " << (uint32_t)buffer.data()[0];
  log(oss.str());
  mycam.DeInit();

  auto napiBuffer = Napi::Buffer<uint8_t>::Copy(env, buffer.data(), bufferSize);
  oss << "First pixel from napi buffer: " << (uint32_t)napiBuffer[0];
  log(oss.str());
  return napiBuffer;
}

class SnapAsyncWorker : public Napi::AsyncWorker
{
public:
  SnapAsyncWorker(
      Napi::Buffer<uint8_t> &data,
      const Napi::Function &_callback,
      Napi::Number _numberofSnaps)
      : Napi::AsyncWorker(_callback),
        ccc(std::make_shared<ThreadSafeCallback>(_callback)),
        numberofSnaps(_numberofSnaps.Int64Value()),
        dataRef(Napi::ObjectReference::New(data, 1)),
        dataPtr(data.Data())
  {
    mycam.Init();  
    bufferIndex = 0;  
    numberOfBuffers = 4;
  }

protected:
  void Execute() override
  {
    log("SnapAsyncWorker::Execute enter");
    //std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // The image dimensions
    const auto width = 960;
    const auto height = 600;
    const auto byteDepth = 4; //RGBA
    const auto bufferSize = width * height * byteDepth;

    while (cnt <= numberofSnaps)
    {
      auto start = chrono::steady_clock::now();
      mycam.Snap(dataPtr + bufferIndex * bufferSize, bufferSize);
      auto end = chrono::steady_clock::now();
      std::ostringstream oss;
      oss << "Snap took " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " msec. ";
      oss << "First pixel: " << (uint32_t)dataPtr[0] << endl;
      log(oss.str());
      ccc->call({});
      bufferIndex = (bufferIndex + 1) % numberOfBuffers;
      cnt++;
    }
    mycam.DeInit();
  }

  void OnOK() override
  {
    Callback().Call({});

    dataRef.Unref();
  }

  void OnError(const Napi::Error &e) override
  {
    Napi::Env env = Env();

    Callback().MakeCallback(
        Receiver().Value(),
        {e.Value(),
         env.Undefined()});
  }

private:
  std::shared_ptr<ThreadSafeCallback> ccc;
  int64_t numberofSnaps;
  Napi::ObjectReference dataRef;
  uint8_t *dataPtr;
  int cnt = 0;
  Camera mycam;
  int64_t bufferIndex;
  int64_t numberOfBuffers;
};

void MethodSnapAsync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  log("Method Snap enter");

  //
  // Account for known potential issues that MUST be handled by
  // synchronously throwing an `Error`
  //
  if (info.Length() != 3)
  {
    Napi::TypeError::New(env, "Invalid argument count").ThrowAsJavaScriptException();
    return;
  }

  if (!info[1].IsFunction())
  {
    Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
    return;
  }

  //
  // Handle all other potential issues asynchronously via the provided callback
  //

  Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
  Napi::Function cb = info[1].As<Napi::Function>();
  auto numberOfSnaps = info[2].As<Napi::Number>();

  (new SnapAsyncWorker(buffer, cb, numberOfSnaps))->Queue();

  return;
}

class ErrorAsyncWorker : public Napi::AsyncWorker
{
public:
  ErrorAsyncWorker(
      const Napi::Function &callback,
      Napi::Error error) : Napi::AsyncWorker(callback), error(error)
  {
  }

protected:
  void Execute() override
  {
    // Do nothing...?
  }

  void OnOK() override
  {
    Napi::Env env = Env();

    Callback().MakeCallback(
        Receiver().Value(),
        {error.Value(),
         env.Undefined()});
  }

  void OnError(const Napi::Error &e) override
  {
    Napi::Env env = Env();

    Callback().MakeCallback(
        Receiver().Value(),
        {e.Value(),
         env.Undefined()});
  }

private:
  Napi::Error error;
};

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  log("Init enter");
  exports.Set(Napi::String::New(env, "GetCameraName"),
              Napi::Function::New(env, MethodGetCameraName));
  exports.Set(Napi::String::New(env, "Snap"),
              Napi::Function::New(env, MethodSnap));
  exports.Set(Napi::String::New(env, "SnapAsync"),
              Napi::Function::New(env, MethodSnapAsync));
  return exports;
}

NODE_API_MODULE(camera, Init)