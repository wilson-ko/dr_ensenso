#include "pcl.hpp"
#include "util.hpp"

#include <stdexcept>

namespace dr {

namespace {
	/// Conversion from ensenso timestamp to PCL timestamp.
	constexpr double CORRECTION = 11644473600.0;
	constexpr double MULTIPLIER = 1000000.0;
	pcl::uint64_t ensensoStampToPcl(double stamp) { return (stamp - CORRECTION) * MULTIPLIER; }
} //namespace

pcl::PointCloud<pcl::PointXYZ> toPointCloud(NxLibItem const & item, std::string const & what) {
	int error = 0;

	// Retrieve metadata.
	int height;
	int width;
	int channels;
	int element_width;
	bool is_float;
	double timestamp;
	item.getBinaryDataInfo(&error, &width, &height, &channels, &element_width, &is_float, &timestamp);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }

	// Make sure data is what we expect.
	std::string what2 = what.empty() ? std::string() : ": " + what;
	if (channels != 3) { throw std::runtime_error("Unexpected number of channels: " + std::to_string(channels) + ", expected 3" + what2 + "."); }
	if (!is_float) { throw std::runtime_error("Expected floating point data for point cloud conversion" + what2 + "."); }
	if (element_width != 4) { throw std::runtime_error("Unexpected data width: " + std::to_string(element_width) + ", expected 4" + what2 + "."); }

	// Retrieve data.
	std::vector<float> point_list;
	item.getBinaryData(&error, point_list, nullptr);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }

	// Copy point cloud and convert in meters
	pcl::PointCloud<pcl::PointXYZ> cloud;
	cloud.header.stamp    = ensensoStampToPcl(timestamp);
	cloud.header.frame_id = "/camera_link";
	cloud.width           = width;
	cloud.height          = height;
	cloud.is_dense        = false;
	cloud.resize(height * width);

	constexpr double CONVERSION_RATE = 1000.0;
	// Copy data in point cloud (and convert milimeters in meters)
	for (size_t i = 0; i < point_list.size (); i += 3) {
		cloud.points[i / 3].x = static_cast<float>(point_list[i] / CONVERSION_RATE);
		cloud.points[i / 3].y = static_cast<float>(point_list[i + 1] / CONVERSION_RATE);
		cloud.points[i / 3].z = static_cast<float>(point_list[i + 2] / CONVERSION_RATE);
	}

	return cloud;
}

} //namespace dr
