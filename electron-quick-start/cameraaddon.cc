#include <fstream>
#include <napi.h>
#include "lib/camera.hpp"

// Namespace for using cout.
using namespace std;

using namespace camerautils;

static std::ofstream logFile("logfile.txt", std::ofstream::out);

static void log(std::string text) {
  logFile << text << endl;
  logFile.flush();
}

Napi::String Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  log("hello method enter");
  Camera mycam;
  auto name = mycam.getName();
  log(name);
  mycam.Init();
  mycam.Snap();
  return Napi::String::New(env, name);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  log("Init enter");
  exports.Set(Napi::String::New(env, "hello"),
              Napi::Function::New(env, Method));
  return exports;
}

NODE_API_MODULE(hello, Init)