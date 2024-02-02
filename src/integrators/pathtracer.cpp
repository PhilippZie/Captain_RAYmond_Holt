#include <lightwave.hpp>

namespace lightwave
{
    class Pathtracer : public SamplingIntegrator
    {
        int depth;

    public:
        Pathtracer(const Properties &properties)
            : SamplingIntegrator(properties)
        {
            depth = properties.get<int>("depth", 2);
        }

        Color Li(const Ray &ray, Sampler &rng) override
        {
            // Initialize variables that are updated every iteration
            Intersection its;
            BsdfSample bsdf;
            Color weight = Color(1);
            Color obEm = Color(0);
            Ray nextRay = ray;
            
            // Iterate over the depth
            for (int i = 0;; i++) {
                // intersect ray against the scene
                its = m_scene->intersect(nextRay, rng);
                // if object hits backround -> return background color
                if (!its)
                {
                    auto bg = m_scene->evaluateBackground(nextRay.direction).value;
                    obEm += bg * weight / std::max(bg.mean() / 10, float(1));
                    break;
                }
                auto hit = (weight * its.evaluateEmission());
                obEm += hit;
                if (i >= depth - 1) break;

                // check if the scene has lights  (next event estimation)
                if (m_scene->hasLights()) {
                    // sample a light source
                    LightSample lightSample = m_scene->sampleLight(rng);
                    // if lightSample can not be intersected
                    if (!lightSample.light->canBeIntersected()) {
                        DirectLightSample directSample = lightSample.light->sampleDirect(its.position, rng);
                        BsdfEval bsdfEval = its.evaluateBsdf(directSample.wi);
                        // create a ray from the current intersection to the lightsource
                        Ray shadowRay(its.position, directSample.wi);
                        //check if intersection between current point and lightsource is possible, s.t. they can "see" ech other
                        bool isVisible = !m_scene->intersect(shadowRay, directSample.distance, rng);
                        if (isVisible) {
                            // maybe the total weight needs to be updated here, not sure
                            auto nee = ((weight * bsdfEval.value * directSample.weight) / lightSample.probability);
                            obEm += nee;
                        }
                    }
                } 

                // Object gets hit -> get Bsdf sample and the emission value of the hit Object
                bsdf = its.sampleBsdf(rng);
                weight *= bsdf.weight;
                // sample invalid -> return the emission Color
                if (bsdf.isInvalid())
                {
                    return obEm;
                }
                
                // Cast ray from the current intersection to the next object (or to infinity)
                nextRay = Ray(its.position, bsdf.wi);
            }
            // contributed bsdf weight and the emission value of the object
            return obEm;
        }

        std::string toString() const override
        {
            return tfm::format(
                "Pathtracer[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image));
        }
    };
}

REGISTER_INTEGRATOR(Pathtracer, "pathtracer")
