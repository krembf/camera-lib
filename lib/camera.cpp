#include <fstream>
#include <thread>
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

void Camera::Snap(uint8_t *buffer, uint32_t bufferSize)
{
	// This smart pointer will receive the grab result data.
	CGrabResultPtr ptrGrabResult;

	const auto width = 960;
	const auto height = 600;
	const auto byteDepth = 4; //RGBA
	const auto expectedBufferSize = width * height * byteDepth;

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
		return;
	}

	// validate image buffer size against camera image dimensions and byteDepth;
	if (bufferSize != expectedBufferSize)
	{
		cerr << "Could not grab an image: Invalid buffer size supplied " << buffer << " " << expectedBufferSize << endl;
		log("Exception occured");
		return;
	}

	if (ptrGrabResult && ptrGrabResult->GrabSucceeded())
	{
		// Create a pylon image.
		CPylonImage image;
		CPylonImage userImage;
		// todo: get the parameters as function arguments
		userImage.AttachUserBuffer(buffer, bufferSize, Pylon::EPixelType::PixelType_BGRA8packed, width, height, 0);
		Pylon::CImageFormatConverter converter;
		converter.OutputPixelFormat = PixelType_BGRA8packed;
		converter.OutputBitAlignment = OutputBitAlignment_MsbAligned;
		cout << "Converter created ...";

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
		converter.Convert(userImage, image);

		// for debug only at this point
		// userImage.Save(EImageFileFormat::ImageFileFormat_Png, "/tmp/mypng.png");

		return;
	}
}

// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 1000;

void Camera::SnapContinuous(CameraSettings cameraSettings, uint8_t *buffer, uint32_t bufferSize, std::function<void()> cb)
{
	// This smart pointer will receive the grab result data.
	CGrabResultPtr ptrGrabResult;

	const auto width = cameraSettings.ImageWidth;
	const auto height = cameraSettings.ImageHeight;
	const auto byteDepth = cameraSettings.ByteDepth; //RGBA
	const auto expectedBufferSize = width * height * byteDepth;
	const auto numberOfBuffers = 4;
	auto bufferIndex = 0;
	auto skipIndex = 0;

	try
	{
		CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
		camera.Open();
		CIntegerParameter camwidth(camera.GetNodeMap(), "Width");
		camwidth.SetValue(width);
		CIntegerParameter camheight(camera.GetNodeMap(), "Height");
		camheight.SetValue(height);		
		camera.StartGrabbing(c_countOfImagesToGrab);

		while (camera.IsGrabbing())
		{
			// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Currently skipping 2/3 of the images because UI cannot keep up with the 
			// buffer updates, so needs some optimization on the rendering part in order
			// to remove this restriction and deliver as much images as possible.
			// if(skipIndex++ %3 != 0)
			// {
			// 	continue;
			// }

			// validate image buffer size against camera image dimensions and byteDepth;
			if (bufferSize != expectedBufferSize)
			{
				cerr << "Could not grab an image: Invalid buffer size supplied " << buffer << " " << expectedBufferSize << endl;
				log("Exception occured");
				return;
			}

			if (ptrGrabResult && ptrGrabResult->GrabSucceeded())
			{
				// Create a pylon image.
				CPylonImage image;
				CPylonImage userImage;
				// todo: get the parameters as function arguments
				cout << "Attaching user buffer at index " << bufferIndex << endl;
				userImage.AttachUserBuffer(buffer + bufferIndex * bufferSize, bufferSize, Pylon::EPixelType::PixelType_BGRA8packed, width, height, 0);
				Pylon::CImageFormatConverter converter;
				converter.OutputPixelFormat = PixelType_BGRA8packed;
				converter.OutputBitAlignment = OutputBitAlignment_MsbAligned;
				cout << "Converter created ...";

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
				converter.Convert(userImage, image);

				cb();
				bufferIndex = (bufferIndex + 1) % numberOfBuffers;
				// std::this_thread::sleep_for(std::chrono::milliseconds(150));
			}
		}
	}
	catch (const GenericException &e)
	{
		cerr << "Could not grab an image: " << endl
			 << e.GetDescription() << endl;
		log("Exception occured");
		log(e.GetDescription());
		return;
	}
}

Camera::~Camera()
{
}
} // namespace camerautils
