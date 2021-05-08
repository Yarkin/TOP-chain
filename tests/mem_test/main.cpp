#include <memory>
#include <vector>
#include <iostream>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "xbase/xutl.h"
#include "xbase/xlog.h"
#include <time.h>
using namespace top;
using namespace std;
void malloc_alloc(int size, std::vector<void *> &datas)
{
    srand(time(NULL));
    int64_t free_memory_size = 0;
    base::xsys_utl::get_memory_load(free_memory_size);
    std::cout << "memory alloc start " << free_memory_size << std::endl;
    uint32_t out_count = 0;
    uint32_t out_total_count = size;
    while (out_count++ < out_total_count)
    {
        auto size = rand() % 1024;
        void *ptr = malloc(size);
        memset(ptr, 0, size);
        datas.push_back(ptr);
    }
    base::xsys_utl::get_memory_load(free_memory_size);
    std::cout << "memory alloc finished " << free_memory_size << std::endl;
}

void free_data(std::vector<void *> &datas)
{
    int64_t free_memory_size = 0;
    int count = 0;
    for (auto iter = datas.begin(); iter != datas.end(); iter++)
    {
        free(*iter);
    }
    // malloc_trim(0);
    base::xsys_utl::get_memory_load(free_memory_size);
    std::cout << "memory after freed " << free_memory_size << std::endl;
}

int main(int argc, char *argv[])
{
    auto iret = mallopt(M_TRIM_THRESHOLD, 64 * 1024) ;
    std::cout << "mallopt " << iret<< std::endl;
    xinit_log("./xmem.log", true, true);
    xset_log_level(enum_xlog_level_debug);
    for (int i = 0; i < 100; i++)
    {
        std::vector<void *> datas;
        malloc_alloc(50 * 1024, datas);
        free_data(datas);
    }
    return 0;
}
