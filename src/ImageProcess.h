#pragma once

#include "opencv2/opencv.hpp"

double contrast(cv::Mat frame) {
	cv::Mat gray;
	int acc = 0;
	frame.convertTo(gray, cv::COLOR_BGR2GRAY);
	for (int i = 0; i < gray.rows; ++i) {
		for (int j = 0; j < gray.cols; ++j) {
			if (i == 0) {

			}
		}
	}
}