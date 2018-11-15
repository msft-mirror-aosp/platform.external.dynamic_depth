#ifndef DYNAMIC_DEPTH_INCLUDES_XMPMETA_XMP_CONST_H_  // NOLINT
#define DYNAMIC_DEPTH_INCLUDES_XMPMETA_XMP_CONST_H_  // NOLINT

namespace photos_editing_formats {

// Constants used in writing XMP metadata.
struct XmpConst {
  // XMP namespaces.
  static const char* Namespace();
  static const char* NamespacePrefix();
  static const char* NodeName();
  static const char* AdobePropName();
  static const char* AdobePropValue();
  static const char* NoteNamespace();

  // XMP headers.
  static const char* Header();
  static const char* ExtensionHeader();
  static const char* HasExtensionPrefix();
  static const char* HasExtension();

  // Sizes.
  static const int ExtensionHeaderOffset();
  static const int MaxBufferSize();
  static const int ExtendedMaxBufferSize();
};

}  // namespace photos_editing_formats

#endif // DYNAMIC_DEPTH_INCLUDES_XMPMETA_XMP_CONST_H_  // NOLINT
