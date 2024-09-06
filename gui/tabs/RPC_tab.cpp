#include "pch-il2cpp.h"
#include "utility.h"
#include "gui-helpers.hpp"
#include "state.hpp"
#include "game.h"
#include "achievements.hpp"
#include "DirectX.h"
#include "imgui/imgui_impl_win32.h" // ImGui_ImplWin32_GetDpiScaleForHwnd
#include "theme.hpp" // ApplyTheme
#include <cstdlib>

namespace RPCTab {
    enum Groups {
        RPC
    };

    static bool openRPC = true;

    void CloseOtherGroups(Groups group) {
        openRPC = group == Groups::RPC;
    }

    void Render() {
        ImGui::SameLine(100 * State.dpiScale);
        ImGui::BeginChild("###RPC", ImVec2(500 * State.dpiScale, 0), true, ImGuiWindowFlags_NoBackground);
        if (TabGroup("RPC", openRPC)) {
            CloseOtherGroups(Groups::RPC);
        }
        ImGui::EndChild();
    }
}