#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        // the refraction index
        float ior = m_ior->scalar(uv);

        Vector normal = Vector(0,0,1);

        // if the ray is entering the medium, we need to flip the ior and the normal
        if (Frame::cosTheta(wo) < 0) {
            ior = 1 / ior;
            normal = -normal;
        }
            
        // calculate the fresnel coefficients
        float F = fresnelDielectric(wo.z(), ior);

        // decide whether to reflect or refract
        if (rng.next() < F) {
            // reflecting wo around the normal gives us the direction wi
            Vector wi = reflect(wo, normal);
            return BsdfSample{wi, m_reflectance->evaluate(uv)};
        } 
        // refracting wo around the normal gives us the direction wi
        Vector wi = refract(wo, normal, ior);
        return BsdfSample{wi, m_transmittance->evaluate(uv)/(ior*ior)};
    }

    std::string toString() const override {
        return tfm::format("Dielectric[\n"
                           "  ior           = %s,\n"
                           "  reflectance   = %s,\n"
                           "  transmittance = %s\n"
                           "]",
                           indent(m_ior), indent(m_reflectance),
                           indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
