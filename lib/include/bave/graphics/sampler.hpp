#pragma once

namespace bave {
struct Sampler {
	enum class Wrap : int { eRepeat, eClampEdge, eClampBorder };
	enum class Filter : int { eLinear, eNearest };
	enum class Border : int { eOpaqueBlack, eOpaqueWhite, eTransparentBlack };

	Wrap wrap_s{Wrap::eRepeat};
	Wrap wrap_t{Wrap::eRepeat};
	Filter min{Filter::eLinear};
	Filter mag{Filter::eLinear};
	Border border{Border::eOpaqueBlack};

	auto operator==(Sampler const&) const -> bool = default;
};
} // namespace bave
