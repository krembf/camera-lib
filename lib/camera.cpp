#include <fstream>
#include "camera.hpp"

//Include files to use the pylon API.
#include <pylon/PylonIncludes.h>

//Namespace for using pylon objects.
using namespace Pylon;
using namespace GenApi;

using namespace std;

static void log(const char *text)
{
	std::ofstream logFile("logfile2.txt", std::ofstream::out);
	logFile << text << endl;
	logFile.flush();
}

// This is a helper function for printing image properties.
static void PrintImageProperties(IImage &image)
{
	std::cout
		<< "Buffer: " << image.GetBuffer()
		<< " Image Size: " << image.GetImageSize()
		<< " Width: " << image.GetWidth()
		<< " Height: " << image.GetHeight()
		<< " Unique: " << image.IsUnique()
		<< " PixelType: " << image.GetPixelType()
		<< endl;
}

namespace camerautils
{
Camera::Camera(/* args */)
{
	log("camera constructor");
}

void Camera::Init()
{
	PylonInitialize();
	log("pylon initialized");
}

void Camera::DeInit()
{
	// Releases all pylon resources.
	PylonTerminate();
	log("pylon terminated");
}

std::string Camera::getName()
{
	return "Basler";
}

void Camera::Snap(uint8_t *buffer)
{
	// This smart pointer will receive the grab result data.
	CGrabResultPtr ptrGrabResult;

	try
	{
		CInstantCamera Camera(CTlFactory::GetInstance().CreateFirstDevice());
		Camera.GrabOne(1000, ptrGrabResult);
	}
	catch (const GenericException &e)
	{
		cerr << "Could not grab an image: " << endl
			 << e.GetDescription() << endl;
		log("Exception occured");
		log(e.GetDescription());
	}

	if (ptrGrabResult && ptrGrabResult->GrabSucceeded())
	{
		// Create a pylon image.
		CPylonImage image;
		CPylonImage userImage;
		// todo: get the parameters as function arguments
		userImage.AttachUserBuffer(buffer, 576000, Pylon::EPixelType::PixelType_Mono8, 960, 600, 0);

		// A pylon grab result class CGrabResultPtr provides a cast operator to IImage.
		// That's why it can be used like an image, e.g. to print its properties or
		// to show it on the screen.
		std::cout << endl
				  << "The properties of the grabbed image." << endl;
		PrintImageProperties(ptrGrabResult);

		// Initializes the image object with the buffer from the grab result.
		// This prevents the reuse of the buffer for grabbing as long as it is
		// not released.
		// Please note that this is not relevant for this example because the
		// camera object has been destroyed already.
		image.AttachGrabResultBuffer(ptrGrabResult);
		std::cout << endl
				  << "The properties of an image with an attached grab result." << endl;
		PrintImageProperties(image);

		// Now the grab result can be released. The grab result buffer is now
		// only held by the pylon image.
		ptrGrabResult.Release();
		std::cout << "After the grab result has been released." << endl;
		PrintImageProperties(image);

		uint8_t *pImageBuffer = (uint8_t *)image.GetBuffer();
		cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl
			 << endl;

		// Copy the image from the grabbed into user one
		userImage.CopyImage(image);
		cout << "Returning image buffer" << endl;

		return;
	}
}

Camera::~Camera()
{
}
} // namespace camerautils
