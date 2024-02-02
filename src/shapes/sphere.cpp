#include <lightwave.hpp>

namespace lightwave
{
    class Sphere : public Shape
    {

        inline void populate(SurfaceEvent &surf, const Point &position) const
        {
            surf.position = position;
            // TODO check uv coordinates
            surf.uv.x() = 0.5 + atan2(position.z(), position.x()) / (2*Pi);
            surf.uv.y() = 0.5 + asin(position.y()) / Pi;
            
            surf.frame = Frame(Vector(position).normalized());
            surf.pdf = Inv4Pi;
        }

    public: 
        Sphere(const Properties &properties)
        {
        }

        bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override
        {
            // calculate the discriminant with the abc formula
            Vector oc = Vector(ray.origin);
            float b = 2.0 * ray.direction.dot(oc);
            float c = oc.dot(oc) - 1.0;
            float discriminant = b * b - 4 * c;

            if (discriminant < 0)
            {
                return false;
            } else if (discriminant == 0) {
                float t = -b / 2;
                if (t < Epsilon || t > its.t) {
                    return false;
                }
                const Point position = ray(t);
                its.t = t;
                populate(its, position);
                return true;
            } else {
                float t1 = (b > 0) ? -0.5f * (b + sqrt(discriminant)) : -0.5f * (b - sqrtf(discriminant));
                float t2 = c / t1;

                float t = min(t1, t2);
                if (t < Epsilon) t = max(t1, t2);
                if (t < Epsilon || t > its.t)
                {
                    return false;
                }
                if (t > 0) {
                    const Point position = ray(t);
                    its.t = t;
                    populate(its, position);
                    return true;
                }
            }
            return false;
        }

        Bounds getBoundingBox() const override
        {
            return Bounds(Point{-1, -1, -1}, Point{1, 1, 1});
        }

        Point getCentroid() const override
        {
            return Point(0);
        }

        AreaSample sampleArea(Sampler &rng) const override{
            Point2 next = rng.next2D();
        
            Point position = squareToUniformSphere(next);
            AreaSample sample;
            populate(sample, position); 
            return sample;
        }

        std::string toString() const override
        {
            return "Sphere[]";
        }
    };
}
REGISTER_SHAPE(Sphere, "sphere")