#include "toys/memcachedServer.h"
#include <signal.h>
#include <vector>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#ifdef HAVE_TCMALLOC
#include <gperftools/heap-profiler.h>
#include <gperftools/malloc_extension.h>
#endif
#include "event/eventLoop.h"
#include "bases/thread.h"

using namespace toys;
int main(int argc, char *argv[])
{
#ifdef HAVE_TCMALLOC
  MallocExtension::Initialize();
#endif
  int items = argc > 1 ? atoi(argv[1]) : 10000;
  int keylen = argc > 2 ? atoi(argv[2]) : 10;
  int valuelen = argc > 3 ? atoi(argv[3]) : 100;
  event::EventLoop loop;
  MemcachedServer server(&loop, std::vector<int>{9999});

  printf("sizeof(Item) = %zd\npid = %d\nitems = %d\nkeylen = %d\nvaluelen = %d\n",
         sizeof(Item), getpid(), items, keylen, valuelen);
  char key[256] = {0};
  string value;
  for (int i = 0; i < items; ++i)
  {
    snprintf(key, sizeof key, "%0*d", keylen, i);
    value.assign(valuelen, "0123456789"[i % 10]);
    ItemPtr item(new Item(key, valuelen + 2, 0));
    item->appendData(value, value.size());
    item->appendData("\r\n", 2);
    assert(item->endwithCRLF());
    bool exists = false;
    bool stored = server.setItem(item, Item::ADD);
    assert(stored);
    (void)stored;
    assert(!exists);
  }
  /*
  Inspector::ArgList arg;
  printf("==========\n%s\n",
         ProcessInspector::overview(HttpRequest::kGet, arg).c_str());
  */
  // TODO: print bytes per item, overhead percent
  fflush(stdout);
#ifdef HAVE_TCMALLOC
  char buf[8192];
  MallocExtension::instance()->GetStats(buf, sizeof buf);
  printf("%s\n", buf);
  HeapProfilerDump("end");

/*
  // only works for tcmalloc_debug
  int blocks = 0;
  size_t total = 0;
  int histogram[kMallocHistogramSize] = { 0, };
  MallocExtension::instance()->MallocMemoryStats(&blocks, &total, histogram);
  printf("==========\nblocks = %d\ntotal = %zd\n", blocks, total);
  for (int i = 0; i < kMallocHistogramSize; ++i)
  {
    printf("%d = %d\n", i, histogram[i]);
  }
*/
#endif
}
