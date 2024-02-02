#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        m_border =
            properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                           {
                                               { "clamp", BorderMode::Clamp },
                                               { "repeat", BorderMode::Repeat },
                                           });

        m_filter = properties.getEnum<FilterMode>(
            "filter", FilterMode::Bilinear,
            {
                { "nearest", FilterMode::Nearest },
                { "bilinear", FilterMode::Bilinear },
            });
    }

    Color evaluate(const Point2 &uv) const override {
        return evaluateFilterMode(Point2(uv.x(), 1-uv.y()));
    }

    inline Color evaluateFilterMode(const Point2 &uv) const {

        Point2 scaledCoords(uv.x() * m_image->resolution().x(), uv.y() * m_image->resolution().y());

        if (m_filter == FilterMode::Nearest) {
            // For nearest neighbour we can just floor the Coords and pass them
            Point2i flooredCoords(floor(scaledCoords.x()), floor(scaledCoords.y()));
            return m_image->get(evaluateBorderMode(flooredCoords)) * m_exposure;
        } else {  
            //FilterMode::Bilinear

            //Shift coordinates by 0.5
            Point2i flooredCoords(floor(scaledCoords.x() - 0.5f), floor(scaledCoords.y() - 0.5f));

            //get the x,y coordinates in the image and the surrounding pixels
            Point2i t = evaluateBorderMode(Point2i(flooredCoords.x(), flooredCoords.y()));
            Point2i p2 = evaluateBorderMode(Point2i(flooredCoords.x(), flooredCoords.y() + 1));
            Point2i p3 = evaluateBorderMode(Point2i(flooredCoords.x() + 1, flooredCoords.y()));
            Point2i p4 = evaluateBorderMode(Point2i(flooredCoords.x() + 1, flooredCoords.y() + 1));
        
            //compute fu, fv factors
            float fu = (scaledCoords.x() - 0.5f - floor(scaledCoords.x() - 0.5f));
            float fv = (scaledCoords.y() - 0.5f - floor(scaledCoords.y() - 0.5f));
            
            
            return ((1-fu) * (1-fv) * m_image->get(t) 
                + (1-fu) * fv * m_image->get(p2)
                + fu * (1-fv) * m_image->get(p3)
                + fu * fv * m_image->get(p4))
                * m_exposure;
        }

    }

    /**
     * evaluates the Border handling on integer Coordinates of the picture
    */
    inline Point2i evaluateBorderMode(const Point2i &imageCoords) const {
        int x;
        int y;
        if (m_border == BorderMode::Repeat) {
            // maps the given coordinates to relativ coordinates of the image
            if (imageCoords.x() < 0) {
                x = imageCoords.x() % (m_image->resolution().x());
                x += x == 0 ? 0 : m_image->resolution().x();
            } else {
                x = imageCoords.x() % (m_image->resolution().x());
            }

            if (imageCoords.y() < 0) {
                y = imageCoords.y() % (m_image->resolution().y());
                y += y== 0 ? 0 : m_image->resolution().y();
            } else {
                y = imageCoords.y() % (m_image->resolution().y());
            }
        } else {
            // BorderMode::Wrap
            //compute slide "wrap mode clamp"
            //maps floats between [-inf, inf] -> [0,1]
            x = imageCoords.x() < 0 ? 0 : (imageCoords.x() > m_image->resolution().x() - 1 ? m_image->resolution().x() - 1 : imageCoords.x());
            y = imageCoords.y() < 0 ? 0 : (imageCoords.y() > m_image->resolution().y() - 1 ? m_image->resolution().y() - 1 : imageCoords.y());
        }

        return Point2i(x, y);
    }
    
    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
