#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {

    ref<Instance> child;
public:
    AreaLight(const Properties &properties) {
        child = properties.getChild<Instance>();
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        AreaSample sample = child->sampleArea(rng);
        Vector wi = sample.position - origin;
        float distance = wi.length();
        wi = wi.normalized();
        Vector wo = sample.frame.toLocal(-wi);

        Color weight = child->emission()->evaluate(sample.uv, wo).value;
        /*
        Vector normal = sample.frame.normal;
        float cos = normal.dot(wo) / (wo.length() * normal.length());
        weight /= cos;
        */
        weight = weight / (sqr(distance) * sample.pdf);
        weight *= Frame::absCosTheta(wo);
        
        // DirectLightSample has vector wi, color weight, float distance
        return DirectLightSample{wi, weight, distance};
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("AreaLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")
