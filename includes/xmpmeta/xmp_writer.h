#ifndef DYNAMIC_DEPTH_INCLUDES_XMPMETA_XMP_WRITER_H_  // NOLINT
#define DYNAMIC_DEPTH_INCLUDES_XMPMETA_XMP_WRITER_H_  // NOLINT

#include <iostream>
#include <memory>
#include <string>

#include "base/port.h"
#include "xmpmeta/xmp_data.h"

namespace photos_editing_formats {

// Creates a new XmpData object and initializes the boilerplate for the
// standard XMP section.
// The extended section is initialized only if create_extended is true.
std::unique_ptr<XmpData> CreateXmpData(bool create_extended);

// Writes  XMP data to an existing JPEG image file.
// This is equivalent to writeXMPMeta in geo/lightfield/metadata/XmpUtil.java.
// If the extended section is not null, this will modify the given XmpData by
// setting a property in the standard section that links it with the
// extended section.
bool WriteLeftEyeAndXmpMeta(const string& left_data, const string& filename,
                            const XmpData& xmp_data);

// Same as above, but allows the caller to manage their own istream and ostream.
// filename is written to only if metadata serialization is successful.
// Assumes the caller will take care of opening and closing the
// output_jpeg_stream, as well as initialization of the input_jpeg_stream.
bool WriteLeftEyeAndXmpMeta(const string& filename, const XmpData& xmp_data,
                            std::istringstream* input_jpeg_stream,
                            std::ofstream* output_jpeg_stream);

// Updates a JPEG input stream with new XMP data and writes it to an
// output stream.
// This is equivalent to writeXMPMeta in geo/lightfield/metadata/XmpUtil.java.
bool AddXmpMetaToJpegStream(std::istream* input_jpeg_stream,
                            const XmpData& xmp_data,
                            std::ostream* output_jpeg_stream);

}  // namespace photos_editing_formats

#endif // DYNAMIC_DEPTH_INCLUDES_XMPMETA_XMP_WRITER_H_  // NOLINT
