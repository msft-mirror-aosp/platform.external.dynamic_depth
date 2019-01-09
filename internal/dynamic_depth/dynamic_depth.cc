#include "dynamic_depth/dynamic_depth.h"

#include <fstream>
#include <sstream>

#include "android-base/logging.h"
#include "dynamic_depth/container.h"
#include "dynamic_depth/item.h"
#include "image_io/gcontainer/gcontainer.h"
#include "xmpmeta/xmp_data.h"
#include "xmpmeta/xmp_parser.h"
#include "xmpmeta/xmp_writer.h"

namespace dynamic_depth {
namespace {

using ::dynamic_depth::xmpmeta::CreateXmpData;
using ::dynamic_depth::xmpmeta::XmpData;

constexpr char kImageMimePrefix[] = "image";

bool IsMimeTypeImage(const string& mime) {
  string mime_lower = mime;
  std::transform(mime_lower.begin(), mime_lower.end(), mime_lower.begin(),
                 ::tolower);
  return strncmp(mime_lower.c_str(), kImageMimePrefix, mime_lower.find("/")) ==
         0;
}

}  // namespace

bool WriteImageAndMetadataAndContainer(std::istream* input_jpeg_stream,
                                       Device* device,
                                       std::ostream* output_jpeg_stream) {
  const std::unique_ptr<XmpData> xmp_data = CreateXmpData(true);
  device->SerializeToXmp(xmp_data.get());
  bool success =
      WriteLeftEyeAndXmpMeta(*xmp_data, input_jpeg_stream, output_jpeg_stream);

  if (device->GetContainer() == nullptr) {
    return success;
  }

  // Append Container:Item elements' payloads.
  for (auto item : device->GetContainer()->GetItems()) {
    const string& payload = item->GetPayloadToSerialize();
    const unsigned int payload_size = item->GetLength();
    if (payload_size <= 0 || payload.empty()) {
      continue;
    }
    output_jpeg_stream->write(payload.c_str(), payload_size);
  }

  return success;
}

bool WriteImageAndMetadataAndContainer(const string& out_filename,
                                       const uint8_t* primary_image_bytes,
                                       size_t primary_image_bytes_count,
                                       Device* device) {
  std::istringstream input_jpeg_stream(
      std::string(reinterpret_cast<const char*>(primary_image_bytes),
                  primary_image_bytes_count));
  std::ofstream output_jpeg_stream;
  output_jpeg_stream.open(out_filename, std::ostream::out);
  bool success = WriteImageAndMetadataAndContainer(&input_jpeg_stream, device,
                                                   &output_jpeg_stream);
  output_jpeg_stream.close();
  return success;
}

bool GetItemPayload(const string& input_image_filename, const Device* device,
                    const string& item_uri, string* out_payload) {
  if (device == nullptr || device->GetContainer() == nullptr) {
    LOG(ERROR) << "No Container element to parse";
    return false;
  }

  return GetItemPayload(input_image_filename, device->GetContainer(), item_uri,
                        out_payload);
}

bool GetItemPayload(const string& input_image_filename,
                    const Container* container, const string& item_uri,
                    string* out_payload) {
  if (container == nullptr) {
    LOG(ERROR) << "Container cannot be null";
    return false;
  }

  size_t file_offset = 0;
  size_t file_length = 0;
  int index = 0;
  bool is_mime_type_image = false;
  for (const auto& item : container->GetItems()) {
    is_mime_type_image = IsMimeTypeImage(item->GetMime());

    if (item_uri.compare(item->GetDataUri()) == 0) {
      // Found a matching item.
      file_length = item->GetLength();
      break;
    }

    file_offset += item->GetLength();
    index++;
  }

  if (file_length == 0) {
    if (index == 0 && is_mime_type_image) {
      LOG(INFO) << "Item references the primary image, Not populating data";
      return true;
    }

    // File length can be zero to indicate the primary image (checked above) or
    // use the last file in the list. If this check fails, it's not in this
    // state.
    if (file_offset == 0) {
      LOG(ERROR) << "Not using the primary image, or not image mime, or not "
                    "the first item, but has file offset of 0";
      return false;
    }
  }

  std::string std_payload;
  bool success =
      ::photos_editing_formats::image_io::gcontainer::ParseFileAfterImage(
          input_image_filename, file_offset, file_length, &std_payload);
  *out_payload = std_payload;
  return success;
}

extern "C" int32_t ValidateAndroidDynamicDepthBuffer(const char* buffer, size_t buffer_length) {
  XmpData xmp_data;
  const string image_data(buffer, buffer_length);
  ReadXmpFromMemory(image_data, /*XmpSkipExtended*/ false, &xmp_data);

  // Check device presence
  std::unique_ptr<Device> device = Device::FromXmp(xmp_data);
  if (device == nullptr) {
    LOG(ERROR) << "Dynamic depth device element not present!";
    return -1;
  }

  // Check profiles
  const Profiles* profiles = device->GetProfiles();
  if (profiles == nullptr) {
    LOG(ERROR) << "No Profile found in the dynamic depth metadata";
    return -1;
  }

  const std::vector<const Profile*> profile_list = profiles->GetProfiles();
  // Stop at the first depth photo profile found.
  bool depth_photo_profile_found = false;
  int camera_index = 0;
  for (auto profile : profile_list) {
    depth_photo_profile_found = !profile->GetType().compare("DepthPhoto");
    if (depth_photo_profile_found) {
      // Use the first one if available.
      auto indices = profile->GetCameraIndices();
      if (!indices.empty()) {
        camera_index = indices[0];
      } else {
        camera_index = -1;
      }
      break;
    }
  }

  if (!depth_photo_profile_found || camera_index < 0) {
    LOG(ERROR) << "No dynamic depth profile found";
    return -1;
  }

  auto cameras = device->GetCameras();
  if (cameras == nullptr || camera_index > cameras->GetCameras().size() ||
      cameras->GetCameras()[camera_index] == nullptr) {
    LOG(ERROR) << "No camera or depth photo data found";
    return -1;
  }

  auto camera = cameras->GetCameras()[camera_index];
  auto depth_map = camera->GetDepthMap();
  if (depth_map == nullptr) {
    LOG(ERROR) << "No depth map found";
    return -1;
  }

  auto depth_uri = depth_map->GetDepthUri();
  if (depth_uri.empty()) {
    LOG(ERROR) << "Invalid depth map URI";
    return -1;
  }

  auto depth_units = depth_map->GetUnits();
  if (depth_units != dynamic_depth::DepthUnits::kMeters) {
    LOG(ERROR) << "Unexpected depth map units";
    return -1;
  }

  auto depth_format = depth_map->GetFormat();
  if (depth_format != dynamic_depth::DepthFormat::kRangeInverse) {
    LOG(ERROR) << "Unexpected depth map format";
    return -1;
  }

  auto near = depth_map->GetNear();
  auto far = depth_map->GetFar();
  if ((near < 0.f) || (far < 0.f) || (near > far) || (near == far)) {
    LOG(ERROR) << "Unexpected depth map near and far values";
    return -1;
  }

  auto confidence_uri = depth_map->GetConfidenceUri();
  if (confidence_uri.empty()) {
    LOG(ERROR) << "No confidence URI";
    return -1;
  }

  return 0;
}

}  // namespace dynamic_depth
