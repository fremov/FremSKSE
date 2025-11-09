#include "include/MenuHandler.h"

bool g_shouldShowHints;

auto MenuHandler::get_singleton() noexcept -> MenuHandler* {
    static MenuHandler instance;
    return std::addressof(instance);
}

auto MenuHandler::register_() -> void {
    if (const auto ui = RE::UI::GetSingleton()) {
        ui->AddEventSink(get_singleton());
    }
}

auto MenuHandler::ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) -> RE::BSEventNotifyControl {
    if (event) {
        if (auto ui = RE::UI::GetSingleton()) {
            bool isLoadingMenu = ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME);
            bool shouldShowWidget = !(
                ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::TweenMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::GiftMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::MagicMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::StatsMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::JournalMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::LockpickingMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::SleepWaitMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::RaceSexMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::MapMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::FaderMenu::MENU_NAME) ||
                ui->IsMenuOpen(RE::CursorMenu::MENU_NAME)
                );

            // ”правление видимостью полосок навыков 
            PrismaUI->Invoke(view, ("toggleSkillsContainer(" + std::string(shouldShowWidget && !isLoadingMenu ? "true" : "false") + ")").c_str());

            // ”правление видимостью виджета сохранени€
            PrismaUI->Invoke(view, ("toggleSaveWidgetContainer(" + std::string(shouldShowWidget && !isLoadingMenu ? "true" : "false") + ")").c_str());

            // ”правление видимостью виджета локаций
            PrismaUI->Invoke(view, ("toggleLocationWidgetContainer(" + std::string(shouldShowWidget && !isLoadingMenu ? "true" : "false") + ")").c_str());

            // ”правление видимостью виджета штрафов
            PrismaUI->Invoke(view, ("updateCrimeContainerVisibility(" + std::string(shouldShowWidget && !isLoadingMenu ? "true" : "false") + ")").c_str());

            // ”правление видимостью виджета подсказок
            PrismaUI->Invoke(view, ("setHintContainerVisibility(" + std::string(shouldShowWidget && !isLoadingMenu ? "true" : "false") + ")").c_str());
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}