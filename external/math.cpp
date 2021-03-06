
#include <cmath>

#include "runtime/list.hpp"
#include "external/math.hpp"

using namespace std;
using namespace runtime;

namespace vm
{
namespace external
{
   
argument
sigmoid(EXTERNAL_ARG(x))
{
   DECLARE_ARG(x, float_val);
   
   const float_val ret(1.0/(1.0 + exp(-x)));
   
   RETURN_ARG(ret);
}

argument
normalize(EXTERNAL_ARG(x))
{
   DECLARE_ARG(x, float_list*);
   float_list *ptr((float_list*)x);
   
   if(float_list::is_null(ptr)) {
      RETURN_ARG(x);
   }
   
   /* find max value */
   float_val max_value(ptr->get_head());
   ptr = ptr->get_tail();
   while(!float_list::is_null(ptr)) {
      const float_val val(ptr->get_head());
   
      if(val > max_value)
         max_value = val;
         
      ptr = ptr->get_tail();
   }
   
   float_val Z(0.0);
   ptr = (float_list*)x;
   
   while(!float_list::is_null(ptr)) {
      Z += std::exp(ptr->get_head() - max_value);
      ptr = ptr->get_tail();
   }
   
   const float_val logZ(std::log(Z));
   ptr = (float_list*)x;
   stack_float_list vals;
   while(!float_list::is_null(ptr)) {
      vals.push(ptr->get_head() - max_value - logZ);
      ptr = ptr->get_tail();  
   }
   
   float_list *ls(from_stack_to_list<stack_float_list, float_list>(vals));
   
   RETURN_ARG(ls);
}

argument
damp(EXTERNAL_ARG(ls1), EXTERNAL_ARG(ls2), EXTERNAL_ARG(fact))
{
   DECLARE_ARG(ls1, float_list *);
   DECLARE_ARG(ls2, float_list *);
   DECLARE_ARG(fact, float_val);
   float_list *ptr1((float_list*)ls1);
   float_list *ptr2((float_list*)ls2);
   
   float_list *nil(float_list::null_list());
   
   if(float_list::is_null(ptr1) || float_list::is_null(ptr2)) {
      
      RETURN_ARG(nil);
   }
   
   stack_float_list vals;
   
   while(!float_list::is_null(ptr1) && !float_list::is_null(ptr2)) {
      const float_val h1(ptr1->get_head());
      const float_val h2(ptr2->get_head());
      const float_val c(std::log(fact * std::exp(h2) +
         (1.0 - fact) * std::exp(h1)));
      
      vals.push(c);
      
      ptr1 = ptr1->get_tail();
      ptr2 = ptr2->get_tail();
   }
   
   float_list *ptr(from_stack_to_list<stack_float_list,float_list>(vals));
   
   RETURN_ARG(ptr);
}

argument
divide(EXTERNAL_ARG(ls1), EXTERNAL_ARG(ls2))
{
   DECLARE_ARG(ls1, float_list*);
   DECLARE_ARG(ls2, float_list*);
   
   float_list *ptr1((float_list*)ls1);
   float_list *ptr2((float_list*)ls2);
   
   stack_float_list vals;
   
   while(!float_list::is_null(ptr1) && !float_list::is_null(ptr2)) {
      vals.push(ptr1->get_head() - ptr2->get_head());
      
      ptr1 = ptr1->get_tail();
      ptr2 = ptr2->get_tail();
   }
   
   float_list *ptr(from_stack_to_list<stack_float_list, float_list>(vals));
      
   RETURN_ARG(ptr);
}

argument
convolve(EXTERNAL_ARG(bin_fact), EXTERNAL_ARG(ls))
{
   DECLARE_ARG(bin_fact, float_list *);
   DECLARE_ARG(ls, float_list *);
   const size_t length(float_list::length((float_list*)ls));
   
   stack_float_list vals;
   
   for(size_t x(0); x < length; ++x) {
      float_val sum(0.0);
      float_list *ptr((float_list*)ls);
      
      size_t y(0);
      while(!float_list::is_null(ptr)) {
         const float_val other(ptr->get_head());
         const float_val val_bin(float_list::get((float_list*)bin_fact, x + y * length, 0.0));
         sum += std::exp(val_bin + other);
         
         ptr = ptr->get_tail();
         ++y;
      }
      
      if(sum == 0) sum = std::numeric_limits<float_val>::min();
      
      sum = std::log(sum);
      
      vals.push(sum);
   }
   
   float_list *ptr(from_stack_to_list<stack_float_list, float_list>(vals));
      
   RETURN_ARG(ptr);
}

argument
addfloatlists(EXTERNAL_ARG(ls1), EXTERNAL_ARG(ls2))
{
   DECLARE_ARG(ls1, float_list*);
   DECLARE_ARG(ls2, float_list*);
   
   float_list *ptr1((float_list*)ls1);
   float_list *ptr2((float_list*)ls2);
   
   stack_float_list vals;
   
   while(!float_list::is_null(ptr1) && !float_list::is_null(ptr2)) {
      vals.push(ptr1->get_head() + ptr2->get_head());
      
      ptr1 = ptr1->get_tail();
      ptr2 = ptr2->get_tail();
   }
   
   float_list *ptr(from_stack_to_list<stack_float_list, float_list>(vals));
      
   RETURN_ARG(ptr);
}

}
}
