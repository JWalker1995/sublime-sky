#include "externalgenerator.h"

#include "network/connectionmanager.h"
#include "network/messagebuilder.h"
#include "schemas/message_generated.h"

namespace worldgen {

ExternalGenerator::ExternalGenerator(game::GameContext &context)
    : context(context)
{}

void ExternalGenerator::generate(spatial::CellKey cube, const pointgen::Chunk *points) {
    network::MessageBuilder::Lock lock(context);

    SublimeSky::Vec3_u32 cellCoord(cube.cellCoord.x, cube.cellCoord.y, cube.cellCoord.z);
    auto chunkCommand = SublimeSky::CreateChunk(lock.getBuilder(), cube.sizeLog2, &cellCoord);
    auto message = SublimeSky::CreateMessage(lock.getBuilder(), SublimeSky::MessageUnion_Chunk, chunkCommand.Union());
    lock.getBuilder().Finish(message);

    context.get<network::ConnectionManager>().send(lock.getBuilder().GetBufferPointer(), lock.getBuilder().GetSize());
}

void ExternalGenerator::handleResponse(const SublimeSky::Chunk *chunk) {
//    chunk->
}

}
