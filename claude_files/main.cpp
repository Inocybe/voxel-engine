#include "Thread.hpp"
#include "ThreadPool.hpp"
#include "VoxelEngine.hpp"
#include "MeshWorker.hpp"
#include "Renderer.hpp"

// ============================================================
// main.cpp
// Wires everything together.
//
// Thread ownership summary:
//
//   Main thread  -- creates OpenGL context, runs render loop,
//                   owns loadedMeshes (inside runRenderLoop).
//                   The ONLY thread allowed to call OpenGL.
//
//   Worker threads (via ThreadPool) -- each runs meshWorker()
//                   for one chunk. Reads world data, builds
//                   CPU-side MeshData, deposits to queue.
//                   Never touch OpenGL.
//
// Data flow:
//
//   engine.world  (written by main before threads start)
//       |
//       | worker threads READ with worldMtx locked
//       v
//   engine.meshUploadQueue  (workers WRITE with meshQueueMtx locked)
//       |
//       | render thread READS with meshQueueMtx locked, then unlocks
//       v
//   glBufferData() -- GPU upload, no mutex, render thread only
//       |
//       v
//   loadedMeshes  -- local to render thread, no mutex needed
//       |
//       v
//   glDrawElements() -- draw call each frame
// ============================================================

int main() {

    // -- Step 1: Create the shared engine state.
    // This one object is passed by reference to every thread.
    // It holds the world data, the upload queue, and the mutexes
    // that coordinate access to both.
    VoxelEngine engine;

    // -- Step 2: Populate the world with chunk data.
    // Done here on the main thread BEFORE worker threads start,
    // so no mutex is needed for this initial population.
    // (No other thread exists yet to race against.)
    for (int cx = 0; cx < 4; cx++) {
        for (int cz = 0; cz < 4; cz++) {
            Chunk chunk;
            chunk.chunkX = cx;
            chunk.chunkZ = cz;

            // Fill with some blocks as a test -- solid from y=0 to y=63.
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int y = 0; y < 64; y++) {
                    for (int z = 0; z < CHUNK_SIZE; z++) {
                        int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * WORLD_HEIGHT;
                        chunk.blocks[index].type = 1; // stone
                    }
                }
            }

            engine.world[{cx, cz}] = std::move(chunk);
        }
    }

    // -- Step 3: Launch worker threads to build chunk meshes.
    //
    // ThreadPool manages a vector of Thread objects.
    // Each addTask() call spins up one OS thread running meshWorker.
    //
    // std::ref(engine) -- MUST use std::ref when passing a reference
    // to a thread. Threads internally copy their arguments, and a raw
    // reference would become dangling. std::ref wraps it in a
    // reference_wrapper that survives the copy safely.
    ThreadPool pool;

    for (int cx = 0; cx < 4; cx++) {
        for (int cz = 0; cz < 4; cz++) {
            pool.addTask(meshWorker, std::ref(engine), cx, cz);
        }
    }

    // -- Step 4: Run the render loop on the main thread.
    //
    // This function loops forever, each frame:
    //   - non-blocking check of meshUploadQueue
    //   - uploads any finished meshes to the GPU
    //   - draws all uploaded chunks
    //   - swap buffers / poll events
    //
    // It exits when engine.running becomes false.
    // (Set engine.running = false from an input handler or ESC key.)
    runRenderLoop(engine);

    // -- Step 5: Cleanup.
    //
    // When runRenderLoop returns, engine.running is false.
    // pool goes out of scope here, calling ThreadPool's destructor,
    // which destroys the vector<Thread>, which destroys each Thread,
    // which calls join() on each OS thread via Thread's destructor.
    //
    // All worker threads see engine.running == false at the top of
    // their loop and have already exited, so join() returns immediately.
    //
    // This is the full RAII chain:
    //   pool destructs
    //     -> vector<Thread> destructs
    //       -> each Thread destructs
    //         -> each m_thread.join() called
    //           -> all OS threads confirmed finished
    //             -> main() returns cleanly

    return 0;
}
