#include <iostream>
#include <exception>

#include <tango.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "VideoCaptureClient.h"

Tango::DeviceProxy* device = nullptr;

vcc::CameraMode cam_mode;

unsigned char* image_byte = nullptr;

Tango::DeviceAttribute devAttr;
Tango::EncodedAttribute enAttr;

cv::Mat image_mat;
cv::Mat image_converted;

void event_function_Jpeg(Tango::EventData* event_data);
void event_function_Frame(Tango::EventData* event_data);

int main(int argc, char* argv[])
{
	try
	{
		device = new Tango::DeviceProxy(argc < 2 ? "CVCam/test/0" : argv[1]);
		int update_time = argc < 3 ? 50 : std::stoi(argv[2]);

		vcc::print_device_info(std::cout, device);
		
		cam_mode = vcc::get_device_camera_mode(device);

		if (cam_mode == vcc::CameraMode::None)
		{
			throw std::exception("Invalid camera mode");
		}

		//vcc::JpegCallBack callBack(event_function_Jpeg);
		//int event_id = device->subscribe_event(std::string("Jpeg"), Tango::CHANGE_EVENT, &callBack, std::vector<std::string>());

		//vcc::JpegCallBack callBack(event_function_Frame);
		//int event_id = device->subscribe_event(std::string("Frame"), Tango::CHANGE_EVENT, &callBack, std::vector<std::string>());

		do
		{
			event_function_Jpeg(nullptr);
		} while (cv::waitKey(update_time) != 0x1B);

		//device->unsubscribe_event(event_id);
	}
	catch (Tango::DevFailed& e)
	{
		delete[] device;
		delete[] image_byte;
		Tango::Except::print_exception(e);
		system("pause");
		return 1;
	}
	catch (std::exception& e)
	{
		delete[] device;
		delete[] image_byte;
		std::cout << e.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		delete[] device;
		delete[] image_byte;
		std::cout << "Unknown exception" << std::endl;
		return 3;
	}

	delete[] image_byte;

	return 0;
}

void event_function_Jpeg(Tango::EventData* event_data)
{
	int width, height;

	delete[] image_byte;
	image_byte = nullptr;

	devAttr = device->read_attribute("Jpeg");

	switch (cam_mode)
	{
	case vcc::CameraMode::RGB:
		enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);
		image_mat = cv::Mat(height, width, CV_8UC4, image_byte);
		cv::cvtColor(image_mat, image_converted, cv::COLOR_RGBA2BGR);
		break;
	case vcc::CameraMode::BGR:
		enAttr.decode_rgb32(&devAttr, &width, &height, &image_byte);
		image_mat = cv::Mat(height, width, CV_8UC4, image_byte);
		cv::cvtColor(image_mat, image_converted, cv::COLOR_BGRA2BGR);
		break;
	case vcc::CameraMode::Grayscale:
		enAttr.decode_gray8(&devAttr, &width, &height, &image_byte);
		image_mat = cv::Mat(height, width, CV_8UC1, image_byte);
		cv::cvtColor(image_mat, image_converted, cv::COLOR_GRAY2BGR);
		break;
	default:
		break;
	}

	cv::imshow("Image", image_converted);
}

void event_function_Frame(Tango::EventData* event_data)
{
	std::cout << event_data->attr_name << std::endl;
}
