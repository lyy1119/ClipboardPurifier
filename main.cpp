#include "cleaner.h"
#include "app_ui.h"

int main(int argc, char* argv[]) {
    run_cleaner_unit_tests();

    ClipboardToolApp app;
    return app.run(argc, argv);
}