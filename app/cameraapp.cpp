#include <iostream>
#include "camera.hpp"

// Namespace for using cout.
using namespace std;

using namespace camerautils;

int main(int argc, char* argv[])
{
	cout<<"Hello from camerautil" << endl;

	Camera mycam;

	cout << "Camera name is " << mycam.getName() << endl;

	cout << "Initializing camera...";
	mycam.Init();
	mycam.Snap();

	return 0;
}