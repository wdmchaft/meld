
#include "sched/thread/queue_buffer.hpp"

using namespace vm;
using namespace process;

namespace sched
{

void
queue_buffer::clear_queue(const process_id id)
{
   assert(id < buffered_work.size());
   
   queue& q(get_queue(id));
   const size_t size(q.size());
   
   q.clear();
   total -= size;
}

bool
queue_buffer::push(const process_id id, work& work)
{
   assert(id < buffered_work.size());
   
   queue& q(get_queue(id));

   q.push(work, work.get_strat_level());
   
   ++total;
   
   return q.size() > THREADS_GLOBAL_WORK_FLUSH;
}

void
queue_buffer::init(const size_t num_threads)
{
   // init buffered queues
   for(size_t i(0); i < num_threads; ++i)
      buffered_work.push_back(queue(program::MAX_STRAT_LEVEL));
}

}
