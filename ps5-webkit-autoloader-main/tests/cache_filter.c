#include <assert.h>
#include "exploit_cache.h"

int main(void) {
    const char *relapse = "/app/test/relapse/index.html?autoload=payload.elf";
    const char *offset = "/app/test/relapse/offsets/13.40.js";
    const char *slopkit = "/app/test/slopkit/slopkit/p2jb.html?go=1";
    const char *umtx2 = "/app/test/umtx2/index.html";
    assert(wkal_cache_keep(relapse, 13.40f, "auto"));
    assert(wkal_cache_keep(offset, 13.40f, "auto"));
    assert(wkal_cache_keep(relapse, 13.60f, "auto"));
    assert(!wkal_cache_keep(slopkit, 13.40f, "auto"));
    assert(!wkal_cache_keep(umtx2, 13.40f, "auto"));
    assert(wkal_cache_keep(relapse, 12.70f, "auto"));
    assert(wkal_cache_keep(relapse, 12.60f, "auto"));
    assert(wkal_cache_keep(relapse, 7.00f, "auto"));
    assert(wkal_cache_keep(slopkit, 12.70f, "auto"));
    assert(!wkal_cache_keep(slopkit, 13.60f, "auto"));
    assert(wkal_cache_keep(umtx2, 5.50f, "auto"));
    assert(!wkal_cache_keep(relapse, 5.50f, "auto"));
    assert(wkal_cache_keep(relapse, 7.00f, "relapse"));
    assert(!wkal_cache_keep(slopkit, 7.00f, "relapse"));
    assert(!wkal_cache_keep(relapse, 13.40f, "p2jb"));
    assert(wkal_cache_keep(slopkit, 13.40f, "p2jb"));
    assert(wkal_cache_keep(relapse, 0, "auto"));
    assert(wkal_cache_keep("/app/test/payloads/payload.elf", 13.40f, "auto"));
    assert(wkal_cache_keep("/app/test/__complete__", 13.40f, "auto"));
    return 0;
}
