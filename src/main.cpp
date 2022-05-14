#include "main.hpp"

// #define _AB_DEBUG_

int main() {
	Init();
	long int brightness = 0, last = -1;
	int i = 0, j = 0;
	cv::Mat frame;
	unsigned int counter = 0;
	int effective = 5;
	system("cls");

	while (true) {
		cv::VideoCapture capture(0);
		capture.set(cv::CAP_PROP_FRAME_HEIGHT, 32);
		capture.set(cv::CAP_PROP_FRAME_WIDTH, 32);
		capture >> frame;

		resize(frame, frame, cv::Size(32, 32));
		frame.convertTo(frame, cv::COLOR_BGR2HSV);

		brightness = 0;
		for (i = 0; i < frame.rows; ++i) {
			for (j = 0; j < frame.cols; ++j) {
				brightness += frame.at<cv::Vec3b>(i, j)[2];
			}
		}
		brightness /= (frame.cols * frame.rows);

#ifndef _AB_DEBUG_

		if (counter++ % 100 == 0) {
			system("cls");
		}

#else

		imshow("win", frame);
		cout << brightness << endl;

#endif // _AB_DEBUG_

		if (last == -1) {
			last = brightness;
		}

		auto delta = last - brightness;

		if (delta != 0) {
			brightness += delta * 0.5 + effective;
			brightness = min(255, brightness);
			cout << "del:" << delta<<"\tbrightness:" << brightness<<endl;
			SetBrightness(brightness * 100 / 255);
			last = brightness;
		}
		capture.release();
		frame.release();

		//GammaRamp.SetBrightness(NULL, brightness + 128);
		Sleep(1000);
	}
}