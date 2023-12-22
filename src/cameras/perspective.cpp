#include <lightwave.hpp>

namespace lightwave
{

    /**
     * @brief A perspective camera with a given field of view angle and transform.
     *
     * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
     * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
     * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
     * are directed in negative y direction ( @code ray.direction.y < 0 ).
     */
    class Perspective : public Camera
    {
        float factor_x;
        float factor_y;

    public:
        Perspective(const Properties &properties)
            : Camera(properties)
        {
            // get fov and fov axis from properties
            const float fov = properties.get<float>("fov");
            std::string fov_axis = properties.get<std::string>("fovAxis");

            float width = m_resolution.x();
            float height = m_resolution.y();
            float aspect_ratio = width / height;
            float tan_fov = tan((fov / 2.0) * (Pi / 180));

            // Calculate x and y factors for the direction Vector, depending on fov and aspect ratio
            if (fov_axis == "x")
            {
                factor_x = tan_fov;
                factor_y = tan_fov / aspect_ratio;
            }
            else
            {
                factor_x = aspect_ratio * tan_fov;
                factor_y = tan_fov;
            }

            // hints:
            // * precompute any expensive operations here (most importantly trigonometric functions)
            // * use m_resolution to find the aspect ratio of the image
        }

        // TODO: Move calculation to constructor
        CameraSample sample(const Point2 &normalized, Sampler &rng) const override
        {

            // Remapping of coordinates regarding the field of view and the aspect ratio, depending
            // on the two factors we calculated above.
            Vector direction = Vector(normalized.x() * factor_x, normalized.y() * factor_y, 1.0f);

            // Generation of new ray with the calculated and normalized direction vector and the origin (0,0,0)
            direction = direction.normalized();
            Ray ray = Ray(Vector(0.f, 0.f, 0.f), direction);
            Color weight = Color(1.0f);

            return CameraSample{

                m_transform->apply(ray).normalized(),
                weight};

            // hints:
            // * use m_transform to transform the local camera coordinate system into the world coordinate system
        }

        std::string toString() const override
        {
            return tfm::format(
                "Perspective[\n"
                "  width = %d,\n"
                "  height = %d,\n"
                "  transform = %s,\n"
                "]",
                m_resolution.x(),
                m_resolution.y(),
                indent(m_transform));
        }
    };

}

REGISTER_CAMERA(Perspective, "perspective")
