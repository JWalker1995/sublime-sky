#pragma once

#if 0

#include <CGAL/Kd_tree.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "render/scenemanager.h"

namespace game { class GameContext; }

namespace render {

class VertSpatialIndex {
public:
    VertSpatialIndex(game::GameContext &context);

    void getVertsInside(glm::vec3 min, glm::vec3 max, std::vector<unsigned int> &dstVertIndices);

private:
    class SearchTraits {
    public:
        SearchTraits(render::SceneManager::MeshHandle meshHandle)
            : meshHandle(meshHandle)
        {}

//        typedef unsigned int Kernel;

        typedef CGAL::Simple_cartesian<float> K;
//        typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

//        typedef typename Kernel::Cartesian_const_iterator_3 Cartesian_const_iterator_d;
//        typedef typename Kernel::Construct_cartesian_const_iterator_3 Construct_cartesian_const_iterator_d;
//        typedef typename Kernel::Point_3 Point_d;
//        typedef typename K::Iso_cuboid_3 Iso_box_d;
//        typedef typename K::Sphere_3 Sphere_d;
        typedef typename K::Construct_iso_cuboid_3 Construct_iso_box_d;

//        typedef typename K::Construct_min_vertex_3 Construct_min_vertex_d;
//        typedef typename K::Construct_max_vertex_3 Construct_max_vertex_d;
//        typedef typename K::Construct_center_3 Construct_center_d;
//        typedef typename K::Compute_squared_radius_3 Compute_squared_radius_d;
        typedef typename K::FT FT;

        typedef CGAL::Dimension_tag<3> Dimension;
        typedef unsigned int Point_d;

        typedef const float * Cartesian_const_iterator_d;
        class Construct_cartesian_const_iterator_d {
        public:
            Construct_cartesian_const_iterator_d(render::SceneManager::MeshHandle meshHandle)
                : meshHandle(meshHandle)
            {}

            typedef Cartesian_const_iterator_d result_type;

            const float* operator()(unsigned int vertIndex) const {
                return meshHandle.readVert(vertIndex).shared.point;
            }

            const float* operator()(unsigned int vertIndex, int) const {
                return meshHandle.readVert(vertIndex).shared.point + 3;
            }

        private:
            render::SceneManager::MeshHandle meshHandle;
        };

        Construct_cartesian_const_iterator_d construct_cartesian_const_iterator_d_object() const {
            return Construct_cartesian_const_iterator_d(meshHandle);
        }

    private:
        render::SceneManager::MeshHandle meshHandle;
    };

    typedef CGAL::Sliding_midpoint<SearchTraits> Splitter;

    SearchTraits searchTraits;
    CGAL::Kd_tree<SearchTraits, Splitter> tree;
};

}

#endif
