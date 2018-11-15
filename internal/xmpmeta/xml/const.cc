#include "xmpmeta/xml/const.h"

namespace photos_editing_formats {
namespace xml {

const char* XmlConst::EncodingStr() { return "UTF-8"; }

// RDF metadata constants.
const char* XmlConst::RdfAbout() { return "about"; }

const char* XmlConst::RdfDescription() { return "Description"; }

const char* XmlConst::RdfNodeName() { return "RDF"; }

const char* XmlConst::RdfNodeNs() {
  return "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
}

const char* XmlConst::RdfPrefix() { return "rdf"; }

const char* XmlConst::RdfSeq() { return "Seq"; }

const char* XmlConst::RdfLi() { return "li"; }

// XML metadata constants.
const char* XmlConst::NsAttrName() { return "xmlns"; }

const char* XmlConst::Separator() { return ":"; }

const char* XmlConst::Version() { return "1.0"; }

}  // namespace xml
}  // namespace photos_editing_formats
