#include <fstream>
#include <napi.h>
#include "lib/camera.hpp"
#include <vector>
#include <sstream>
#include <thread>

// Namespace for using cout.
using namespace std;

using namespace camerautils;

static std::ofstream logFile("logfile.txt", std::ofstream::out);

static void log(std::string text)
{
  logFile << text << endl;
  logFile.flush();
}

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
      const Napi::Function &callback,
      Napi::Env _env) : Napi::AsyncWorker(callback),
                        env(_env),
                        dataRef(Napi::ObjectReference::New(data, 1)),
                        dataPtr(data.Data())                        
  {
  }

protected:
  void Execute() override
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    Camera mycam;

    // The image dimensions
    const auto width = 960;
    const auto height = 600;
    const auto byteDepth = 4; //RGBA
    const auto bufferSize = width * height * byteDepth;

    mycam.Init();
    auto start = chrono::steady_clock::now();
    mycam.Snap(dataPtr, bufferSize);
    auto end = chrono::steady_clock::now();
    std::ostringstream oss;
    oss << "Snap took " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " msec";
    log(oss.str());
    oss << "First pixel: " << (uint32_t)dataPtr[0];
    log(oss.str());
    mycam.DeInit();

    oss << "First pixel from napi buffer async: " << (uint32_t)dataPtr[0];
    log(oss.str());
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
  Napi::Env env;
  Napi::ObjectReference dataRef;
  uint8_t *dataPtr;
};

void MethodSnapAsync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  log("Method Snap enter");

  //
  // Account for known potential issues that MUST be handled by
  // synchronously throwing an `Error`
  //
  if (info.Length() != 2)
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

  for(auto i = 0; i < 200; i++)
  {
    (new SnapAsyncWorker(buffer, cb, env))->Queue();
  }

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
  exports.Set(Napi::String::New(env, "snapAsync"),
              Napi::Function::New(env, MethodSnapAsync));
  return exports;
}

NODE_API_MODULE(camera, Init)