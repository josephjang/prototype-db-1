#include <benchmark/benchmark.h>

#include "storage_engine.h"
#include "libcuckoo/cuckoohash_map.hh"
#include "absl/container/flat_hash_map.h"

namespace protodb1 {

class StorageEngineBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const benchmark::State& state) {
  }

  void TearDown(const benchmark::State& state) {}

  void SetUpKeys() {
    keys_.resize(num_keys_);
    for (int i = 0; i < num_keys_; i++) {
      keys_[i] = i;
    }
  }

  size_t num_keys_ = 100000;
  std::vector<std::string> keys_;
};

BENCHMARK_DEFINE_F(StorageEngineBenchmark, StdUnorderedMapSet)(benchmark::State& state) {
  std::unordered_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
  }
  

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }

  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, StdUnorderedMapSet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, StdUnorderedMapGet)(benchmark::State& state) {
  std::unordered_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      auto it = map.find(keys_[i]);
      if (it != map.end()) {
        *it;
      }
    }
  }

  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, StdUnorderedMapGet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, CuckoohashMapSet)(benchmark::State& state) {
  libcuckoo::cuckoohash_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }
  
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, CuckoohashMapSet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, CuckoohashMapSetWithAbseilHash)(benchmark::State& state) {
  libcuckoo::cuckoohash_map<std::string, std::string, absl::container_internal::hash_default_hash<std::string>> map;
  if (state.thread_index == 0) {
    SetUpKeys();
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }
  
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, CuckoohashMapSetWithAbseilHash)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, CuckoohashMapGet)(benchmark::State& state) {
  libcuckoo::cuckoohash_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      map.find(keys_[i]);
    }
  }
  
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, CuckoohashMapGet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, AbseilFlatHashMapSet)(benchmark::State& state) {
  absl::flat_hash_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }
  
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, AbseilFlatHashMapSet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, AbseilFlatHashMapGet)(benchmark::State& state) {
  absl::flat_hash_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
    for (int i = 0; i < 100000; i++) {
      map.insert_or_assign(keys_[i], keys_[i]);
    }
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      auto it = map.find(keys_[i]);
      if (it != map.end()) {
        *it;
      }
    }
  }
  
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, AbseilFlatHashMapGet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, Protodb1StorageEngineSet)(benchmark::State& state) {
  protodb1::StorageEngine storage_engine;
  if (state.thread_index == 0) {
    SetUpKeys();
  }

  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      storage_engine.Set(keys_[i], keys_[i]);
    }
  }

  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, Protodb1StorageEngineSet)->UseRealTime()->Unit(benchmark::kMillisecond);


BENCHMARK_DEFINE_F(StorageEngineBenchmark, Protodb1StorageEngineGet)(benchmark::State& state) {
  protodb1::StorageEngine storage_engine;
  if (state.thread_index == 0) {
    SetUpKeys();
    for (int i = 0; i < 100000; i++) {
      storage_engine.Set(keys_[i], keys_[i]);
    }
  }
  
  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      storage_engine.Get(keys_[i]);
    }
  }

  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, Protodb1StorageEngineGet)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, Protodb1StorageEngineGetAndSet)
(benchmark::State& state) {
  protodb1::StorageEngine storage_engine;
  if (state.thread_index == 0) {
    SetUpKeys();
  }
  
  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      storage_engine.Set(keys_[i], keys_[i]);
    }
    for (int i = 0; i < 100000; i++) {
      storage_engine.Get(keys_[i]);
    }
  }
  
  state.SetItemsProcessed(state.iterations() * num_keys_ * 2);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, Protodb1StorageEngineGetAndSet)->UseRealTime()->Unit(benchmark::kMillisecond);

}  // namespace protodb1

BENCHMARK_MAIN();
