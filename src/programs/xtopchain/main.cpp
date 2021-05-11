#include <assert.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#ifdef __LINUX_PLATFORM__
#include <malloc.h>
#endif
#include "xchaininit/xinit.h"
#include "xpbase/base/top_utils.h"
#include "xchaininit/version.h"
#include "xdata/xoperation_config.h"

#include "xmetrics/xmetrics.h"
#include "tcmalloc_options.h"

using namespace top;

void malloc_thread_proc() {
    #ifdef __LINUX_PLATFORM__
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(64));
        malloc_trim(0);
    }
    #endif
}

void start_monitor_thread() {
    std::thread(malloc_thread_proc).detach();
}

int main(int argc, char * argv[]) {
    #ifdef __LINUX_PLATFORM__
    auto iret = mallopt(M_TRIM_THRESHOLD, 64 * 1024) ;
    #endif
    StartCheckHeap();

    // use this to join or quit network, config show_cmd = false will come here
    // auto elect_mgr = elect_main.elect_manager();
    try {
        if (argc < 2) {
            std::cout << "usage: xtopchain config_file1 [config_file2]" << std::endl;
            _Exit(0);
        }

        if (strcmp(argv[1], "-v") == 0) {
            print_version();
            _Exit(0);
        } else {
            print_version();
            std::string config_file1 = argv[1];
            std::string config_file2;
            if (argc > 2) {
                config_file2 = argv[2];
            }
            start_monitor_thread();
            topchain_init(config_file1, config_file2);
        }
    } catch (const std::runtime_error& e) {
        xerror("Unhandled runtime_error Exception reached the top of main %s", e.what());
    } catch (const std::exception& e) {
        xerror("Unhandled Exception reached the top of main %s", e.what());
    } catch (...) {
        xerror("unknow exception");
    }
    return 0;
}
