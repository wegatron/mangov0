#include <editor/editor.h>
#ifdef IMGUI_ENABLE_TEST_ENGINE
#include <imgui_te_engine.h>
#include <imgui_te_exporters.h>
#include <engine/functional/global/engine_context.h>
#include "editor_tests.h"
#endif

#include <cstdio>
#include <cstring>

// ── CLI arg helpers ──────────────────────────────────────────────────────────
static const char* find_arg(int argc, char** argv, const char* flag) {
    for (int i = 1; i < argc - 1; ++i)
        if (strcmp(argv[i], flag) == 0)
            return argv[i + 1];
    return nullptr;
}
static bool has_flag(int argc, char** argv, const char* flag) {
    for (int i = 1; i < argc; ++i)
        if (strcmp(argv[i], flag) == 0)
            return true;
    return false;
}

int main(int argc, char **argv) {
    mango::Editor *editor = new mango::Editor;
    editor->init();

#ifdef IMGUI_ENABLE_TEST_ENGINE
    ImGuiTestEngine* engine = static_cast<ImGuiTestEngine*>(mango::g_engine.getTestEngine());
    RegisterEditorTests(engine);

    // ── CLI-driven test execution ────────────────────────────────────────────
    // Usage:
    //   --test <filter>      Queue tests matching filter (e.g. "editor/asset", "all")
    //   --exit-on-done       Close window automatically when queue is empty
    //   --export <file.xml>  Write JUnit XML result file
    //   --verbose            Log to TTY (stdout)
    //
    // Examples:
    //   mango_editor.exe --test all --exit-on-done
    //   mango_editor.exe --test "editor/asset" --exit-on-done --export results.xml
    //   mango_editor.exe --test "editor/asset/click_folder_icon" --exit-on-done --verbose
    const char* test_filter  = find_arg(argc, argv, "--test");
    const char* export_file  = find_arg(argc, argv, "--export");
    bool exit_on_done        = has_flag(argc, argv, "--exit-on-done");
    bool verbose             = has_flag(argc, argv, "--verbose");

    if (test_filter) {
        ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(engine);
        test_io.ConfigNoThrottle       = true;                       // skip vsync, run fast
        test_io.ConfigRunSpeed         = ImGuiTestRunSpeed_Fast;
        test_io.ConfigLogToTTY         = verbose;
        test_io.ConfigWatchdogKillTest = 10.0f;                      // fail stuck tests quickly in CLI mode

        if (export_file) {
            test_io.ExportResultsFilename = export_file;
            test_io.ExportResultsFormat   = ImGuiTestEngineExportFormat_JUnitXml;
        }

        const char* filter = (strcmp(test_filter, "all") == 0) ? nullptr : test_filter;
        ImGuiTestEngine_QueueTests(engine, ImGuiTestGroup_Tests, filter,
                                   ImGuiTestRunFlags_RunFromCommandLine);
        printf("[TestEngine] Queued tests (filter: \"%s\")\n", test_filter);
    }

    ImGuiTestEngine_InstallDefaultCrashHandler();

    // exit_check lambda: stop loop once all queued tests finish.
    // Guard with seen_non_empty to avoid exiting on frame 0 before the
    // coroutine has had a chance to drain the queue.
    auto exit_check = [&, seen_non_empty = false]() mutable -> bool {
        if (!exit_on_done || !test_filter) return false;
        // Override any INI-persisted capture settings that would cause
        // IM_ASSERT(0) in CaptureScreenshot when ScreenCaptureFunc is null.
        ImGuiTestEngineIO& io = ImGuiTestEngine_GetIO(engine);
        io.ConfigCaptureOnError = false;
        io.ConfigCaptureEnabled = false;
        if (!ImGuiTestEngine_IsTestQueueEmpty(engine)) {
            seen_non_empty = true;
            return false;
        }
        return seen_non_empty;
    };

    editor->run(exit_check);

    // Gather results BEFORE destroy() — engine is freed inside editor->destroy().
    int exit_code = 0;
    if (test_filter) {
        int count_tested = 0, count_success = 0;
        ImGuiTestEngine_GetResult(engine, count_tested, count_success);
        printf("[TestEngine] Results: %d/%d passed\n", count_success, count_tested);
        if (export_file)
            printf("[TestEngine] Report written to: %s\n", export_file);
        if (exit_on_done)
            exit_code = (count_tested > 0 && count_success == count_tested) ? 0 : 1;
    }
#else
    editor->run();
    const int exit_code = 0;
#endif

    editor->destroy();
    delete editor;

    return exit_code;
}
