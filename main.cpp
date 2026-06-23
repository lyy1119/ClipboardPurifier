#include "cleaner.h"
#include "appUi.h"

int main(int argc, char* argv[]) {
    run_cleaner_unit_tests();

    ClipboardToolApp app;
    return app.run(argc, argv);
}