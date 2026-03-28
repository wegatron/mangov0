#ifdef IMGUI_ENABLE_TEST_ENGINE
#include "editor_tests.h"
#include <imgui_te_engine.h>
#include <imgui_te_context.h>
#include <imgui/imgui.h>

void RegisterEditorTests(ImGuiTestEngine* engine) {
    // ── Sanity: ImGui frame loop runs without crash ──
    {
        ImGuiTest* t = IM_REGISTER_TEST(engine, "engine/sanity", "imgui_no_crash");
        t->TestFunc = [](ImGuiTestContext* ctx) {
            ctx->Yield(10);
        };
    }

    // ── Menu: File menu can be opened ──
    {
        ImGuiTest* t = IM_REGISTER_TEST(engine, "editor/menu", "file_menu_opens");
        t->TestFunc = [](ImGuiTestContext* ctx) {
            ctx->MenuClick("##MainMenuBar/File");
            IM_CHECK(ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup));
            ctx->KeyPress(ImGuiKey_Escape);
        };
    }

    // ── Menu: Keyboard navigation in File menu ──
    {
        ImGuiTest* t = IM_REGISTER_TEST(engine, "editor/menu", "menu_navigation");
        t->TestFunc = [](ImGuiTestContext* ctx) {
            ctx->MenuClick("##MainMenuBar/File");
            ctx->KeyPress(ImGuiKey_DownArrow);
            ctx->KeyPress(ImGuiKey_Escape);
        };
    }

    // ── Panel: Asset panel is visible ──
    {
        ImGuiTest* t = IM_REGISTER_TEST(engine, "editor/panel", "asset_panel_visible");
        t->TestFunc = [](ImGuiTestContext* ctx) {
            IM_CHECK_NO_RET(ctx->WindowInfo("Asset").Window != nullptr);
        };
    }

    // ── Asset: click folder icon by name ──
    // Each folder icon is a BeginGroup(Image + Text(basename))EndGroup in Asset panel.
    // The test engine finds the Text item by basename via ** wildcard and clicks its
    // position — inside the group rect, triggering IsItemClicked on the group.
    // ArgVariant selects which folder: 0=cube, 1=plane, 2=buster_drone, 3=sphere_on_plane
    {
        ImGuiTest* t = IM_REGISTER_TEST(engine, "editor/asset", "click_folder_icon");
        t->ArgVariant = 0; // default: cube
        t->TestFunc = [](ImGuiTestContext* ctx) {
            static const char* k_folders[] = { "cube", "plane", "buster_drone", "sphere_on_plane" };
            int idx = ctx->Test->ArgVariant;
            IM_ASSERT(idx >= 0 && idx < IM_ARRAYSIZE(k_folders));
            const char* folder_name = k_folders[idx];

            // Ensure Asset window exists and is the active (focused) tab so that
            // ImGui::Begin() returns true and constructFolderFiles() runs.
            ImGuiTestItemInfo win_info = ctx->WindowInfo("###Asset");
            IM_CHECK_NO_RET(win_info.ID != 0);
            ctx->WindowFocus("###Asset");
            ctx->Yield(2); // let tab switch take effect so folder icons are rendered

            // Search recursively for the InvisibleButton label matching the folder name
            char path[128];
            ImFormatString(path, sizeof(path), "###Asset/**/%s", folder_name);
            ImGuiTestItemInfo item = ctx->ItemInfo(path, ImGuiTestOpFlags_NoError);
            if (item.ID != 0)
            {
                ctx->MouseMoveToPos(item.RectFull.GetCenter());
                ctx->MouseClick(0);
            }
            else
            {
                // Fallback: set ref to Asset window then search
                ctx->SetRef("###Asset");
                ImFormatString(path, sizeof(path), "**/%s", folder_name);
                ImGuiTestItemInfo fallback = ctx->ItemInfo(path, ImGuiTestOpFlags_NoError);
                IM_CHECK_NO_RET(fallback.ID != 0);
                if (fallback.ID != 0)
                    ctx->ItemClick(path, 0, ImGuiTestOpFlags_NoError);
            }
            ctx->Yield(2);
        };
    }
}
#endif
