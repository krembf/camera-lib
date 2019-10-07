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
  logFile << "[" << std::this_thread::get_id() << "] " << text << endl;
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
      const Napi::Function &_callback,
      Napi::Number _numberofSnaps)
      : Napi::AsyncWorker(_callback),
        threadSafeCallback(std::make_shared<ThreadSafeCallback>(_callback)),
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
      threadSafeCallback->call({});
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
  std::shared_ptr<ThreadSafeCallback> threadSafeCallback;
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

class SnapAsyncWorkerWithCallback : public Napi::AsyncWorker
{
public:
  SnapAsyncWorkerWithCallback(
      CameraSettings cameraSettings,
      Napi::Buffer<uint8_t> &data,
      const Napi::Function &_callback,
      Napi::Number _numberofSnaps)
      : Napi::AsyncWorker(_callback),
        _cameraSettings(cameraSettings),
        threadSafeCallback(std::make_shared<ThreadSafeCallback>(_callback)),
        numberofSnaps(_numberofSnaps.Int64Value()),
        dataRef(Napi::ObjectReference::New(data, 1)),
        dataPtr(data.Data())
  {
    log("SnapAsyncWorkerWithCallback Constructor enter");
    mycam.Init();  
    bufferIndex = 0;
    numberOfBuffers = 4;
  }

protected:
  void Execute() override
  {
    log("SnapAsyncWorkerWithCallback::Execute enter");

    // The image dimensions
    const auto width = _cameraSettings.ImageWidth;
    const auto height = _cameraSettings.ImageHeight;
    const auto byteDepth = _cameraSettings.ByteDepth; //RGBA
    const auto bufferSize = width * height * byteDepth;

    mycam.SnapContinuous(_cameraSettings, dataPtr, bufferSize,
      [this]() {
        log("Lambda Callback Enter");
        this->threadSafeCallback->call({});
        }
      );

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
  CameraSettings _cameraSettings; 
  std::shared_ptr<ThreadSafeCallback> threadSafeCallback;
  int64_t numberofSnaps;
  Napi::ObjectReference dataRef;
  uint8_t *dataPtr;
  int cnt = 0;
  Camera mycam; 
  int64_t bufferIndex;
  int64_t numberOfBuffers;
};

class WrappedCameraSettings : public Napi::ObjectWrap<WrappedCameraSettings>
{
public:
  Napi::Value GetValue(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->_value);
  }

  void SetValue(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    // Napi::Env env = info.Env();
    // ...
    this->_value = static_cast<double>(value.ToNumber());
  }

  Napi::Value GetImageWidth(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->_imageWidth);
  }

  void SetImageWidth(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    this->_imageWidth = static_cast<int>(value.ToNumber());
  }  

  Napi::Value GetImageHeight(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->_imageHeight);
  }

  void SetImageHeight(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    this->_imageHeight = static_cast<int>(value.ToNumber());
  }   

  Napi::Value GetByteDepth(const Napi::CallbackInfo &info)
  {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->_byteDepth);
  }

  void SetByteDepth(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    this->_byteDepth = static_cast<int>(value.ToNumber());
  }     

  camerautils::CameraSettings ToCameraSetings()
  {
    camerautils::CameraSettings cameraSettings;
    cameraSettings.ImageWidth = _imageWidth;
    cameraSettings.ImageHeight = _imageHeight;
    cameraSettings.ByteDepth = _byteDepth;
    return cameraSettings;
  }

  static Napi::Object Init(Napi::Env env, Napi::Object exports)
  {
    Napi::Function func = DefineClass(env, "WrappedCameraSettings", {
      InstanceAccessor("value", &WrappedCameraSettings::GetValue, &WrappedCameraSettings::SetValue),
      InstanceAccessor("imageWidth", &WrappedCameraSettings::GetImageWidth, &WrappedCameraSettings::SetImageWidth),
      InstanceAccessor("imageHeight", &WrappedCameraSettings::GetImageHeight, &WrappedCameraSettings::SetImageHeight),
      InstanceAccessor("byteDepth", &WrappedCameraSettings::GetByteDepth, &WrappedCameraSettings::SetByteDepth)
    }, nullptr);

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("WrappedCameraSettings", func);

    return exports;
  }

  WrappedCameraSettings(const Napi::CallbackInfo &info): Napi::ObjectWrap<WrappedCameraSettings>(info)
  {
    // Napi::Env env = info.Env();
    // ...
    // Napi::Number value = info[0].As<Napi::Number>();
    // this->_value = value.DoubleValue();
  }

private:
  static Napi::FunctionReference constructor;
  double _value;
  int _imageWidth;
  int _imageHeight;
  int _byteDepth;
};

Napi::FunctionReference WrappedCameraSettings::constructor;


void MethodSnapWithCallback(const Napi::CallbackInfo &info)
{
  log("MethodSnapWithCallback enter");
  Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
  Napi::Function cb = info[1].As<Napi::Function>();
  auto numberOfSnaps = info[2].As<Napi::Number>();
  WrappedCameraSettings *cs = Napi::ObjectWrap<WrappedCameraSettings>::Unwrap(info[3].As<Napi::Object>());

  std::ostringstream oss;
  oss << "Got camera setting " << (double) cs->GetValue(info).As<Napi::Number>();
  oss << "Got camera width " << (int) cs->GetImageWidth(info).As<Napi::Number>();
  log(oss.str());

  (new SnapAsyncWorkerWithCallback(cs->ToCameraSetings(), buffer, cb, numberOfSnaps))->Queue();
  log("MethodSnapWithCallback Exit");
}

// Initialize native add-on
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  log("Init enter");
  exports.Set(Napi::String::New(env, "GetCameraName"),
              Napi::Function::New(env, MethodGetCameraName));
  exports.Set(Napi::String::New(env, "Snap"),
              Napi::Function::New(env, MethodSnap));
  exports.Set(Napi::String::New(env, "SnapAsync"),
              Napi::Function::New(env, MethodSnapAsync));
  exports.Set(Napi::String::New(env, "SnapWithCallback"),
              Napi::Function::New(env, MethodSnapWithCallback));  

  WrappedCameraSettings::Init(env, exports);

  return exports;
}

NODE_API_MODULE(camera, Init)