#include <lightwave.hpp>

namespace lightwave
{
    class AlbedoIntegrator : public SamplingIntegrator
    {


    public:
        AlbedoIntegrator(const Properties &properties)
            : SamplingIntegrator(properties)
        {

        }

        Color Li(const Ray &ray, Sampler &rng) override
        {
            // intersect ray against the scene
            Intersection intersection = m_scene->intersect(ray, rng);
            if (!intersection)
            {
                return m_scene->evaluateBackground(ray.direction).value;
            }
            //Color albedo = intersection.evaluateBsdf(-ray.direction).value;
            Color albedo = intersection.instance->bsdf()->evaluateAlbedo(intersection.uv);
            return albedo;
        }

        std::string toString() const override
        {
            return tfm::format(
                "AlbedoIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image));
        }
    };
}

REGISTER_INTEGRATOR(AlbedoIntegrator, "albedos")
