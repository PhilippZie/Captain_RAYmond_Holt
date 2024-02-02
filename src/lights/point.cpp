#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {

    Color power;
    Point position;
public:
    PointLight(const Properties &properties) {
        power = properties.get<Color>("power",Color(0));
        position = properties.get<Point>("position",Point(0));
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // calculate the distance between the light and the shadow ray origin
        Vector wi(position - origin);
        float distance = wi.length();
        // calculate the intensity of the light
        Color intensity = power * Inv4Pi;
        // calculate the weight
        Color weight = intensity / (distance * distance);

        return DirectLightSample{wi.normalized(), weight, distance};
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("PointLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
