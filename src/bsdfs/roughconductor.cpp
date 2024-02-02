#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        Color reflectance = m_reflectance->evaluate(uv);
        // microfacet normal
        Vector normal = (wi + wo) / (wi + wo).length();
        // calculate g1 for wi and wo (smith masking-shadowing function)
        float g1_wi = microfacet::smithG1(alpha, normal, wi);
        float g1_wo = microfacet::smithG1(alpha, normal, wo);
        // calculate the microfacet distribution
        float d_wm = microfacet::evaluateGGX(alpha, normal);
        // fraction of the formula
        float fraction = 1/ abs(4 * Frame::cosTheta(wo));
        // implement the formula from the task sheet
        Color weight = reflectance * g1_wi * g1_wo * d_wm * fraction;

        return BsdfEval{weight};
        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        // reflectance R
        Color reflectance = m_reflectance->evaluate(uv);
        // microfacet normal
        Vector normal = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        // calculate wi with the sampled microfacet normal
        Vector wi = reflect(wo, normal);
        // calculate g1
        float g1 = microfacet::smithG1(alpha, normal, wi);
        // the final weight is the reflectance times the g1 term
        Color weight = reflectance * g1;

        return BsdfSample{wi, weight};


        // hints:
        // * do not forget to cancel out as many terms from your equations as possible!
        //   (the resulting sample weight is only a product of two factors)
    }

    std::string toString() const override {
        return tfm::format("RoughConductor[\n"
                           "  reflectance = %s,\n"
                           "  roughness = %s\n"
                           "]",
                           indent(m_reflectance), indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
