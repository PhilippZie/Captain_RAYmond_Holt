#include <lightwave.hpp>

namespace lightwave
{

    class CheckerboardTexture : public Texture
    {
        Color color0;
        Color color1;
        Vector2 scale;

    public:
        CheckerboardTexture(const Properties &properties)
        {
            color0 = properties.get<Color>("color0", Color(0));
            color1 = properties.get<Color>("color1", Color(1));
            scale = properties.get<Vector2>("scale");
        }

        Color evaluate(const Point2 &uv) const override{

            float x = fmodf(floor(uv.x() * scale.x()), 2);

            float y = fmodf(floor(uv.y() * scale.y()), 2);

            if (x == y) {
                return color0;
            } else {
                return color1;
            }

        }

        std::string toString() const override
        {
            return tfm::format("CheckerboardTexture[\n"
                               "  value = %s\n"
                               "]",
                               indent(color0), indent(color1));
        }
    };

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
