#include <iostream>
#include <vector>
#include "camera.hpp"

// Namespace for using cout.
using namespace std;

using namespace camerautils;

int main(int argc, char *argv[])
{
	cout << "Hello from cameraapp" << endl;

	// The image dimensions
	const auto width = 960;
	const auto height = 600;
	const auto byteDepth = 4; //RGBA
	const auto bufferSize = width * height * byteDepth;

	Camera mycam;
	std::vector<uint8_t> myBuffer(bufferSize);

	cout << "Camera name is " << mycam.getName() << endl;

	cout << "Initializing camera...";
	mycam.Init();
	mycam.Snap(myBuffer.data(), bufferSize);
	cout << "Received image buffer" << endl;
	cout << "Gray value of first pixel: " << (uint32_t)myBuffer.data()[0] << endl
		 << endl;

	mycam.DeInit();

	return 0;
}