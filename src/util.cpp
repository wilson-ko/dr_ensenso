#include "util.hpp"

#include <fstream>
#include <sstream>

#include <fmt/format.h>

namespace dr {

std::optional<NxLibItem> findCameraBySerial(std::string const & serial) {
	NxLibItem camera = NxLibItem{}[itmCameras][itmBySerialNo][serial];
	if (!camera.exists()) {
		return {};
	}
	return camera;
}

std::optional<NxLibItem> findCameraByEepromId(int eeprom_id) {
	NxLibItem camera = NxLibItem{}[itmCameras][itmByEepromId][eeprom_id];
	if (!camera.exists()) { return {}; }
	return camera;
}

std::optional<NxLibItem> findCameraByLink(std::string const & linked_to, LogFunction const & logger) {
	if (logger) { logger(fmt::format("Looking for camera linked to target {}.", linked_to)); }
	NxLibItem cameras = NxLibItem{}[itmCameras][itmBySerialNo];
	for (int i = 0; i < cameras.count(); ++i) {
		std::string serial = getNx<std::string>(cameras[i][itmSerialNumber]);
		NxLibItem camera = cameras[serial];

		if (!camera[itmLink][itmTarget].exists()) {
			if (logger) { logger(fmt::format("Camera with serial {} is not linked to anything.")); }
			continue;
		}
		std::string target = getNx<std::string>(camera[itmLink][itmTarget]);
		if (target == linked_to) {
			if (logger) { logger(fmt::format("Camera with serial {} is linked our target: {}", serial, target)); }
			return camera;
		}
		if (logger) { logger(fmt::format("Camera with serial {} is not linked to our target: {}", serial, target)); }
	}
	if (logger) { logger(fmt::format("No camera linked to our target {}  found.", linked_to)); }
	return {};
}

std::optional<NxLibItem> findCameraByType(std::string const & wanted_type, LogFunction const & logger) {
	if (logger) { logger(fmt::format("Looking for camera of type {}.", wanted_type)); }
	NxLibItem cameras = NxLibItem{}[itmCameras][itmBySerialNo];
	for (int i = 0; i < cameras.count(); ++i) {
		std::string serial = getNx<std::string>(cameras[i][itmSerialNumber]);
		NxLibItem camera = cameras[serial];

		std::string type = getNx<std::string>(camera[itmType]);

		if (type == wanted_type) {
			if (logger) { logger(fmt::format("Camera with serial {} is the wanted type: {}", serial, type)); }
			return camera;
		}
		if (logger) { logger(fmt::format("Camera with serial {} is not the wanted type: {}", serial, type)); }
	}
	if (logger) { logger(fmt::format("No camera of the wanted type {} found.", wanted_type)); }
	return {};
}

namespace {
	/// Open an optional camera, or return nothing.
	std::optional<NxLibItem> openCamera(std::optional<NxLibItem> camera) {
	if (!camera) { return {}; }

		NxLibCommand command(cmdOpen);
		setNx(command.parameters()[itmCameras], getNx<std::string>((*camera)[itmSerialNumber]));
		executeNx(command);

		return camera;
	}
} //namespace

std::optional<NxLibItem> openCameraBySerial(std::string const & serial) {
	return openCamera(findCameraBySerial(serial));
}

std::optional<NxLibItem> openCameraByEepromId(int eeprom_id) {
	return openCamera(findCameraByEepromId(eeprom_id));
}

std::optional<NxLibItem> openCameraByLink(std::string const & serial, LogFunction const & logger) {
	return openCamera(findCameraByLink(serial, logger));
}

std::optional<NxLibItem> openCameraByType(std::string const & type, LogFunction const &  logger) {
	return openCamera(findCameraByType(type, logger));
}

void executeNx(NxLibCommand const & command, std::string const & what) {
	int error = 0;
	command.execute(&error);
	if (static_cast<bool>(error)) { throwCommandError(error, what); }
}

std::int64_t getNxBinaryTimestamp(NxLibItem const & item, std::string const & what) {
	int error = 0;
	double timestamp = 0;
	item.getBinaryDataInfo(&error, nullptr, nullptr, nullptr, nullptr, nullptr, &timestamp);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }
	constexpr double TIME_SHIFT = 11644473600.0;
	constexpr double MILLION = 1e6;
	return (timestamp - TIME_SHIFT) *  MILLION; // Correct for epoch and turn into microseconds.
}

void setNxJson(NxLibItem const & item, std::string const & json, std::string const & what) {
	int error = 0;
	item.setJson(&error, json, true);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }
}

bool setNxJsonFromFile(NxLibItem const & item, std::string const & filename, std::string const & what) {
	std::ifstream file;
	file.open(filename);

	if (!file.good()) {
		return false;
	}

	file.exceptions(std::ios::failbit | std::ios::badbit);

	std::stringstream buffer;
	buffer << file.rdbuf();
	setNxJson(item, buffer.str(), what);

	return true;
}

std::string getNxJson(NxLibItem const & item, std::string const & what) {
	int error = 0;
	std::string result = item.asJson(&error, true);
	if (static_cast<bool>(error)) { throw NxError(item, error, what); }
	return result;
}

void writeNxJsonToFile(NxLibItem const & item, std::string const & filename, std::string const & what) {
	std::ofstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);
	file.open(filename);
	file << getNxJson(item, what);
}

} //namespace dr
