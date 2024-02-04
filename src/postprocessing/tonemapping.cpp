#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief ToneMapping postprocess, inherits from Postprocess.
 */
class ToneMapping : public Postprocess {
    private:
    public:
        ToneMapping(const Properties &properties) : Postprocess(properties) { 

        }

        void execute() override {
            float width = m_input->resolution().x();
            float height = m_input->resolution().y();

            m_output->initialize(m_input->resolution());


            for (int i = 0; i < width * height; i++) {
                Color pixel = m_input->data()[i];
                m_output->data()[i] = pixel /(Color(1) + pixel);
            }

            m_output->save();
        }


        std::string toString() const override
        {
            return "ToneMapping";
        }
    };
}
REGISTER_POSTPROCESS(ToneMapping, "tonemapping")