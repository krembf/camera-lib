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
      const Napi::Function &callback, 
      Napi::Env _env) : 
      Napi::AsyncWorker(callback), 
      dataRef(Napi::ObjectReference::New(data, 1)), 
      dataPtr(data.Data()), 
      dataLength(data.Length()), 
      env(_env)
    {
    }

protected:
  void Execute() override
  {
    Camera mycam;

    // The image dimensions
    const auto width = 960;
    const auto height = 600;
    const auto byteDepth = 4; //RGBA
    const auto bufferSize = width * height * byteDepth;

    mycam.Init();
    mycam.Snap(dataPtr, bufferSize);
    std::ostringstream oss;
    oss << "First pixel: " << (uint32_t)dataPtr[0];
    log(oss.str());
    mycam.DeInit();

    oss << "First pixel from napi buffer async: " << (uint32_t)dataPtr[0];
    log(oss.str());
  }

  void OnOK() override
  {
    Napi::Env env = Env();

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
  size_t dataLength;
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

  (new SnapAsyncWorker(buffer, cb, env))->Queue();

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

class SumAsyncWorker : public Napi::AsyncWorker
{
public:
  SumAsyncWorker(
      const Napi::Function &callback,
      const double arg0,
      const double arg1) : Napi::AsyncWorker(callback), arg0(arg0), arg1(arg1), sum(0)
  {
  }

protected:
  void Execute() override
  {
    sum = arg0 + arg1;
  }

  void OnOK() override
  {
    Napi::Env env = Env();

    Callback().MakeCallback(
        Receiver().Value(),
        {env.Null(),
         Napi::Number::New(env, sum)});
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
  double arg0;
  double arg1;
  double sum;
};

void SumAsyncCallback(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  //
  // Account for known potential issues that MUST be handled by
  // synchronously throwing an `Error`
  //
  if (info.Length() < 3)
  {
    Napi::TypeError::New(env, "Invalid argument count").ThrowAsJavaScriptException();
    return;
  }

  if (!info[2].IsFunction())
  {
    Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
    return;
  }

  //
  // Handle all other potential issues asynchronously via the provided callback
  //

  Napi::Function cb = info[2].As<Napi::Function>();

  if (info.Length() != 3)
  {
    (new ErrorAsyncWorker(cb, Napi::TypeError::New(env, "Invalid argument count")))->Queue();
  }
  else if (!info[0].IsNumber() || !info[1].IsNumber())
  {
    (new ErrorAsyncWorker(cb, Napi::TypeError::New(env, "Invalid argument types")))->Queue();
  }
  else
  {
    double arg0 = info[0].As<Napi::Number>().DoubleValue();
    double arg1 = info[1].As<Napi::Number>().DoubleValue();

    (new SumAsyncWorker(cb, arg0, arg1))->Queue();
  }

  return;
}

Napi::Value CallEmit(const Napi::CallbackInfo &info)
{
    char buff1[128];
    char buff2[128];
    int  sensor1 = 0;
    int  sensor2 = 0;

    Napi::Env env = info.Env();

    Napi::Function emit = info[0].As<Napi::Function>();
    emit.Call(  {Napi::String::New(env, "start")}  );

    for (int i = 0; i < 5; i++)
    {
        // Let us simulate some delay for collecting data from its sensors
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        sprintf(buff1, "sensor1 data %d ...", ++sensor1);

        emit.Call( { Napi::String::New(env, "sensor1"),
                   Napi::String::New(env, buff1 ) } );

        // Let, sensor 2 data is reported half the rate as sensor1
        if (i % 2)
        {
            sprintf(buff2, "sensor2 data %d ...", ++sensor2);
            emit.Call({ Napi::String::New(env, "sensor2"),
                       Napi::String::New(env, buff2) });
        }
    }

    emit.Call( {Napi::String::New(env, "end")} );
    return Napi::String::New( env, "OK" );
}

Napi::Value SnapEmit(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  log("Method Snap enter");
  Camera mycam;

  Napi::Function emit = info[0].As<Napi::Function>();
  emit.Call({Napi::String::New(env, "start")});

  // The image dimensions
  const auto width = 960;
  const auto height = 600;
  const auto byteDepth = 4; //RGBA
  const auto bufferSize = width * height * byteDepth;

  std::vector<uint8_t> buffer(bufferSize);
  mycam.Init();
  for (int i = 0; i < 2; i++)
  {
    // Let us simulate some delay for collecting data from its sensors
    // std::this_thread::sleep_for(std::chrono::milliseconds(250));

    mycam.Snap(buffer.data(), bufferSize);
    std::ostringstream oss;
    oss << "First pixel: " << (uint32_t)buffer.data()[0];
    log(oss.str());

    auto napiBuffer = Napi::Buffer<uint8_t>::Copy(env, buffer.data(), bufferSize);
    oss << "First pixel from napi buffer: " << (uint32_t)napiBuffer[0];
    log(oss.str());

    emit.Call({Napi::String::New(env, "buffer"), napiBuffer});
  }

  emit.Call({Napi::String::New(env, "end")});

  mycam.DeInit();
  return Napi::String::New(env, "OK");
}

Napi::Value SnapEmitThread(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  log("Method SnapEmitThread enter");

  Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
  uint8_t *dataPtr(buffer.Data());
  Napi::Function emit = info[1].As<Napi::Function>();
  emit.Call({Napi::String::New(env, "start")});

  Camera mycam;
  mycam.Init();

  for (int i = 0; i < 5; i++)
  {
    std::thread([&mycam, &buffer] {
      // The image dimensions
      const auto width = 960;
      const auto height = 600;
      const auto byteDepth = 4; //RGBA
      const auto bufferSize = width * height * byteDepth;
      // Let us simulate some delay for collecting data from its sensors
      std::this_thread::sleep_for(std::chrono::milliseconds(250));

      mycam.Snap(buffer.Data(), bufferSize);
      std::ostringstream oss;
      oss << "First pixel: " << (uint32_t)buffer.Data()[0] << endl;
      log(oss.str());  
    }).join();
    
    std::ostringstream oss;
    oss << "First pixel after thread execution: " << (uint32_t)buffer.Data()[0] << endl;
    log(oss.str());     
    oss << "Emitting buffer event" << endl;
    log(oss.str());
    emit.Call({Napi::String::New(env, "buffer")});
  }

  emit.Call({Napi::String::New(env, "end")});

  mycam.DeInit();

  return Napi::String::New(env, "OK");
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  log("Init enter");
  exports.Set(Napi::String::New(env, "GetCameraName"),
              Napi::Function::New(env, MethodGetCameraName));
  exports.Set(Napi::String::New(env, "Snap"),
              Napi::Function::New(env, MethodSnap));
  exports.Set(Napi::String::New(env, "SnapAsync"),
              Napi::Function::New(env, MethodSnapAsync));
  exports.Set(Napi::String::New(env, "add"),
              Napi::Function::New(env, SumAsyncCallback));
  exports.Set(Napi::String::New(env, "callEmit"),
              Napi::Function::New(env, CallEmit));
  exports.Set(Napi::String::New(env, "snapEmit"),
              Napi::Function::New(env, SnapEmit));
  exports.Set(Napi::String::New(env, "snapEmitThread"),
              Napi::Function::New(env, SnapEmitThread));              
  return exports;
}

NODE_API_MODULE(camera, Init)