//#include <sparsehash/sparse_hash_map>
//#include <sparsehash/sparse_hash_set>

#ifndef AFFINITY_HPP
#define AFFINITY_HPP
#include <tr1/unordered_map>
#include <deque>
#include <list>
#include <stdint.h>
#include <assert.h> 
using namespace std;

typedef uint8_t wsize_t;
typedef uint16_t func_t;
typedef pair<func_t,func_t> funcpair_t;
func_t totalFuncs;

/*
   struct affEntry{
   func_t first,second;
   affEntry();
   affEntry(func_t,func_t);
   affEntry(const affEntry&);
   affEntry& operator= (const affEntry&);
   bool operator== (const affEntry&) const;
   };
   */

funcpair_t reverse_pair(funcpair_t p){
	return funcpair_t(p.second,p.first);
}

funcpair_t unordered_funcpair(func_t s1,func_t s2){
  return (s1<s2)?(funcpair_t(s1,s2)):(funcpair_t(s2,s1));
}

struct funcpair_eq{
  bool operator()(funcpair_t s1, funcpair_t s2) const {
    if ((s1.first == s2.first) && (s1.second == s2.second))
      return true;
		/*
    if ((s1.second == s2.first) && (s1.first == s2.second))
      return true;
		*/
    return false;
  }
};


struct funcpair_hash{
  size_t operator()(const funcpair_t& s) const{
    //func_t smaller=(s.first<s.second)?(s.first):(s.second);
    //func_t bigger=(s.first<s.second)?(s.second):(s.first);
    return tr1::hash<func_t>()(totalFuncs*s.first + s.second);
  }
};

/*
   struct funcpair_t_eq{
   bool operator()(const funcpair_t &s1, const funcpair_t &s2) const{
   return (s1.first == s2.first) && (s1.second == s2.second);
   }
   };
   */

typedef tr1::unordered_map <const funcpair_t, uint64_t, funcpair_hash, funcpair_eq > JointFreqMap;
//typedef tr1::unordered_map <const funcpair_t, uint32_t **, funcpair_hash, funcpair_eq > JointFreqRangeMap;

struct SingleUpdateEntry{
  func_t func;
  wsize_t min_wsize;
  SingleUpdateEntry(func_t a, wsize_t b): func(a), min_wsize(b){}
};

struct JointUpdateEntry{
  funcpair_t func_pair;
  wsize_t min_wsize;
  JointUpdateEntry(funcpair_t a, wsize_t b):func_pair(a), min_wsize(b){}
};


struct disjointSet {
  static disjointSet ** sets;
  deque<func_t> elements;
  size_t size(){ return elements.size();}
  static void mergeSets(disjointSet *, disjointSet *);
  static void mergeSets(func_t id1, func_t id2){
    if(sets[id1]!=sets[id2])
      mergeSets(sets[id1],sets[id2]);
  }

  static void init_new_set(func_t id){
    sets[id]= new disjointSet();
    sets[id]->elements.push_back(id);
  }

};

disjointSet ** disjointSet::sets = 0;


struct SampledWindow{
  wsize_t wsize;
  list<func_t> partial_trace_list;
  list<SingleUpdateEntry> single_update_list;
  list<JointUpdateEntry> joint_update_list;

  SampledWindow():wsize(0){}

 	void add_single_update_entry(SingleUpdateEntry &sue){
    single_update_list.push_back(sue);
  }

  void add_joint_update_entry(JointUpdateEntry &jue){
    joint_update_list.push_back(jue);
  }
};

struct Prefetcher{
	func_t savee;
	func_t savior;
	uint32_t misscount;
	uint32_t savecount;

	Prefetcher(func_t a, func_t b, uint32_t c, uint32_t d): savee(a), savior(b), misscount(c), savecount(d){}
	static bool prefetcher_cmp(const Prefetcher &p1, const Prefetcher &p2) {
		assert((p1.savee == p2.savee) || (p1.savior == p2.savior));
		if(p1.savecount > p2.savecount)
			return true;

		if(p1.savecount < p2.savecount)
			return false;
	
		if(p1.savior < p2.savior)
			return true;
		
		if(p1.savior > p2.savior)
			return false;

		if(p1.savee < p2.savee)
			return true;
		else
			return false;
	}

	static bool is_significant(const Prefetcher &p){
		return (p.savecount*5 > p.misscount);
	}
};

//void print_trace(list<SampledWindow> *);
void initialize_affinity_data(float,func_t,func_t,func_t);
void sequential_update_affinity(list<func_t>::iterator);
void affinityAtExitHandler();


bool (*affEntryCmp)(const funcpair_t& pair_left, const funcpair_t& pair_right);
//bool affEntry1DCmp(const funcpair_t& pair_left, const funcpair_t& pair_right);
bool affEntry2DCmp(const funcpair_t& pair_left, const funcpair_t& pair_right);
void print_trace();
#endif /* AFFINITY_HPP */

