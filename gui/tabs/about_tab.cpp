#include "pch-il2cpp.h"
#include "settings_tab.h"
#include "utility.h"
#include "gui-helpers.hpp"
#include "state.hpp"
#include "game.h"
#include "achievements.hpp"
#include "DirectX.h"
#include "imgui/imgui_impl_win32.h" // ImGui_ImplWin32_GetDpiScaleForHwnd
#include "theme.hpp" // ApplyTheme
#include <cstdlib>

namespace AboutTab {
    enum Groups {
        Welcome,
        Credits,
        Help
    };

    static bool openWelcome = true; //default to welcome tab group
    static bool openCredits = false;
    static bool openHelp = false;

    void CloseOtherGroups(Groups group) {
        openWelcome = group == Groups::Welcome;
        openCredits = group == Groups::Credits;
        openHelp = group == Groups::Help;
    }

    void Render() {
        ImGui::SameLine(100 * State.dpiScale);
        ImGui::BeginChild("###About", ImVec2(500 * State.dpiScale, 0), true, ImGuiWindowFlags_NoBackground);
        if (TabGroup("Welcome", openWelcome)) {
            CloseOtherGroups(Groups::Welcome);
        }
        ImGui::SameLine();
        if (TabGroup("Credits", openCredits)) {
            CloseOtherGroups(Groups::Credits);
        }
        ImGui::SameLine();
        if (TabGroup("Help", openHelp)) {
            CloseOtherGroups(Groups::Help);
        }

        if (openWelcome) {
            std::string welcomeText = std::format("Welcome {}to BombHacker {} !", State.HasOpenedMenuBefore ? "back " : "", State.SickoVersion);
            ImGui::Text(welcomeText.c_str());
            ImGui::Text("BombHacker is a highly sophisticated cheat menu for Among Us based on SickoMenu 3.6.1 [Not Original].");
            ImGui::Text("SickoMenu is a highly sophisticated cheat menu for Among Us based on AmongUsMenu.");
            ImGui::Text("It promises more features and regular updates.");
            ImGui::Text("Make sure to use the latest version of SickoMenu from the GitHub repository!");
            ImGui::Text("Use the \"Check for Updates\" button to download the latest release!");
            if (ImGui::Button("GitHub")) {
                OpenLink("https://github.com/g0aty/SickoMenu");
            }
            ImGui::SameLine();
            if (ImGui::Button("Check for Updates")) {
                OpenLink("https://github.com/g0aty/SickoMenu/releases/latest");
            }
            ImGui::Text("Join the Discord server for support, bug reports, and sneak peeks!");
            if (ImGui::Button("Join our Discord!")) {
                OpenLink("https://dsc.gg/sickos"); //sickomenu discord invite
            }
            ImGui::Text("SickoMenu is a free and open-source software.");
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ("If you've paid for this menu, demand a refund immediately!"));
            //hopefully stop people from reselling a foss menu for actual money

            ImGui::Dummy(ImVec2(5, 5) * State.dpiScale);
            if (ToggleButton("Fuck Hudson", &State.FuckHudson)) State.Save();
        }

        if (openCredits) {
            ImGui::Text("SickoMenu is a fork of AmongUsMenu, go check it out!");
            if (ImGui::Button("AmongUsMenu")) {
                OpenLink("https://github.com/BitCrackers/AmongUsMenu");
            }
            ImGui::Text("Some people who contributed to AUM:");
            if (ImGui::Button("KulaGGin")) {
                OpenLink("https://github.com/KulaGGin");
            }
            ImGui::SameLine();
            ImGui::Text("(Helped with some ImGui code for replay system)");

            if (ImGui::Button("tomsa000")) {
                OpenLink("https://github.com/tomsa000");
            }
            ImGui::SameLine();
            ImGui::Text("(Helped with fixing memory leaks and smart pointers)");

            if (ImGui::Button("cddjr")) {
                OpenLink("https://github.com/cddjr");
            }
            ImGui::SameLine();
            ImGui::Text("(Helped in updating to the Fungle release)");

            ImGui::Text("Everyone else who contributed to AUM and I couldn't list here.");
            ImGui::Text("Thank you for making SickoMenu possible!");
        }
        if (openHelp) {
            ImGui::Dummy(ImVec2(5, 5) * State.dpiScale);
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ("Disable Host Anticheat -> Turn off anticheat, moving the server to \"modded\" region!\n\nSpoof Modded Host -> Spoofing modded server host!\n\nEnable Anticheat (SMAC) -> Turn on advance anticheat, based on SickoMenu!\n\nFlip Skeld -> Flip Skeld map (works only with enabled \"Disable Host Anticheat\")\n\nPlayers Count -> The maximum number of players who can join the your lobby!\n\nDisable Specific RPC Call ID -> Disables certain RPC Call (from cheat-detect)"));
        }
            ImGui::EndChild();
        }
    }