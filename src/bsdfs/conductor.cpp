#include <lightwave.hpp>

namespace lightwave {

class Conductor : public Bsdf {
    ref<Texture> m_reflectance;

public:
    Conductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting `wo' is zero, hence we can just ignore
        // that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {

        // reflecting wo around the normal gives us the direction wi
        // Vector(0,0,1) is the normal in the local coordinate system
        Vector wi = reflect(wo, Vector(0, 0, 1));
        return BsdfSample{wi, m_reflectance->evaluate(uv)};
    }

    std::string toString() const override {
        return tfm::format("Conductor[\n"
                           "  reflectance = %s\n"
                           "]",
                           indent(m_reflectance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Conductor, "conductor")
