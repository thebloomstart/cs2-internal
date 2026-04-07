#pragma once
#include <vector>
#include <optional>
#include <memory>
#include <shared_mutex>
#include "sdk/interfaces/igameresourceservice.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/entity.h"

namespace EntityCache {
    struct cached_entity {
        bool valid() const noexcept {
            return controller && pawn;
        }

       bool update_pawn() noexcept {
            this->pawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(this->controller->GetPawnHandle());
            return this->pawn != nullptr;
        }

        CBaseHandle handle;
        CCSPlayerController* controller{ nullptr };
        C_CSPlayerPawn* pawn{ nullptr };
    };

    class entity_cache final {
    public:
        entity_cache(const entity_cache&) = delete;
        entity_cache& operator=(const entity_cache&) = delete;

        entity_cache(entity_cache&&) noexcept = default;
        entity_cache& operator=(entity_cache&&) noexcept = default;

        entity_cache() = default;
        ~entity_cache() = default;

        template<typename Func>
        void read_players(Func&& f) const {
            std::shared_lock lock(players_mutex);
            f(players);
        }

        template<typename Func>
        void write_players(Func&& f) {
            std::unique_lock lock(players_mutex);
            f(players);
        }

        [[nodiscard]] std::vector<cached_entity> get_players_snapshot() const {
            std::shared_lock lock(players_mutex);
            return players;
        }

        void clear_players() noexcept {
            std::unique_lock lock(players_mutex);
            players.clear();
        }

        void reserve_players(size_t size) {
            std::unique_lock lock(players_mutex);
            players.reserve(size);
        }

    private:
        mutable std::shared_mutex players_mutex;
        std::vector<cached_entity> players;
    };

    inline std::unique_ptr<entity_cache> g_entity_cache = std::make_unique<entity_cache>();
}