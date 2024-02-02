#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave
{

    /**
     * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
     * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
     * triangles are combined in a single shape.
     */
    class TriangleMesh : public AccelerationStructure
    {
        /**
         * @brief The index buffer of the triangles.
         * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
         * vertex index (into @c m_vertices ) of the triangle.
         * This list will always contain as many elements as there are triangles.
         */
        std::vector<Vector3i> m_triangles;
        /**
         * @brief The vertex buffer of the triangles, indexed by m_triangles.
         * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
         * vertices.
         */
        std::vector<Vertex> m_vertices;
        /// @brief The file this mesh was loaded from, for logging and debugging purposes.
        std::filesystem::path m_originalPath;
        /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
        bool m_smoothNormals;

        inline void populate(SurfaceEvent &surf, const Point &position, const Vector &normal, const Vertex intVert) const
        {
            surf.position = position;

            surf.uv = intVert.texcoords;

            surf.frame = Frame(normal);
            surf.pdf = 0;
        }

    protected:
        int numberOfPrimitives() const override
        {
            return int(m_triangles.size());
        }

        bool intersect(int primitiveIndex, const Ray &ray, Intersection &its, Sampler &rng) const override
        {

            Vector3i vert = m_triangles[primitiveIndex];
            Vertex v1 = m_vertices[vert.x()];
            Vertex v2 = m_vertices[vert.y()];
            Vertex v3 = m_vertices[vert.z()];

            // This is beautiful and explains everything
            /*          1
                        /\
               e1->  /         \   <-e2
                /________________\
               2                   3
            */

            Vector t(ray.origin - v1.position);
            Vector e1(v2.position - v1.position);
            Vector e2(v3.position - v1.position);

            Vector t_normal(e1.cross(e2));

            if (t_normal.dot(ray.direction) == 0)
            {
                return false;
            }

            Vector p = ray.direction.cross(e2);
            Vector q = t.cross(e1);

            Vector tuv = (1 / (p.dot(e1))) * Vector(q.dot(e2), p.dot(t), q.dot(ray.direction));

            if (!(0 <= tuv.y() && tuv.y() <= 1 && 0 <= tuv.z() && tuv.z() <= 1 && tuv.y() + tuv.z() <= 1) || tuv.x() < 0)
            {
                // 0 <= u <= 1, 0 <= v <= 1, and u+v <=1.
                return false;
            }

            if (tuv.x() < Epsilon || tuv.x() > its.t)
            {
                return false;
            }

            its.t = tuv.x(); // its.t = t
            Point pos = ray(tuv.x());

            Vertex intVert = Vertex::interpolate(Vector2(tuv.y(), tuv.z()), v1, v2, v3);

            if (m_smoothNormals)
            {
                populate(its, pos, intVert.normal.normalized(), intVert);
                return true;
            }

            populate(its, pos, t_normal.normalized(), intVert);
            return true;

            // hints:
            // * use m_triangles[primitiveIndex] to get the vertex indices of the triangle that should be intersected
            // * if m_smoothNormals is true, interpolate the vertex normals from m_vertices
            //   * make sure that your shading frame stays orthonormal!
            // * if m_smoothNormals is false, use the geometrical normal (can be computed from the vertex positions)
        }

        Bounds getBoundingBox(int primitiveIndex) const override
        {
            Bounds box = Bounds::empty();

            Vector3i vert = m_triangles[primitiveIndex];
            Vertex v1 = m_vertices[vert.x()];
            Vertex v2 = m_vertices[vert.y()];
            Vertex v3 = m_vertices[vert.z()];

            box.extend(v1.position);
            box.extend(v2.position);
            box.extend(v3.position);

            return box;
        }

        Point getCentroid(int primitiveIndex) const override
        {
            return getBoundingBox(primitiveIndex).center();
        }

    public:
        TriangleMesh(const Properties &properties)
        {
            m_originalPath = properties.get<std::filesystem::path>("filename");
            m_smoothNormals = properties.get<bool>("smooth", true);
            readPLY(m_originalPath.string(), m_triangles, m_vertices);
            logger(EInfo, "loaded ply with %d triangles, %d vertices",
                   m_triangles.size(),
                   m_vertices.size());
            buildAccelerationStructure();
        }

        AreaSample sampleArea(Sampler &rng) const override{
            // only implement this if you need triangle mesh area light sampling for your rendering competition
            NOT_IMPLEMENTED}

        std::string toString() const override
        {
            return tfm::format(
                "Mesh[\n"
                "  vertices = %d,\n"
                "  triangles = %d,\n"
                "  filename = \"%s\"\n"
                "]",
                m_vertices.size(),
                m_triangles.size(),
                m_originalPath.generic_string());
        }
    };

}

REGISTER_SHAPE(TriangleMesh, "mesh")
