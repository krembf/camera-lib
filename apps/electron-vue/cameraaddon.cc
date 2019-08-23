#include <fstream>
#include <napi.h>
#include "lib/camera.hpp"
#include <vector>
#include <sstream>

// Namespace for using cout.
using namespace std;

using namespace camerautils;

static std::ofstream logFile("logfile.txt", std::ofstream::out);

static void log(std::string text) {
  logFile << text << endl;
  logFile.flush();
}

Napi::String MethodGetCameraName(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  log("Method GetCameraName enter");
  Camera mycam;
  auto name = mycam.getName();
  log(name);
  return Napi::String::New(env, name);
}

Napi::Buffer<uint8_t> MethodSnap(const Napi::CallbackInfo& info) {
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
  oss << "First pixel: " << (uint32_t) buffer.data()[0];
  log(oss.str());
  mycam.DeInit();

  auto napiBuffer = Napi::Buffer<uint8_t>::Copy(env, buffer.data(), bufferSize);
  oss << "First pixel from napi buffer: " << (uint32_t) napiBuffer[0];
  log(oss.str());
  return napiBuffer;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  log("Init enter");
  exports.Set(Napi::String::New(env, "GetCameraName"),
              Napi::Function::New(env, MethodGetCameraName));
  exports.Set(Napi::String::New(env, "Snap"),
              Napi::Function::New(env, MethodSnap));              
  return exports;
}

NODE_API_MODULE(camera, Init)