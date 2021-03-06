
#include <vector>
#include <iostream>
#include <tr1/unordered_set>

#include "mem/thread.hpp"

using namespace boost;
using namespace std;
using namespace std::tr1;

#ifdef ALLOCATOR_ASSERT
mutex allocator_mtx;
unordered_set<void*> mem_set;
#endif

namespace mem
{
   
static bool init(void);

static pool *main_pool(new pool());
static vector<pool*> vec;
static pthread_key_t pool_key;
static bool started(init());

static void
cleanup_memsystem(void)
{
   delete main_pool;
   pthread_key_delete(pool_key);
}

static bool
init(void)
{
   int ret(pthread_key_create(&pool_key, NULL));
   assert(ret == 0);
   atexit(cleanup_memsystem);
   return true;
}

void
init(const size_t num_threads)
{
   vec.resize(num_threads);
}

void
create_pool(const size_t id)
{
   thread::id pid(this_thread::get_id());
   
   pool *pl(new pool());
   vec[id] = pl;
   pthread_setspecific(pool_key, pl);
}

pool*
get_pool(void)
{
   pool *pl((pool*)pthread_getspecific(pool_key));
   
   if(pl == NULL)
      return main_pool;
   
   return pl;
}

void
cleanup(const size_t num_threads)
{
   for(size_t i(0); i < num_threads; ++i)
      delete vec[i];

   vec.clear();

   pthread_setspecific(pool_key, NULL);

#ifdef ALLOCATOR_ASSERT
   assert(mem_set.size() == 0);
#endif
}

}
