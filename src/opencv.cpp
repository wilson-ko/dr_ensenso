// Must include opencv version information before nxLib, so make it the first include.
#include <opencv2/opencv.hpp>
#include "opencv.hpp"
#include "util.hpp"

namespace dr {

cv::Mat toCvMat(NxLibItem const & item, std::string const & what) {
	int error = 0;
	cv::Mat result;
	item.getBinaryData(&error, result, nullptr);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }

	// convert RGB output from camera to OpenCV standard (BGR)
	if (result.channels() == 3) {
		cv::cvtColor(result, result, cv::COLOR_RGB2BGR);
	}
	return result;
}

cv::Mat toCameraMatrix(NxLibItem const & item, std::string const & camera, std::string const & what) {
	int error = 0;
	cv::Mat result = cv::Mat::zeros(3, 3, CV_64F);
	for (std::size_t i=0; i<3; i++) {
		for (std::size_t j=0; j<3; j++) {
			result.at<double>(i,j) = item[itmMonocular][camera == "Left" ? itmLeft : itmRight][itmCamera][j][i].asDouble(&error);
			if (static_cast<bool>(error)) { throw NxError(item, error, what); }
		}
	}
	return result;
}

cv::Mat toDistortionParameters(NxLibItem const & item, std::string const & camera, std::string const & what) {
	int error = 0;
	cv::Mat result = cv::Mat::zeros(8, 1, CV_64F);
	for (std::size_t i=0; i<5; i++) {
		result.at<double>(i) = item[itmMonocular][camera == "Left" ? itmLeft : itmRight][itmDistortion][i].asDouble(&error);
		if (static_cast<bool>(error)) { throw NxError(item, error, what); }
	}
	return result;
}

cv::Mat toRectificationMatrix(NxLibItem const & item, std::string const & camera, std::string const & what) {
	int error = 0;
	cv::Mat result = cv::Mat::zeros(3, 3, CV_64F);
	for (std::size_t i=0; i<3; i++) {
		for (std::size_t j=0; j<3; j++) {
			result.at<double>(i,j) = item[itmStereo][camera == "Left" ? itmLeft : itmRight][itmRotation][j][i].asDouble(&error);
			if (static_cast<bool>(error)) { throw NxError(item, error, what); }
		}
	}
	return result;
}

cv::Mat toProjectionMatrix(NxLibItem const & item, std::string const & camera, std::string const & what) {
	int error = 0;
	cv::Mat result = cv::Mat::zeros(3, 4, CV_64F);
	for (std::size_t i=0; i<3; i++) {
		for (std::size_t j=0; j<3; j++) {
			result.at<double>(i,j) = item[itmStereo][camera == "Left" ? itmLeft : itmRight][itmCamera][j][i].asDouble(&error);
			if (static_cast<bool>(error)) { throw NxError(item, error, what); }
		}
	}

	if (camera == "Right") {
		constexpr double DIVIDER = 1000.0;
		double B = item[itmStereo][itmBaseline].asDouble() / DIVIDER; //NOLINT
		double fx = result.at<double>(0,0);
		result.at<double>(0,3) = (-fx * B);
	}
	return result;
}

void toNxLibItem(NxLibItem const & item, cv::Mat const & value, std::string const & what) {
	int error = 0;
	item.setBinaryData(&error, value);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }
}

} //namespace dr
