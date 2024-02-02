#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave
{

    void Instance::transformFrame(SurfaceEvent &surf) const
    {
        // hints:
        // * transform the hitpoint and frame here
        // * if m_flipNormal is true, flip the direction of the bitangent (which in effect flips the normal)
        // * make sure that the frame is orthonormal (you are free to change the bitangent for this, but keep
        //   the direction of the transformed tangent the same)

        // transform the point
        surf.position = m_transform->apply(surf.position);

        if (m_normal) {
            Frame frame = surf.frame;

            //evaluate the normal shading texture
            Color n = m_normal->evaluate(surf.uv);

            //remap coordinates from [0,1] to [-1,1]
            Vector m = Vector(2*n[0] - 1, 2*n[1] - 1, 2*n[2] - 1);

            //compute new normal
            Vector new_normal = frame.toWorld(m);
            new_normal = m_transform->applyNormal(new_normal).normalized();

            surf.frame = Frame(new_normal);

        } else {
            Vector world_tangent = m_transform->apply(surf.frame.tangent);
            Vector world_bitangent = m_transform->apply(surf.frame.bitangent);
            
            if (m_flipNormal)
            {
                // flip bitangent if m_flipNormal
                world_bitangent = -world_bitangent;
            }
            // Constructs the normal from the tangent and bitangent, and builds an orthonormal basis
            Vector normal = world_tangent.cross(world_bitangent);
            surf.pdf /= normal.length();
            surf.frame = Frame(normal.normalized());
        }
    }

    bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const
    {
        if (its.isShadowRay && m_invsibileForShadows) return false;
        
        if (!m_transform)
        {
            // fast path, if no transform is needed
            Ray localRay = worldRay;
            if (m_shape->intersect(localRay, its, rng))
            {
                its.instance = this;
                return true;
            }
            else
            {
                return false;
            }
        }

        const float previousT = its.t;
        Ray localRay = m_transform->inverse(worldRay);
        float length = localRay.direction.length();
        localRay.direction /= length;

        // hints:
        // * transform the ray (do not forget to normalize!)
        // * how does its.t need to change?

        its.t *= length;

        const bool wasIntersected = m_shape->intersect(localRay, its, rng);
        if (wasIntersected)
        {

            its.t /= length;
            // hint: how does its.t need to change?
            // * get the local intersection point and transform it
            //Point local_p = localRay(its.t);
            //local_p = m_transform->apply(local_p);

            // shoot a vector from the local intersection point to the origin and BOOM, the length is its.t!
            //Vector kick = local_p - worldRay.origin;
            //its.t = kick.length();

            its.instance = this;
            transformFrame(its);
            return true;
        }
        else
        {
            its.t = previousT;
            return false;
        }
    }

    Bounds Instance::getBoundingBox() const
    {
        if (!m_transform)
        {
            // fast path
            return m_shape->getBoundingBox();
        }

        const Bounds untransformedAABB = m_shape->getBoundingBox();
        if (untransformedAABB.isUnbounded())
        {
            return Bounds::full();
        }

        Bounds result;
        for (int point = 0; point < 8; point++)
        {
            Point p = untransformedAABB.min();
            for (int dim = 0; dim < p.Dimension; dim++)
            {
                if ((point >> dim) & 1)
                {
                    p[dim] = untransformedAABB.max()[dim];
                }
            }
            p = m_transform->apply(p);
            result.extend(p);
        }
        return result;
    }

    Point Instance::getCentroid() const
    {
        if (!m_transform)
        {
            // fast path
            return m_shape->getCentroid();
        }

        return m_transform->apply(m_shape->getCentroid());
    }

    AreaSample Instance::sampleArea(Sampler &rng) const
    {
        // add inverse m_transform for advanced sampling with origin
        
        AreaSample sample = m_shape->sampleArea(rng);
        transformFrame(sample);
        return sample;
    }

}

REGISTER_CLASS(Instance, "instance", "default")
