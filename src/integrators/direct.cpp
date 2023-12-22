#include <lightwave.hpp>

namespace lightwave
{
    class DirectIntegrator : public SamplingIntegrator
    {

    public:
        DirectIntegrator(const Properties &properties)
            : SamplingIntegrator(properties)
        {
        }

        Color Li(const Ray &ray, Sampler &rng) override
        {
            // intersect ray against the scene
            Intersection its = m_scene->intersect(ray, rng);

            // if object hits backround -> return background color
            if (!its)
            {
                return m_scene->evaluateBackground(ray.direction).value;
            }

            // Object gets hit -> get Bsdf sample and the emission value of the hit Object
            BsdfSample bsdf = its.sampleBsdf(rng);
            Color firstObEm = (its.evaluateEmission());

            // sample invalid -> return the emission Color
            if (bsdf.isInvalid())
            {
                return firstObEm;
            }

            // check if the scene has lights 
            if (m_scene->hasLights()) {
                // sample a light source
                LightSample lightSample = m_scene->sampleLight(rng);
                DirectLightSample directSample = lightSample.light->sampleDirect(its.position, rng);
                BsdfEval bsdfEval = its.evaluateBsdf(directSample.wi);

                // create a ray from the current intersection to the lightsource
                Ray lightRay(its.position, directSample.wi);
                bool isVisible = !m_scene->intersect(lightRay, directSample.distance, rng);

                //check if intersection between current point and lightsource is possible, s.t. they can "see" ech other
                if ((!lightSample.light->canBeIntersected()) && isVisible) {
                    firstObEm += (directSample.weight * bsdfEval.value) / lightSample.probability;
                }
            }

            // Cast ray from the current intersection to the next object (or to infinity)
            Ray secRay = Ray(its.position, bsdf.wi, 1);
            Intersection secIts = m_scene->intersect(secRay, rng);

            // second ray goes to infinity -> evaluate color with background, weight and the emission value
            if (!secIts)
            {
                return (m_scene->evaluateBackground(secRay.direction).value) * bsdf.weight + firstObEm;
            }

            // Secondary Ray hits another object -> get this objects Emission
            // contributed bsdf weight and the emission value of the first object
            return secIts.evaluateEmission()*bsdf.weight + firstObEm;
        }

        std::string toString() const override
        {
            return tfm::format(
                "DirectIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image));
        }
    };
}

REGISTER_INTEGRATOR(DirectIntegrator, "direct")
