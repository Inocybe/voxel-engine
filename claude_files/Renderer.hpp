#pragma once
#include "VoxelEngine.hpp"
#include <mutex>
#include <unordered_map>

// ============================================================
// Renderer.hpp
// The render loop. This runs on the MAIN thread because OpenGL
// requires all calls to come from the thread that created the
// OpenGL context (which is always the main thread).
//
// Each frame the render loop does two things:
//   1. Non-blocking drain of the mesh upload queue.
//      If worker threads have finished any meshes, upload them
//      to the GPU right now. If not, skip it and draw anyway.
//   2. Draw every chunk mesh that has been uploaded so far.
//
// "Non-blocking" means we never WAIT for worker threads here.
// The render loop keeps running at full speed every frame
// regardless of whether workers have finished anything.
// New chunks appear on screen as soon as their mesh is ready.
// ============================================================

// Forward declarations of OpenGL functions used below.
// In your real project these come from glad.h / glew.h.
// Listed here so the code compiles as a reference example.
extern void glGenVertexArrays(int n, unsigned int* arrays);
extern void glGenBuffers(int n, unsigned int* buffers);
extern void glBindVertexArray(unsigned int array);
extern void glBindBuffer(unsigned int target, unsigned int buffer);
extern void glBufferData(unsigned int target, long size, const void* data, unsigned int usage);
extern void glVertexAttribPointer(unsigned int index, int size, unsigned int type,
                                  bool normalized, int stride, const void* pointer);
extern void glEnableVertexAttribArray(unsigned int index);
extern void glDrawElements(unsigned int mode, int count, unsigned int type, const void* indices);
extern void glDeleteVertexArrays(int n, const unsigned int* arrays);
extern void glDeleteBuffers(int n, const unsigned int* buffers);

// OpenGL constants (normally from glad/glew)
constexpr unsigned int GL_ARRAY_BUFFER         = 0x8892;
constexpr unsigned int GL_ELEMENT_ARRAY_BUFFER = 0x8893;
constexpr unsigned int GL_STATIC_DRAW          = 0x88B4;
constexpr unsigned int GL_FLOAT                = 0x1406;
constexpr unsigned int GL_UNSIGNED_INT         = 0x1405;
constexpr unsigned int GL_TRIANGLES            = 0x0004;

// ============================================================
// uploadMesh -- takes CPU-side MeshData and sends it to the GPU.
// Returns a ChunkMesh containing the OpenGL handles (VAO/VBO/EBO).
// Only call this from the render/main thread.
// ============================================================
inline ChunkMesh uploadMesh(const MeshData& data) {
    ChunkMesh mesh;
    mesh.chunkX     = data.chunkX;
    mesh.chunkZ     = data.chunkZ;
    mesh.indexCount = static_cast<int>(data.indices.size());

    // glGenVertexArrays creates a VAO handle (just an integer ID).
    // The VAO remembers: "for this chunk, vertex data is laid out
    // as: 3 floats for position, 2 floats for UV, 3 floats for normal."
    // Once set up you just bind the VAO and draw -- no need to
    // re-describe the layout every frame.
    glGenVertexArrays(1, &mesh.VAO);

    // glGenBuffers creates VBO and EBO handles (also just integer IDs).
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    // Bind the VAO first -- any buffer binds and attribute setups
    // below will be remembered by this VAO.
    glBindVertexArray(mesh.VAO);

    // Bind and upload vertex data to the VBO.
    // GL_ARRAY_BUFFER means "this is vertex data" (not indices).
    // data.vertices.size() * sizeof(float) = total bytes.
    // data.vertices.data() = raw C pointer to the vector's internal array.
    //   OpenGL needs a raw pointer, not a C++ vector object.
    // GL_STATIC_DRAW = hint to the GPU that this data won't change often.
    //   The driver uses this to decide where in GPU memory to store it.
    //   Other options: GL_DYNAMIC_DRAW (changes frequently),
    //                  GL_STREAM_DRAW (changes every frame).
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<long>(data.vertices.size() * sizeof(float)),
        data.vertices.data(),
        GL_STATIC_DRAW
    );

    // Bind and upload index data to the EBO.
    // GL_ELEMENT_ARRAY_BUFFER means "this is index data".
    // Indices tell OpenGL which vertices form each triangle.
    // Sharing vertices via indices is more efficient than repeating them.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<long>(data.indices.size() * sizeof(unsigned int)),
        data.indices.data(),
        GL_STATIC_DRAW
    );

    // Describe the vertex layout to the VAO.
    // This tells the shader: attribute 0 = position (3 floats),
    // with stride = 8 floats (3 pos + 2 UV + 3 normal = 8 floats per vertex),
    // starting at offset 0.
    // Your actual layout depends on how you packed your vertex data.
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return mesh;
}

// ============================================================
// runRenderLoop -- the main render loop.
// Call this from main() after OpenGL context creation.
// Runs until engine.running is set to false.
// ============================================================
inline void runRenderLoop(VoxelEngine& engine) {

    // loadedMeshes stores every chunk mesh that has been uploaded to the GPU.
    // Keyed by (chunkX, chunkZ) so we can look up and replace chunks when
    // they get remeshed (e.g. when a block is placed/broken).
    //
    // std::unordered_map vs std::map:
    //   map          -- sorted tree, O(log n) lookup, keys in order
    //   unordered_map -- hash table, O(1) average lookup, no order
    //   We don't need sorted chunk order, we just need fast lookup
    //   by position -- so unordered_map is the right choice.
    //
    // This lives entirely on the render thread. No mutex needed because
    // no other thread ever touches loadedMeshes.
    std::unordered_map<std::pair<int,int>, ChunkMesh, PairHash> loadedMeshes;

    // Main render loop -- runs every frame until the game closes.
    // engine.running is an atomic<bool> written by main() when the
    // player quits. Checking it here costs almost nothing (atomic
    // reads are a single CPU instruction).
    while (engine.running) {

        // ---- Step 1: Upload any finished meshes from the queue ----
        //
        // unique_lock is used here (not lock_guard) because we need to
        // manually call lock.unlock() before the GPU upload and
        // lock.lock() after. lock_guard cannot do this.
        //
        // Why unlock during GPU upload?
        //   glBufferData can take a non-trivial amount of time.
        //   If we held meshQueueMtx the whole time, worker threads
        //   could not deposit new finished meshes until we were done.
        //   Releasing the lock during the upload lets workers keep
        //   depositing freely while we do GPU work.
        {
            std::unique_lock<std::mutex> lock(engine.meshQueueMtx);

            // Drain everything currently in the queue this frame.
            // "while" not "if" -- there may be multiple meshes waiting.
            while (!engine.meshUploadQueue.empty()) {

                // front() reads the first element WITHOUT removing it.
                // pop() removes it WITHOUT returning the value.
                // These are separate in C++ by design: if pop() returned
                // the value AND threw an exception, you'd lose the data.
                // Separating them means: read first, then safely remove.
                //
                // std::move transfers ownership of the vectors inside MeshData
                // out of the queue element. Free, no copy.
                MeshData data = std::move(engine.meshUploadQueue.front());
                engine.meshUploadQueue.pop();

                // Release the lock before touching OpenGL.
                // Workers can now push to the queue while we upload.
                lock.unlock();

                // Upload to GPU -- only safe on the render thread.
                ChunkMesh chunkMesh = uploadMesh(data);

                // Store in our local map so we draw it every frame.
                // If a mesh for this chunk already existed (remesh),
                // this overwrites it. Old GPU buffers should be deleted
                // first in a complete implementation (glDeleteBuffers).
                loadedMeshes[{data.chunkX, data.chunkZ}] = chunkMesh;

                // Re-acquire the lock before checking the queue again.
                lock.lock();
            }
        }
        // meshQueueMtx is now unlocked.

        // ---- Step 2: Draw every uploaded chunk ----
        //
        // "auto& [pos, mesh]" is C++17 STRUCTURED BINDINGS.
        // Instead of: auto& pair = entry; pair.first / pair.second
        // You write:  auto& [pos, mesh]
        // The compiler destructures the map entry into named variables.
        // The & means we're referencing the actual map entry, not copying it.
        for (auto& [pos, mesh] : loadedMeshes) {
            // Bind this chunk's VAO -- the GPU now knows the vertex layout.
            glBindVertexArray(mesh.VAO);

            // Draw using indices from the EBO.
            // GL_TRIANGLES = draw every 3 indices as one triangle.
            // mesh.indexCount = total number of indices.
            // GL_UNSIGNED_INT = each index is an unsigned int.
            // nullptr = use the currently bound EBO (set when VAO was built).
            //
            // glDrawElements is preferred over glDrawArrays because it
            // reuses shared vertices between triangles via the index buffer,
            // reducing the amount of data on the GPU.
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
        }

        // Your swap buffers and poll events go here:
        // glfwSwapBuffers(window);
        // glfwPollEvents();
    }

    // Cleanup GPU resources when the loop exits.
    for (auto& [pos, mesh] : loadedMeshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
}
