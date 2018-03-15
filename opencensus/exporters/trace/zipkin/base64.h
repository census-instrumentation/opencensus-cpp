#ifndef OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_BASE64_H_
#define OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_BASE64_H_

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace opencensus {
namespace base64 {

const std::string base64_padding[] = {"", "==", "="};

inline const std::string Encode(const uint8_t *data, size_t size) {
  using namespace boost::archive::iterators;
  typedef const uint8_t *iterator_type;
  typedef base64_from_binary<transform_width<iterator_type, 6, 8>> base64_enc;
  std::stringstream ss;
  std::copy(base64_enc(data), base64_enc(data + size),
            std::ostream_iterator<char>(ss));
  ss << base64_padding[size % 3];
  return ss.str();
}

inline const std::string Encode(const std::string &text) {
  using namespace boost::archive::iterators;
  typedef std::string::const_iterator iterator_type;
  typedef base64_from_binary<transform_width<iterator_type, 6, 8>> base64_enc;
  std::stringstream ss;
  std::copy(base64_enc(text.begin()), base64_enc(text.end()),
            std::ostream_iterator<char>(ss));
  ss << base64_padding[text.size() % 3];
  return ss.str();
}

inline const std::string Encode(const std::vector<uint8_t> &data) {
  using namespace boost::archive::iterators;
  typedef std::vector<uint8_t>::const_iterator iterator_type;
  typedef base64_from_binary<transform_width<iterator_type, 6, 8>> base64_enc;
  std::stringstream ss;
  std::copy(base64_enc(data.begin()), base64_enc(data.end()),
            std::ostream_iterator<char>(ss));
  ss << base64_padding[data.size() % 3];
  return ss.str();
}

inline const std::string Decode(const std::string &encoded) {
  using namespace boost::archive::iterators;
  typedef std::string::const_iterator iterator_type;
  typedef transform_width<binary_from_base64<iterator_type>, 8, 6> base64_dec;
  return boost::algorithm::trim_right_copy_if(
      std::string(base64_dec(std::begin(encoded)),
                  base64_dec(std::end(encoded))),
      [](char c) { return c == '\0'; });
}

}  // namespace base64
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_BASE64_H_
