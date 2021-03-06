
#ifndef SCHED_THREAD_QUEUE_BUFFER_HPP
#define SCHED_THREAD_QUEUE_BUFFER_HPP

#include "conf.hpp"

#include <vector>

#include "vm/predicate.hpp"
#include "queue/simple_linear_pqueue.hpp"
#include "sched/base.hpp"
#include "mem/allocator.hpp"

namespace sched
{
   
class queue_buffer
{
public:
   
   typedef queue::simple_linear_pqueue<process::work> queue;
   
private:
  
   size_t total;
   
   std::vector<queue, mem::allocator<queue> > buffered_work;
   
public:
   
   inline bool empty(void) const { return total == 0; }
   
   inline bool empty(const vm::process_id id) const
   {
      assert(id < buffered_work.size());
      return buffered_work[id].empty();
   }
   
   queue& get_queue(const vm::process_id id) {
      assert(id < buffered_work.size());
      return buffered_work[id];
   }
   
   void clear_queue(const vm::process_id);
   
   bool push(const vm::process_id, process::work&);
   
   void init(const size_t);
   
   explicit queue_buffer(void):
      total(0)
   {
   }
   
   ~queue_buffer(void)
   {
      assert(empty());
   }
};

}

#endif
