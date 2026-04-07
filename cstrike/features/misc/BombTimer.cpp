//#include "../../core/menu.h"
//#include <chrono> // Для работы с таймером
//#include <iostream>
//
//// used: config variables
//#include "../../core/variables.h"
//
//// used: iinputsystem
//#include "../../core/interfaces.h"
//#include "../../sdk/interfaces/iengineclient.h"
//#include "../../sdk/interfaces/inetworkclientservice.h"
//#include "../../sdk/interfaces/iglobalvars.h"
//#include "../../sdk/interfaces/ienginecvar.h"
//#include "../dependencies/imgui/imgui.h"
//#include "../../core/config.h"
//
//#include <Windows.h>
//#include <thread>
//
//// used: overlay's context
//#include "../../features/visuals/overlay.h"
//
//// used: notifications
//#include "../../utilities/notify.h"
//#include <imgui/imgui_impl_dx11.h>
//#include "../../sdk/interfaces/ischemasystem.h"
//#include "../../sdk/entity.h"
//#include "../../core/sdk.h"
//
//
//#include "../../sdk/interfaces/cgameentitysystem.h"
//// used: cusercmd
//#include "../../sdk/datatypes/usercmd.h"
//
//// used: activation button
//#include "../../utilities/inputsystem.h"
//#include "../../sdk/interfaces/imaterialsystem.h"
//#include <unordered_map>
//#include <algorithm>
//#include <iostream>
//
//using GameTime_t = std::float_t;
//using GameTick_t = std::int32_t;
//
//struct Entity_t
//{
//    enum EType : unsigned int
//    {
//        INVALID = 0,
//        PLAYER,
//        PLANTED_C4
//    };
//
//    C_BaseEntity* pEntity = nullptr;
//    EType eType = EType::INVALID;
//};
//static std::vector<Entity_t> vecEntities;
//
//void MENU::BombTimer()
//{
//    if (!C_GET(bool, Vars.bBombTimer))
//        return;
//
//    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
//
//    vecEntities.clear(); // Очистка вектора перед обновлением
//    for (int i = 0; i < nHighestIndex; i++)
//    {
//        C_BaseEntity* Entity = I::GameResourceService->pGameEntitySystem->Get<C_BaseEntity>(i);
//        if (!Entity)
//            continue;
//
//        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
//        if (pEntity == nullptr)
//            continue;
//
//        SchemaClassInfoData_t* pClassInfo = nullptr;
//        pEntity->GetSchemaClassInfo(&pClassInfo);
//        if (pClassInfo == nullptr)
//            continue;
//
//        const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szName);
//
//        switch (uHashedName)
//        {
//        case FNV1A::HashConst("CCSPlayerController"):
//            vecEntities.emplace_back(Entity_t(pEntity, Entity_t::EType::PLAYER));
//            break;
//
//        case FNV1A::HashConst("C_PlantedC4"):
//            vecEntities.emplace_back(Entity_t(pEntity, Entity_t::EType::PLANTED_C4));
//            break;
//
//        default:
//            break;
//        }
//    }
//
//    // Проверяем, есть ли объект с типом PLANTED_C4 в vecEntities
//    C_PlantedC4* plantedBomb = nullptr;
//    for (const Entity_t& entity : vecEntities)
//    {
//        if (entity.eType == Entity_t::EType::PLANTED_C4)
//        {
//            plantedBomb = reinterpret_cast<C_PlantedC4*>(entity.pEntity);
//            break;
//        }
//    }
//
//    if (!plantedBomb) // Если бомба не заложена, просто выходим
//        return;
//
//    // Работаем с таймером
//    ImGuiStyle& style = ImGui::GetStyle();
//    ImGui::PushFont(FONT::pExtra);
//
//    static void* lastBomb = nullptr;
//    static int time_remaining = 0;
//
//    // Если новая бомба, сбрасываем таймер
//    if (lastBomb != plantedBomb)
//    {
//        time_remaining = static_cast<int>(plantedBomb->m_flTimerLength()); // Сбрасываем таймер
//        lastBomb = plantedBomb; // Обновляем ссылку на бомбу
//    }
//
//    // Уменьшение таймера
//    static auto last_update_time = std::chrono::steady_clock::now();
//    auto current_time = std::chrono::steady_clock::now();
//    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_update_time).count();
//
//    if (elapsed_time >= 1) // Каждую секунду
//    {
//        last_update_time = current_time;
//        if (time_remaining > 0)
//            time_remaining -= 1;
//    }
//
//    if (time_remaining < 0)
//        time_remaining = 0;
//
//    // Настройки окна
//    ImGui::SetNextWindowSize(ImVec2(200, 50)); // Установите ширину и высоту окна
//
//    ImGui::Begin(CS_XOR("BombInfo"), &MENU::bMainWindowOpened,
//        ImGuiWindowFlags_NoScrollbar |
//        ImGuiWindowFlags_NoScrollWithMouse |
//        ImGuiWindowFlags_NoCollapse |
//        ImGuiWindowFlags_NoResize |
//        ImGuiWindowFlags_NoTitleBar);
//
//    // Отображение информации
//    std::int32_t site = plantedBomb->m_nBombSite();
//    std::string site_str = "Bomb Site: ";
//    if (site == 1)
//        site_str += "B";
//    else if (site == 0)
//        site_str += "A";
//    else
//        site_str += "Unknown";
//
//    std::string time_str = "Time to Detonation: " + std::to_string(time_remaining) + "s";
//
//    ImGui::Text("%s", site_str.c_str());
//    ImGui::Text("%s", time_str.c_str());
//
//    ImGui::End();
//    ImGui::PopFont();
//}