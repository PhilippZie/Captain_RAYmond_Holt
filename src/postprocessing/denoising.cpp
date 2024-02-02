#ifdef LW_WITH_OIDN
#include <lightwave.hpp>
#include <OpenImageDenoise/oidn.hpp>

namespace lightwave {

/**
 * @brief Denoising postprocess, inherits from Postprocess. Implements the OpenImageDenoise library.
 */
class Denoising : public Postprocess {
    private:
        ref<Image> m_normals;
        ref<Image> m_albedo;
    public:
        Denoising(const Properties &properties) : Postprocess(properties) { 
            m_normals = properties.get<Image>("normals");
            m_albedo = properties.get<Image>("albedos"); 

        }

        void execute() override {
            float width = m_input->resolution().x();
            float height = m_input->resolution().y();

            m_output->initialize(m_input->resolution());
            // Create an OIDN device
            oidn::DeviceRef device = oidn::newDevice(oidn::DeviceType::CPU);
            device.commit();
            //oidn::BufferRef colorBuf  = device.newBuffer(width * height * 3 * sizeof(float));

            // Create a denoising filter
            oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
            filter.setImage("color", m_input->data(), oidn::Format::Float3, width, height);
            filter.setImage("normals", m_normals->data(), oidn::Format::Float3, width, height);
            filter.setImage("albedo", m_albedo->data(), oidn::Format::Float3, width, height);
            filter.setImage("output", m_output->data(), oidn::Format::Float3, width, height);
            filter.commit();
            

            // Filter the image
            filter.execute();
            const char* errorMessage;
            if (device.getError(errorMessage) != oidn::Error::None)
                std::cout << "Error: " << errorMessage << std::endl;
            m_output->save();
            Streaming stream {*m_output};
            stream.update();
        }
        std::string toString() const override
        {
            return "Denosing";
        }
    };
}
REGISTER_POSTPROCESS(Denoising, "denoising")
#endif