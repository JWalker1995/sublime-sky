#include "simplegenerator.h"

#include "world/hashtreeworld.h"
#include "render/scenemanager.h"
#include "particle/particlemanager.h"
#include "util/pool.h"

namespace worldgen {

SimpleGenerator::SimpleGenerator(game::GameContext &context)
    : context(context)
    , noise(123)
{
    assert(pointgen::Chunk::size == world::Chunk::size);

    render::SceneManager::MaterialMutator airMat = context.get<render::SceneManager>().createMaterial();
    airMat.shared.renderModel = graphics::MaterialShared::RenderModel::Blinn;
    std::fill_n(airMat.shared.colorAmbient, 4, 1.0f);
    std::fill_n(airMat.shared.colorDiffuse, 4, 1.0f);
    std::fill_n(airMat.shared.colorSpecular, 4, 1.0f);
    airMat.shared.shininess = 1.0f;
    airMat.local.name = "Air";
    airMat.local.phase = graphics::MaterialLocal::Phase::Gas;
    airMat.local.mass = 1.0f;
    airMaterialIndex = airMat.index;

    render::SceneManager::MaterialMutator groundMat = context.get<render::SceneManager>().createMaterial();
    groundMat.shared.renderModel = graphics::MaterialShared::RenderModel::Blinn;
    std::fill_n(groundMat.shared.colorAmbient, 4, 1.0f);
    std::fill_n(groundMat.shared.colorDiffuse, 4, 1.0f);
    std::fill_n(groundMat.shared.colorSpecular, 4, 1.0f);
    groundMat.shared.shininess = 1.0f;
    groundMat.local.name = "Air";
    groundMat.local.phase = graphics::MaterialLocal::Phase::Solid;
    groundMat.local.mass = 1.0f;
    groundMaterialIndex = groundMat.index;
}

void SimpleGenerator::generate(spatial::CellKey cube, const pointgen::Chunk *points) {
    world::HashTreeWorld::Cell &cell = context.get<world::HashTreeWorld>().lookupChunk(cube);

    if (!cell.second.chunk) {
        cell.second.chunk = context.get<util::Pool<world::Chunk>>().alloc();
    }

    bool allSame = true;
    bool allState = getState(points->points[0][0][0]);

    for (unsigned int i = 0; i < world::Chunk::size; i++) {
        for (unsigned int j = 0; j < world::Chunk::size; j++) {
            for (unsigned int k = 0; k < world::Chunk::size; k++) {
                bool state = getState(points->points[i][j][k]);
                cell.second.chunk->cells[i][j][k].materialIndex = static_cast<world::MaterialIndex>(state ? groundMaterialIndex : airMaterialIndex);

                allSame &= state == allState;
            }
        }
    }

    glm::vec3 particlePosition(0.0f, 0.0f, 100.0f);
    if (cube.contains(spatial::UintCoord::fromPoint(particlePosition))) {
        particle::Particle &p = context.get<particle::ParticleManager>().createParticle();
        p.position = particlePosition;
        p.velocity = glm::vec3(0.1f, 0.0f, 0.0f);
        p.mass = 1.0f;
        p.energy = 0.0f;
    }

    allSame = false;

    if (allSame) {
        context.get<util::Pool<world::Chunk>>().free(cell.second.chunk);
        cell.second.chunk = 0;
        cell.second.constantMaterialIndex = static_cast<world::MaterialIndex>(allState ? groundMaterialIndex : airMaterialIndex);
    } else {
#ifndef NDEBUG
        cell.second.constantMaterialIndex = static_cast<world::MaterialIndex>(-1);
#endif
    }

    context.get<world::HashTreeWorld>().updateGasMasks(&cell.second);
}

bool SimpleGenerator::getState(glm::vec3 point) {
    if (std::isnan(point.x)) {
        return false;
    }

//    glm::vec2 treeCenter(point.x, point.y);
//    float v = 10.0f;
//    while (true) {
//        float newV = noise.octave_noise_2d(2, 0.5f, 0.1f, treeCenter.x, treeCenter.y);
//        if (std::fabs(newV - v) < 1e-6) {
//            break;
//        }
//        v = newV;
//        float dx = (noise.octave_noise_2d(2, 0.5f, 0.1f, treeCenter.x + 1e-3, treeCenter.y) - v) / 1e-3;
//        float dy = (noise.octave_noise_2d(2, 0.5f, 0.1f, treeCenter.x, treeCenter.y + 1e-3) - v) / 1e-3;

//        float scale = (1.0f + (rand() % 256) / 256.0f) * 1.0f;
//        treeCenter.x += dx * scale;
//        treeCenter.y += dy * scale;
//    }

    static constexpr float scaleXY = 0.1f;
    static constexpr float scaleZ = 10.0f;
    float z = noise.octave_noise_2d(8, 0.5f, scaleXY, point.x, point.y) * scaleZ;

    if (point.z >= 20.0f && point.z < 22.0f) {
        if (noise.octave_noise_2d(2, 0.5f, 0.1f, point.x, point.y) > 0.6f) {
            if (rand() % 64 == 0) {
                particle::Particle &p = context.get<particle::ParticleManager>().createParticle();
                p.position = point - glm::vec3(0.0f, 0.0f, 4.0f);
                p.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                p.mass = 1.0f;
                p.energy = 0.0f;

                context.get<particle::ParticleManager>().addBond(p, point);
            }

            return true;
        }
    }

//    if (glm::distance2(treeCenter, glm::vec2(point.x, point.y)) < 1.0f) {
//        z += 4.0f;
//    }

    return point.z < z;
}

}
