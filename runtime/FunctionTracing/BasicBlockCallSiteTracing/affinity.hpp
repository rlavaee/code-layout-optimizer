//#include <sparsehash/sparse_hash_map>
//#include <sparsehash/sparse_hash_set>

#ifndef AFFINITY_HPP
#define AFFINITY_HPP
#include <deque>
#include <list>
#include <functional>
#include <unordered_map>
#include <stdint.h>
#include <set>
#include <cstring>

#define MAX_FILE_NAME 30
using namespace std;

const char * version_str = ".bbabc";

typedef uint16_t wsize_t;
typedef uint16_t func_t;
typedef uint16_t bb_t;
func_t totalFuncs;
bb_t * bb_count;

typedef pair<bb_t,bb_t> bb_pair_t;

class Record{
  func_t fid;
  bb_t bbid;
  public:
  Record(){};
  Record(func_t a, bb_t b): fid(a),bbid(b) {};
  Record(const Record &rec): fid(rec.getFuncId()), bbid(rec.getBBId()){};

  func_t getFuncId() const{
    return fid;
  }

  bb_t getBBId() const{
    return bbid;
  }

  bool operator < (const Record &rhs) const{
    if(getFuncId() < rhs.getFuncId())
      return true;
    if(getFuncId() > rhs.getFuncId())
      return false;
    if(getBBId() < rhs.getBBId())
      return true;
    return false;
  }

  bool operator == (const Record &rhs) const{
    return getFuncId()==rhs.getFuncId() && getBBId()==rhs.getBBId();
  }

  bool operator != (const Record &rhs) const {return !(*this==rhs);}

};

typedef pair<Record,Record> RecordPair;

bool haveSameFunctions(const RecordPair &rec_pair){
  return rec_pair.first.getFuncId() == rec_pair.second.getFuncId();
}

struct RecordHash{
  size_t operator()(const Record& rec) const{
    return hash<uint32_t>()( ((uint32_t)rec.getFuncId() *totalFuncs) + rec.getBBId());
  }
};

RecordPair unordered_RecordPair(Record s1,Record s2){
  return (s1<s2)?(RecordPair(s1,s2)):(RecordPair(s2,s1));
}


struct RecordPair_hash{
  size_t operator()(const RecordPair& rec_pair) const{
    return hash<uint32_t>()( ((RecordHash()(rec_pair.first)) << 16) + RecordHash()(rec_pair.second));
  }
};


struct bb_pair_hash{
  size_t operator()(const bb_pair_t &bbp) const{
    return hash<uint32_t>()( ((uint32_t)bbp.first << 16) + bbp.second);
  }
};

typedef std::unordered_map <const RecordPair, uint32_t *, RecordPair_hash > JointFreqMap;
typedef std::unordered_map <const Record, uint32_t *, RecordHash> SingleFreqMap;
typedef std::unordered_map <const bb_pair_t, uint32_t , bb_pair_hash> FallThroughMap;


struct disjointSet {
  static std::unordered_map<const Record, disjointSet *,RecordHash> sets;
  deque<Record> elements;
  size_t size(){ return elements.size();}

  static void init_new_set(const Record &rec){
    sets[rec]= new disjointSet();
    sets[rec]->elements.push_back(rec);
  }

  static bool is_connected_to_right(const Record &rec){
    //assert(sets[rec] && "Has not been initialized!\n");
    return sets[rec]->elements.back()!=rec;
  }

  static bool is_connected_to_left(const Record &rec){
    //assert(sets[rec] && "Has not been initialized!\n");
    return sets[rec]->elements.front()!=rec;
  }

  static void mergeBasicBlocksSameFunction(func_t fid, const bb_pair_t &bb_pair);
  static void mergeSets(Record const &left_rec, Record const &right_rec);

};

std::unordered_map<const Record, disjointSet *,RecordHash> disjointSet::sets = std::unordered_map<const Record, disjointSet *,RecordHash>();

struct SampledWindow{
  wsize_t wsize;
  list<Record> partial_trace_list;
  set<Record> owners;

  SampledWindow() :wsize(0){};
  ~SampledWindow(){};
  void erase(list<Record>::iterator trace_iter){
    partial_trace_list.erase(trace_iter);
    wsize--;
  };

  void push_front(Record rec){
    partial_trace_list.push_front(rec);
    wsize++;
  }

  size_t size(){
    return wsize;
  } 
};



void print_trace(list<SampledWindow> *);
void initialize_affinity_data();
void sequential_update_affinity(const Record &rec, list<SampledWindow>::iterator, bool missed);
void affinityAtExitHandler();


bool jointFreqSameFunctionsCmp(const RecordPair& pair_left, const RecordPair& pair_right);
bool jointFreqCountCmp(const RecordPair& pair_left, const RecordPair& pair_right);

bool fall_through_cmp (const bb_pair_t &left_pair, const bb_pair_t &right_pair);

char * get_versioned_filename(const char * basename){
  char * versioned_name = new char [MAX_FILE_NAME];
  strcpy(versioned_name,basename);
  strcat(versioned_name,version_str);
  return versioned_name;
}

#endif /* AFFINITY_HPP */
