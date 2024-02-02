#include <lightwave.hpp>

namespace lightwave
{

    class Diffuse : public Bsdf
    {
        ref<Texture> m_albedo;

    public:
        Diffuse(const Properties &properties)
        {
            m_albedo = properties.get<Texture>("albedo");
        }

        BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
            
            return BsdfEval{m_albedo->evaluate(uv) * InvPi * max(0, Frame::cosTheta(wi))};
        }


        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override
        {

            // calculate the vector that gets reflected from the diffuse area
            Vector t = squareToCosineHemisphere(rng.next2D());

            return BsdfSample{t, m_albedo->evaluate(uv)};
        }

        Color evaluateAlbedo(const Point2 &uv) const override {
            return m_albedo->evaluate(uv);
        }

        std::string toString() const override
        {
            return tfm::format("Diffuse[\n"
                               "  albedo = %s\n"
                               "]",
                               indent(m_albedo));
        }
    };

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
