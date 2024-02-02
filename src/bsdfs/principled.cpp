#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        
        return BsdfEval{color * InvPi * max(0,Frame::cosTheta(wi))};

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {

        // calculate the vector that gets reflected from the diffuse area
        Vector t = squareToCosineHemisphere(rng.next2D());

        return BsdfSample{t, color};

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        // calculate the microfacet normal (wm)
        Vector normal = (wi + wo) / (wi + wo).length();

        // smith masking shadowing function
        float g1_wi = microfacet::smithG1(alpha, normal, wi);
        float g1_wo = microfacet::smithG1(alpha, normal, wo);

        float d_wm = microfacet::evaluateGGX(alpha, normal);

        // fraction of the formula
        float fraction = 1 / abs(4 * Frame::cosTheta(wo));
        // implement the formula from the task sheet
        Color weight = color * g1_wi * g1_wo * d_wm * fraction;

        return BsdfEval{weight};

        // hints:
        // * copy your roughconductor bsdf evaluate here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {

        // microfacet normal
        Vector normal = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        // calculate wi with the sampled microfacet normal
        Vector wi = reflect(wo, normal);
        // calculate g1 using the smith masking shadowing function
        float g1 = microfacet::smithG1(alpha, normal, wi);
        // the final weight is the reflectance(color) times the g1 term
        Color weight = color * g1;

        return BsdfSample{wi, weight};

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular = m_specular->scalar(uv);
        const auto metallic = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto combination = combine(uv, wo);
        
        BsdfEval diff = combination.diffuse.evaluate(wo, wi);
        BsdfEval metal = combination.metallic.evaluate(wo, wi);
        return BsdfEval{diff.value + metal.value};

        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto combination = combine(uv, wo);

        // get the probability of the diffuse lobe
        float prob_diff = combination.diffuseSelectionProb;
        
        // determine which lobe to sample
        if (rng.next() < prob_diff) {
            // sample the diffuse lobe
            BsdfSample samp = combination.diffuse.sample(wo, rng);
            if (prob_diff != 0) {
                samp.weight /= prob_diff;
            } else {
                return BsdfSample::invalid();
            }
            return samp;
        } else {
            // sample the metallic lobe
            BsdfSample samp = combination.metallic.sample(wo, rng);
            if (1-prob_diff != 0) {
                samp.weight /= 1 - prob_diff;
            } else {
                return BsdfSample::invalid();
            }
            return samp;
        }

        // hint: sample either `combination.diffuse` (probability
        // `combination.diffuseSelectionProb`) or `combination.metallic`
    }

    Color evaluateAlbedo(const Point2 &uv) const override {
        return m_baseColor->evaluate(uv);
    }

    std::string toString() const override {
        return tfm::format("Principled[\n"
                           "  baseColor = %s,\n"
                           "  roughness = %s,\n"
                           "  metallic  = %s,\n"
                           "  specular  = %s,\n"
                           "]",
                           indent(m_baseColor), indent(m_roughness),
                           indent(m_metallic), indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
