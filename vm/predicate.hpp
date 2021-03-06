
#ifndef PREDICATE_HPP
#define PREDICATE_HPP

#include <string>
#include <vector>
#include <assert.h>

#include "vm/types.hpp"
#include "vm/defs.hpp"
#include "utils/types.hpp"

namespace vm {

const size_t PRED_DESCRIPTOR_BASE_SIZE = 4;
const size_t PRED_ARGS_MAX = 32;
const size_t PRED_NAME_SIZE_MAX = 32;
const size_t PRED_AGG_INFO_MAX = 32;

class program;

class predicate {
private:
   friend class program;
   
   static predicate_id current_id;
   
   predicate_id id;
   std::string name;
   strat_level level;
   
   std::vector<field_type> types;
   std::vector<size_t> fields_size;
   std::vector<size_t> fields_offset;
   
   size_t tuple_size;
   
   typedef struct {
      field_num field;
      aggregate_type type;
      aggregate_safeness safeness;
      strat_level local_level;
      predicate_id remote_pred_id;
      predicate *remote_pred;
   } aggregate_info;
   
   aggregate_info *agg_info;
   
   bool is_route;
   bool is_linear;
   bool is_reverse_route;
   bool is_action;
   
   void build_field_info(void);
   void build_aggregate_info(void);
   void cache_info(void);
   
   explicit predicate(void);

   virtual ~predicate(void);
   
public:
   
   inline bool is_aggregate(void) const { return agg_info != NULL; }
   
   inline aggregate_safeness get_agg_safeness(void) const { return agg_info->safeness; }
   inline bool is_unsafe_agg(void) const { return get_agg_safeness() == AGG_UNSAFE || get_agg_safeness() == AGG_IMMEDIATE; }
   inline const predicate *get_remote_pred(void) const { return agg_info->remote_pred; }
   strat_level get_agg_strat_level(void) const { return agg_info->local_level; }
   
   inline bool is_route_pred(void) const { return is_route || is_reverse_route; }

	inline bool is_reverse_route_pred(void) const { return is_reverse_route; }

   inline bool is_linear_pred(void) const { return is_linear; }

   inline bool is_action_pred(void) const { return is_action; }
   
   inline field_num get_aggregate_field(void) const { return agg_info->field; }
   inline aggregate_type get_aggregate_type(void) const { return agg_info->type; }
   
   inline predicate_id get_id(void) const { return id; }
   
   inline size_t num_fields(void) const { return types.size(); }
   
   inline field_type get_field_type(const field_num field) const { return types[field]; }
   inline size_t get_field_size(const field_num field) const { return fields_size[field]; }
   
   inline std::string get_name(void) const { return name; }
   
   inline size_t get_size(void) const { return tuple_size; }
   
   inline strat_level get_strat_level(void) const { return level; }
   
   void print(std::ostream&) const;
   
   static predicate* make_predicate_from_buf(utils::byte *buf, code_size_t *code_size, const predicate_id);
};

std::ostream& operator<<(std::ostream&, const predicate&);

}

#endif
