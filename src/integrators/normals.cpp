#include <lightwave.hpp>

namespace lightwave
{
    class NormalIntegrator : public SamplingIntegrator
    {

        bool remap;

    public:
        NormalIntegrator(const Properties &properties)
            : SamplingIntegrator(properties)
        {
            remap = properties.get<bool>("remap", true);
        }

        Color Li(const Ray &ray, Sampler &rng) override
        {
            // intersect ray against the scene
            Intersection intersection = m_scene->intersect(ray, rng);

            Vector normal = intersection.frame.normal;

            /* Is that right???
            if (!intersection) {
                return m_scene->evaluateBackground(ray.direction).value;
            } */ 

            if (remap)
            {
                normal = (normal + Vector(1)) / 2;
            }

            return Color(normal);
        }

        std::string toString() const override
        {
            return tfm::format(
                "NormalIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image));
        }
    };
}

REGISTER_INTEGRATOR(NormalIntegrator, "normals")
