#pragma once
#include <array>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <utility>  // std::pair

// ============================================================
// VoxelEngine.hpp
// Defines all shared data structures and the VoxelEngine state.
//
// SYNCHRONIZATION PRIMITIVES -- what they are and why:
//
//   std::mutex
//     A mutual exclusion lock. Only ONE thread can hold it at
//     a time. Any other thread that tries to lock it will WAIT
//     until the holder unlocks it. Use when multiple threads
//     need to read or write the same complex data structure
//     (like a map, vector, or queue).
//     Cost: slightly slow -- involves OS coordination.
//
//   std::atomic<T>
//     A single variable that can be safely read/written by
//     multiple threads simultaneously with NO mutex needed.
//     Works only for simple types: bool, int, float, pointer.
//     Under the hood, the CPU guarantees the read/write is
//     one indivisible operation so threads can't half-read it.
//     Cost: very fast -- handled by the CPU itself.
//
//   std::lock_guard<std::mutex>
//     Locks a mutex on construction, unlocks on destruction.
//     RAII for mutexes. Use when you just need to lock, do work,
//     and release. Cannot manually unlock mid-scope.
//
//   std::unique_lock<std::mutex>
//     Same as lock_guard but more flexible -- can manually call
//     .lock() and .unlock() at any point. Required when you need
//     to temporarily release the lock mid-scope (e.g. during a
//     slow GPU upload while still inside a loop).
//
// WHICH MUTEX PROTECTS WHICH DATA:
//
//   worldMtx     --> protects: world (the chunk map)
//   meshQueueMtx --> protects: meshUploadQueue
//
//   The mutex and its data have no compile-time connection --
//   C++ does not enforce this. It is purely your discipline.
//   Convention: declare the mutex directly above the data it
//   protects, and only ever lock that mutex when touching that data.
// ============================================================

constexpr int CHUNK_SIZE   = 16;
constexpr int WORLD_HEIGHT = 256;

// ----------------------------------------------------------
// Block -- the smallest unit of the world.
// uint8_t = unsigned 8-bit integer (0-255).
// Chosen over int because it uses 1 byte instead of 4,
// and a chunk has 16*256*16 = 65,536 blocks -- memory matters.
// 0 = air (not rendered), anything else = a block type.
// ----------------------------------------------------------
struct Block {
    uint8_t type = 0;
};

// ----------------------------------------------------------
// Chunk -- a 16x256x16 column of blocks.
//
// Blocks are stored in a FLAT 1D array even though the world
// is 3D. This is intentional:
//   - 1D arrays are contiguous in memory (cache friendly)
//   - 3D arrays use pointer chains which cause cache misses
//
// To convert 3D coords (x, y, z) to a 1D index:
//   index = x + y * CHUNK_SIZE + z * CHUNK_SIZE * WORLD_HEIGHT
//
// Think of it as: x moves along a row, y moves to a new row,
// z moves to a new slab of rows.
// ----------------------------------------------------------
struct Chunk {
    std::array<Block, CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT> blocks;
    int chunkX = 0;
    int chunkZ = 0;
};

// ----------------------------------------------------------
// MeshData -- raw CPU-side vertex and index data.
// Built by worker threads. NOT on the GPU yet.
//
// vertices: flat array of floats -- x,y,z positions, UVs, normals
// indices:  which vertices form each triangle (avoids duplicating
//           shared vertices between adjacent triangles)
// ----------------------------------------------------------
struct MeshData {
    std::vector<float>        vertices;
    std::vector<unsigned int> indices;
    int chunkX = 0;
    int chunkZ = 0;
};

// ----------------------------------------------------------
// ChunkMesh -- GPU-side handles after upload.
// Lives only on the render thread.
//
// VAO (Vertex Array Object):
//   Remembers the FORMAT of your vertex data -- which parts
//   are positions, which are UVs, which are normals, etc.
//   Bind it once, then draw without re-describing the layout.
//
// VBO (Vertex Buffer Object):
//   The actual vertex data on the GPU -- positions, UVs, normals.
//
// EBO (Element Buffer Object):
//   The index data on the GPU -- tells OpenGL which vertices
//   form each triangle. More efficient than repeating vertices.
// ----------------------------------------------------------
struct ChunkMesh {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    int indexCount = 0;
    int chunkX = 0;
    int chunkZ = 0;
};

// ----------------------------------------------------------
// Pair hash -- std::unordered_map needs a hash function for
// its key type. std::pair<int,int> has no built-in hash,
// so we provide one. XOR-combining the two ints is a simple
// and fast approach.
// ----------------------------------------------------------
struct PairHash {
    size_t operator()(const std::pair<int,int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 16);
    }
};

// ----------------------------------------------------------
// VoxelEngine -- the shared state of the entire engine.
//
// Worker threads and the render thread all hold a reference
// to ONE instance of this struct (passed as VoxelEngine&).
// They communicate through the queues and maps inside it,
// using the mutexes to coordinate access.
//
// std::atomic<bool> running:
//   The global on/off switch for all threads.
//   Every thread's main loop checks this. When the player
//   closes the game, set running = false and every thread
//   will finish its current iteration and exit cleanly.
//   Atomic because it's written by one thread (main) and
//   read by many others simultaneously -- a regular bool
//   would be a data race (undefined behavior).
// ----------------------------------------------------------
struct VoxelEngine {

    // ---- World data ----------------------------------------
    // Only ONE thread should touch world at a time.
    // Lock worldMtx before reading or writing world.
    std::mutex worldMtx;
    std::unordered_map<std::pair<int,int>, Chunk, PairHash> world;

    // ---- Mesh upload queue ---------------------------------
    // Worker threads push finished MeshData here.
    // Render thread pops from here and uploads to GPU.
    // Lock meshQueueMtx before touching meshUploadQueue.
    std::mutex               meshQueueMtx;
    std::queue<MeshData>     meshUploadQueue;

    // ---- Global shutdown flag ------------------------------
    // Set to false to signal all threads to stop looping.
    // atomic<bool> so any thread can write it safely without a mutex.
    std::atomic<bool> running { true };
};
