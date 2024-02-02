#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
    Color intensity;
    Vector direction;
public:
    DirectionalLight(const Properties &properties) {
        intensity = properties.get<Color>("intensity", Color::black());
        direction = properties.get<Vector>("direction");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // light source to infinity and beyond
        return DirectLightSample{direction.normalized(), intensity, Infinity};
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("DirectionalLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
