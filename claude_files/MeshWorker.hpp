#pragma once
#include "VoxelEngine.hpp"
#include <mutex>

// ============================================================
// MeshWorker.hpp
// The function that runs on each worker thread.
// Its job: read one chunk from the world, build its mesh
// (vertex + index data on the CPU), then hand it off to the
// render thread via the shared queue.
//
// This function never touches OpenGL. It only produces CPU-side
// MeshData. The render thread is the only one allowed to call
// OpenGL functions, because OpenGL requires all calls to come
// from the thread that created the context.
// ============================================================

// Build face data for one visible block face and append to mesh.
// direction: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
// wx, wy, wz: world-space position of the block
inline void addFace(MeshData& mesh, int wx, int wy, int wz, int direction) {
    // Each face is a quad made of 2 triangles (6 indices, 4 vertices).
    // This is where you'd put your actual face geometry.
    // Placeholder -- real implementation adds 4 vertices and 6 indices.
    (void)mesh; (void)wx; (void)wy; (void)wz; (void)direction;
}

// Returns true if the block at (x,y,z) in the chunk is air (type == 0).
// We use this to check whether a face should be rendered:
// a face between two solid blocks is hidden and should be skipped.
inline bool isAir(const Chunk& chunk, int x, int y, int z) {
    // Bounds check -- treat out-of-chunk positions as air for now.
    // A production engine would also check the neighboring chunk.
    if (x < 0 || x >= CHUNK_SIZE)   return true;
    if (y < 0 || y >= WORLD_HEIGHT)  return true;
    if (z < 0 || z >= CHUNK_SIZE)   return true;
    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * WORLD_HEIGHT;
    return chunk.blocks[index].type == 0;
}

// ============================================================
// meshWorker -- runs on a worker thread, one call per chunk.
//
// Parameters:
//   engine  -- shared engine state, passed BY REFERENCE so this
//              thread sees the same world as everyone else.
//              (use std::ref(engine) when passing to addTask)
//   chunkX, chunkZ -- which chunk to build a mesh for.
// ============================================================
inline void meshWorker(VoxelEngine& engine, int chunkX, int chunkZ) {

    // -- Step 1: Create a LOCAL MeshData on this thread's stack.
    // No mutex needed here -- nothing else can see this variable yet.
    MeshData mesh;
    mesh.chunkX = chunkX;
    mesh.chunkZ = chunkZ;

    // -- Step 2: Read the chunk data from the shared world.
    //
    // The { } here are a DELIBERATE SCOPE, not an if or loop.
    // When execution reaches the closing }, lock is destroyed,
    // which automatically unlocks worldMtx. This is RAII on a mutex.
    //
    // We keep this scope as TIGHT as possible -- we lock, copy out
    // the data we need, then immediately unlock. We do NOT hold
    // the lock during the slow mesh-building loop below, because
    // that would block every other thread that needs to read the
    // world for the entire duration.
    {
        // lock_guard locks worldMtx right now and unlocks at }.
        // Use lock_guard (not unique_lock) here because we just need
        // a simple lock-and-release with no manual unlocking.
        std::lock_guard<std::mutex> lock(engine.worldMtx);

        // Check the chunk actually exists in the world map.
        auto it = engine.world.find({chunkX, chunkZ});
        if (it == engine.world.end()) {
            return;  // chunk doesn't exist yet, nothing to build
        }

        const Chunk& chunk = it->second;

        // -- Step 3: Build the mesh.
        // Triple nested loop visits every block position in the chunk.
        // x and z go 0..CHUNK_SIZE, y goes 0..WORLD_HEIGHT.
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < WORLD_HEIGHT; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {

                    // Convert 3D (x,y,z) to 1D array index.
                    // Why flat array? Contiguous memory = no pointer chasing
                    // = faster cache access when looping all blocks.
                    int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * WORLD_HEIGHT;
                    Block b = chunk.blocks[index];

                    // Skip air blocks entirely -- nothing to draw.
                    // "continue" jumps to the next loop iteration immediately.
                    if (b.type == 0) continue;

                    // Face culling -- only add a face if the neighbor is air.
                    // A face shared between two solid blocks is NEVER visible,
                    // so adding it just wastes GPU time drawing nothing.
                    // World position = chunk position * chunk size + local position.
                    int wx = chunkX * CHUNK_SIZE + x;
                    int wy = y;
                    int wz = chunkZ * CHUNK_SIZE + z;

                    if (isAir(chunk, x + 1, y, z)) addFace(mesh, wx, wy, wz, 0); // +X
                    if (isAir(chunk, x - 1, y, z)) addFace(mesh, wx, wy, wz, 1); // -X
                    if (isAir(chunk, x, y + 1, z)) addFace(mesh, wx, wy, wz, 2); // +Y
                    if (isAir(chunk, x, y - 1, z)) addFace(mesh, wx, wy, wz, 3); // -Y
                    if (isAir(chunk, x, y, z + 1)) addFace(mesh, wx, wy, wz, 4); // +Z
                    if (isAir(chunk, x, y, z - 1)) addFace(mesh, wx, wy, wz, 5); // -Z
                }
            }
        }
    }
    // worldMtx is now UNLOCKED. We hold our local MeshData copy.
    // Other threads can freely access the world again.

    // -- Step 4: Deposit the finished mesh into the upload queue.
    //
    // Another tight scope -- lock just long enough to push to the queue.
    {
        std::lock_guard<std::mutex> lock(engine.meshQueueMtx);

        // std::move(mesh) TRANSFERS ownership of the vertex/index vectors
        // into the queue WITHOUT copying them. After this line, mesh is empty.
        // This is important -- vertices/indices could be megabytes of data.
        // Copying would be very slow. Moving is essentially free (just pointer swap).
        engine.meshUploadQueue.push(std::move(mesh));
    }
    // meshQueueMtx is now unlocked.
    // The render thread will find this data next time it checks the queue.
}
