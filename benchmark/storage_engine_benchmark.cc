#include <benchmark/benchmark.h>

#include "storage_engine.h"

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

BENCHMARK_DEFINE_F(StorageEngineBenchmark, StdUnorderedMapGet)(benchmark::State& state) {
  std::unordered_map<std::string, std::string> map;
  if (state.thread_index == 0) {
    SetUpKeys();
  }
  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      map.find(keys_[i]);
    }
  }
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, StdUnorderedMapGet)->Threads(12);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, BM_Get)(benchmark::State& state) {
  protodb1::StorageEngine storage_engine;
  if (state.thread_index == 0) {
    SetUpKeys();
  }
  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      storage_engine.Get(keys_[i]);
    }
  }
  state.SetItemsProcessed(state.iterations() * num_keys_);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, BM_Get)->Threads(12);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, BM_Set)(benchmark::State& state) {
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
BENCHMARK_REGISTER_F(StorageEngineBenchmark, BM_Set)->Threads(12);

BENCHMARK_DEFINE_F(StorageEngineBenchmark, BM_GetAndSet)
(benchmark::State& state) {
  protodb1::StorageEngine storage_engine;
  if (state.thread_index == 0) {
    SetUpKeys();
  }
  for (auto _ : state) {
    for (int i = 0; i < 100000; i++) {
      storage_engine.Get(keys_[i]);
    }
    for (int i = 0; i < 100000; i++) {
      storage_engine.Set(keys_[i], keys_[i]);
    }
  }
  state.SetItemsProcessed(state.iterations() * num_keys_ * 2);
}
BENCHMARK_REGISTER_F(StorageEngineBenchmark, BM_GetAndSet)->Threads(12);

}  // namespace protodb1

BENCHMARK_MAIN();
