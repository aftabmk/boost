#pragma once

#include <atomic>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace kv_cache {

// ============================================================================
// Constants
// ============================================================================

constexpr size_t SLOT_SIZE = 128 * 1024;           // 128 KB
constexpr size_t BLOCK_SIZE = 2 * 1024 * 1024;     // 2 MB
constexpr size_t SLOTS_PER_BLOCK = BLOCK_SIZE / SLOT_SIZE;  // 16
constexpr size_t MAX_VERSIONS = 3;                 // head + 2 previous
constexpr size_t MAX_CHUNKS = 16;                  // max chunked payload
constexpr uint32_t INVALID_BLOCK = 0xFFFFFFFF;
constexpr uint8_t INVALID_SLOT = 0xFF;

// Flags for version entries
constexpr uint8_t FLAG_CONTIGUOUS = 0x01;
constexpr uint8_t FLAG_CHUNKED = 0x02;
constexpr uint8_t FLAG_TOMBSTONE = 0x04;
constexpr uint8_t FLAG_MULTI_SLOT = 0x08;

// Block states
constexpr uint8_t BLOCK_ACTIVE = 0;
constexpr uint8_t BLOCK_FRAGMENTED = 1;
constexpr uint8_t BLOCK_COMPACTING = 2;
constexpr uint8_t BLOCK_FREE = 3;

// ============================================================================
// Payload Header (at start of each slot, 32 bytes)
// ============================================================================

struct alignas(16) PayloadHeader {
    uint32_t key_hash;      // Low 32 bits of key hash for validation
    uint32_t version;       // Version number
    uint32_t length;        // Actual payload length
    uint32_t checksum;      // xxHash32 of payload
    uint16_t refcount;      // Number of version entries pointing here
    uint16_t flags;         // FLAG_MULTI_SLOT, etc.
    uint8_t  run_length;    // For multi-slot: number of contiguous slots
    uint8_t  run_index;     // For multi-slot: 0 = start, 1..N = continuation
    uint32_t padding;       // Align to 32 bytes
};

// Continuation header for multi-slot payloads (slots 1..N)
struct alignas(16) ContinuationHeader {
    uint32_t parent_slot;   // Start slot index
    uint32_t run_index;     // 1..run_length-1
    uint32_t checksum;      // xxHash32 of this chunk
    uint32_t padding;
};

// ============================================================================
// Version Entry (16 bytes) - stored in key slot's version ring
// ============================================================================

struct alignas(16) VersionEntry {
    uint32_t block_id;      // Which 2 MB block
    uint8_t  slot_idx;      // 0-15, which 128 KB slot within block
    uint8_t  flags;         // CONTIGUOUS, CHUNKED, TOMBSTONE, MULTI_SLOT
    uint16_t run_length;    // For contiguous: number of slots (1-16)
    uint32_t length;        // Actual payload length
    uint32_t version;       // Monotonically increasing version number
};

// ============================================================================
// Key Slot (64 bytes = 1 cache line)
// ============================================================================

struct alignas(64) KeySlot {
    uint64_t hash;                      // xxHash64 of key
    std::atomic<uint8_t> head_idx{0};   // 0, 1, or 2 - which ver_idx is current head
    uint8_t  flags;                     // Key-level flags (1 = initialized)
    uint16_t key_len;                   // Original key length
    uint32_t pad;                       // Alignment
    
    VersionEntry ver_idx[MAX_VERSIONS]; // 3 x 16B = 48B
    
    // Total: 16 + 48 = 64 bytes
};

static_assert(sizeof(KeySlot) == 64, "KeySlot must be 64 bytes");

// ============================================================================
// Chunk Table (for chunked/fallback allocation, 64 bytes)
// ============================================================================

struct alignas(16) ChunkEntry {
    uint32_t block_id;
    uint8_t  slot_idx;
    uint8_t  chunk_idx;
    uint16_t checksum;
};

struct alignas(64) ChunkTable {
    uint8_t num_chunks;
    uint8_t flags;
    uint16_t total_length;
    uint32_t checksum;
    ChunkEntry chunks[MAX_CHUNKS];
    uint8_t padding[64 - 8 - MAX_CHUNKS * 8];
};

static_assert(sizeof(ChunkTable) == 64, "ChunkTable must be 64 bytes");

// ============================================================================
// Block Metadata (32 bytes)
// ============================================================================

struct alignas(16) BlockEntry {
    void* base_addr;           // mmap'd 2 MB huge page address
    uint16_t next_slot;        // Bump pointer for contiguous alloc (0-16)
    uint16_t used_slots;       // Count of live slots
    uint16_t largest_free_run; // Max contiguous free slots
    uint8_t state;             // ACTIVE, FRAGMENTED, COMPACTING, FREE
    uint8_t alloc_mode;        // 0=CONTIGUOUS, 1=CHUNKED_FALLBACK
    uint16_t free_bitmap;      // 16 bits: 1=free, 0=used
    uint32_t padding;
};

static_assert(sizeof(BlockEntry) == 32, "BlockEntry size check");

} // namespace kv_cache