namespace mango {

template <typename componentType>
componentType getNormalizePixelValue()
{
  static_assert(!std::is_scalar<componentType>::value, "not implement for value type .");
  static_assert(std::is_scalar<componentType>::value, "not implement for value type .");
  return 1;
}

template <> float getNormalizePixelValue() { return 1.0f; }
template <> double getNormalizePixelValue() { return 1.0f; }
template <> uint8_t getNormalizePixelValue() { return 255; }

// Adds padding to multi-channel interleaved data by inserting dummy values, or
// discards trailing channels. This is useful for platforms that only accept
// 4-component data, since users often wish to submit (or receive) 3-component
// data.
template <typename componentType>
void reshapeImageData(void *dest, const void *src, size_t srcChannelCount,
          size_t dstChannelCount, size_t numSrcBytes) {
  const componentType normalize_value = getNormalizePixelValue<componentType>();
  const componentType *in = (const componentType *)src;
  componentType *out = (componentType *)dest;
  const size_t pixels_count = (numSrcBytes / sizeof(componentType)) / srcChannelCount;
  const int minChannelCount = (srcChannelCount < dstChannelCount) ? srcChannelCount : dstChannelCount;
  for (size_t index = 0; index < pixels_count; ++index) {
    for (size_t channel = 0; channel < minChannelCount; ++channel) {
      out[channel] = in[channel];
    }
    for (size_t channel = srcChannelCount; channel < dstChannelCount;
         ++channel) {
      out[channel] = normalize_value;
    }
    in += srcChannelCount;
    out += dstChannelCount;
  }
}

} // namespace mango