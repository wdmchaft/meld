
#include <fstream>
#include <sys/stat.h>
#include <iostream>

#include "vm/program.hpp"
#include "db/tuple.hpp"
#include "vm/instr.hpp"
#include "db/database.hpp"
#include "utils/types.hpp"
#include "vm/state.hpp"
#ifdef USE_UI
#include "ui/macros.hpp"
#endif

using namespace std;
using namespace db;
using namespace vm;
using namespace vm::instr;
using namespace process;
using namespace utils;

namespace vm {

static const size_t PREDICATE_DESCRIPTOR_SIZE = sizeof(code_size_t) +
                                                PRED_DESCRIPTOR_BASE_SIZE +
                                                PRED_ARGS_MAX +
                                                PRED_NAME_SIZE_MAX +
                                                PRED_AGG_INFO_MAX;
strat_level program::MAX_STRAT_LEVEL(0);
size_t SETPRIO_PREDICATE_ID(1);
size_t SETCOLOR_PREDICATE_ID(2);
size_t SETEDGELABEL_PREDICATE_ID(3);

program::program(const string& filename):
   init(NULL)
{
   state::PROGRAM = this;
   
   ifstream fp(filename.c_str(), ios::in | ios::binary);
   
   if(!fp.is_open())
      throw load_file_error(filename);

   // read number of predicates
   byte buf[PREDICATE_DESCRIPTOR_SIZE];
   
   fp.read((char*)buf, sizeof(byte));
   
   const size_t num_predicates = (size_t)buf[0];
   
   predicates.resize(num_predicates);
   code_size.resize(num_predicates);
   code.resize(num_predicates);
   state::NUM_PREDICATES = num_predicates;
   
   // skip nodes
   int_val num_nodes;
   fp.read((char*)&num_nodes, sizeof(int_val));
   
   fp.seekg(num_nodes * database::node_size, ios_base::cur);

	// read string constants
	int_val num_strings;
	fp.read((char*)&num_strings, sizeof(int_val));
	
	default_strings.reserve(num_strings);
	
	for(int i(0); i < num_strings; ++i) {
		int_val length;
		
		fp.read((char*)&length, sizeof(int_val));
		
		char str[length + 1];
		fp.read(str, sizeof(char) * length);
		str[length] = '\0';
		default_strings.push_back(runtime::rstring::make_default_string(str));
	}

   // read predicate information
   for(size_t i(0); i < num_predicates; ++i) {
      code_size_t size;
      fp.read((char*)buf, PREDICATE_DESCRIPTOR_SIZE);
      
      predicates[i] = predicate::make_predicate_from_buf((unsigned char*)buf, &size, (predicate_id)i);
      code_size[i] = size;

      MAX_STRAT_LEVEL = max(predicates[i]->get_strat_level() + 1, MAX_STRAT_LEVEL);
		
		const string name(predicates[i]->get_name());
		
		if(name == "setprio")
			SETPRIO_PREDICATE_ID = i;
		else if(name == "setcolor")
			SETCOLOR_PREDICATE_ID = i;
		else if(name == "setedgelabel")
			SETEDGELABEL_PREDICATE_ID = i;
      
      if(predicates[i]->is_route_pred())
         route_predicates.push_back(predicates[i]);
   }
   
   safe = true;
   for(size_t i(0); i < num_predicates; ++i) {
      predicates[i]->cache_info();
      if(predicates[i]->is_aggregate() && predicates[i]->is_unsafe_agg())
         safe = false;
   }
   
   // read predicate code
   for(size_t i(0); i < num_predicates; ++i) {
      const size_t size = code_size[i];
      code[i] = new byte_code_el[size];
      
      fp.read((char*)code[i], size);
   }
}

program::~program(void)
{
   for(size_t i(0); i < num_predicates(); ++i) {
      delete predicates[i];
      delete [] code[i];
   }
   MAX_STRAT_LEVEL = 0;
}

predicate*
program::get_predicate(const predicate_id& id) const
{
   if((size_t)id >= num_predicates()) {
      return NULL;
   }
   
   return predicates[id];
}

predicate*
program::get_route_predicate(const size_t& i) const
{
   assert(i < num_route_predicates());
   
   return route_predicates[i];
}

byte_code
program::get_bytecode(const predicate_id& id) const
{
   if((size_t)id >= num_predicates())
      return NULL;
      
   return code[id];
}

void
program::print_predicate_code(ostream& out, predicate* p) const
{
   out << "PROCESS " << p->get_name()
      << " (" << code_size[p->get_id()] << "):" << endl;
   instrs_print(code[p->get_id()], code_size[p->get_id()], 0, this, out);
   out << "END PROCESS;" << endl;
}

void
program::print_bytecode(ostream& out) const
{
   for(size_t i = 0; i < num_predicates(); ++i) {
      predicate_id id = (predicate_id)i;
      
      if(i != 0)
         cout << endl;
         
      print_predicate_code(out, get_predicate(id));
   }
}

void
program::print_bytecode_by_predicate(ostream& out, const string& name) const
{
	predicate *p(get_predicate_by_name(name));
	
	if(p == NULL) {
		cerr << "Predicate " << name << " not found." << endl;
		return;
	}
   print_predicate_code(out, get_predicate_by_name(name));
}

void
program::print_predicates(ostream& cout) const
{
   if(is_safe())
      cout << ">> Safe program" << endl;
   else
      cout << ">> Unsafe program" << endl;
   for(size_t i(0); i < num_predicates(); ++i) {
      cout << predicates[i] << " " << *predicates[i] << endl;
   }
}

#ifdef USE_UI
using namespace json_spirit;

Value
program::dump_json(void) const
{
	Array preds_json;

	for(size_t i(0); i < num_predicates(); ++i) {
		Object obj;
		predicate *pred(get_predicate((predicate_id)i));

		UI_ADD_FIELD(obj, "name", pred->get_name());

		Array field_types;

		for(size_t j(0); j < pred->num_fields(); ++j) {
			switch(pred->get_field_type(j)) {
				case FIELD_INT:
					UI_ADD_ELEM(field_types, "int");
					break;
				case FIELD_FLOAT:
					UI_ADD_ELEM(field_types, "float");
					break;
				case FIELD_NODE:
					UI_ADD_ELEM(field_types, "node");
					break;
				case FIELD_STRING:
					UI_ADD_ELEM(field_types, "string");
					break;
				default:
					throw type_error("Unrecognized field type " + to_string(pred->get_field_type(j)) + " (program::dump_json)");
			}
		}
		UI_ADD_FIELD(obj, "fields", field_types);

		UI_ADD_FIELD(obj, "route",
				pred->is_route_pred() ? UI_YES : UI_NIL);
		UI_ADD_FIELD(obj, "reverse_route",
				pred->is_reverse_route_pred() ? UI_YES : UI_NIL);
		UI_ADD_FIELD(obj, "linear",
			pred->is_linear_pred() ? UI_YES : UI_NIL);

		UI_ADD_ELEM(preds_json, obj);
	}

	return preds_json;
}
#endif

predicate*
program::get_predicate_by_name(const string& name) const
{
   for(size_t i(0); i < num_predicates(); ++i) {
      predicate *pred(get_predicate((predicate_id)i));
      
      if(pred->get_name() == name)
         return pred;
   }
   
   return NULL;
}

predicate*
program::get_init_predicate(void) const
{
   if(init == NULL) {
      init = get_predicate(INIT_PREDICATE_ID);
      if(init->get_name() != "_init") {
         cerr << "_init program should be predicate #" << (int)INIT_PREDICATE_ID << endl;
         init = get_predicate_by_name("_init");
      }
      assert(init->get_name() == "_init");
   }

   assert(init != NULL);
      
   return init;
}

predicate*
program::get_edge_predicate(void) const
{
   return get_predicate_by_name("edge");
}

tuple*
program::new_tuple(const predicate_id& id) const
{
   return new tuple(get_predicate(id));
}

}
