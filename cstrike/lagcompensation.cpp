#include "lagcompensation.h"
#include <future>
#include <vector>
#include <mutex>
#include "core/sdk.h"

static std::mutex records_mutex;
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))

void PlayerLog::run() {
    if (!SDK::LocalPawn)
        return;

    static const size_t num_threads = min(std::thread::hardware_concurrency(), size_t(8));

    static std::vector<std::future<void>> futures;
    futures.clear();
    futures.reserve(num_threads);

    static std::mutex local_records_mutex;

    EntityCache::g_entity_cache->read_players([&](const auto& players) {
        const size_t players_size = players.size();
        if (players_size == 0) return;

        const size_t chunk_size = max(
            size_t(1),
            (players_size + num_threads - 1) / num_threads
        );

        for (size_t i = 0; i < num_threads && i * chunk_size < players_size; ++i) {
            const size_t start_idx = i * chunk_size;
            const size_t end_idx = min(start_idx + chunk_size, players_size);

            futures.push_back(std::async(std::launch::async, [&, start_idx, end_idx]() {
                static std::vector<lag_record_t> record_buffer;
                record_buffer.clear();
                record_buffer.reserve(32);

                for (size_t j = start_idx; j < end_idx; ++j) {
                    EntityCache::cached_entity cached_ent = players[j];

                    if (!cached_ent.controller) {
                        continue;
                    }

                    cached_ent.update_pawn();

                    auto entry_index = cached_ent.controller->GetRefEHandle().GetEntryIndex();

                    {
                        std::lock_guard<std::mutex> lock(local_records_mutex);
                        auto& log = g_player_records->player_records[entry_index];

                        if (!cached_ent.valid() ||
                            !cached_ent.pawn->IsOtherEnemy(SDK::LocalPawn) ||
                            !cached_ent.controller->IsPawnAlive() ||
                            cached_ent.pawn->GetHasImmunity()) {
                            log.records.clear();
                            continue;
                        }

                        lag_record_t new_data(cached_ent.pawn);
                        if (log.records.empty() || new_data.m_simulation_time > log.records.back().m_simulation_time) {
                            log.records.push_back(new_data);
                        }

                        while (log.records.size() > 32) {
                            log.records.pop_front();
                        }

                        while (log.records.size() > 1 && !log.records.front().is_in_bounds()) {
                            log.records.pop_front();
                        }

                        log.newest = log.get_record(true);
                        log.oldest = log.get_record(false);
                    }
                }
                }));
        }
        });

    for (auto& future : futures) {
        if (future.valid()) {
            future.wait();
        }
    }
}