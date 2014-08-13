
#include <chrono>
#include <functional>
#include <memory>
#include <stdlib.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/random/mersenne_twister.hpp>
#include <boost/thread/tss.hpp>

#include "folly/stats/Histogram.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "rocksdb/db.h"

namespace {

void generateRandomData(std::string& data, uint32_t size) {
  static boost::thread_specific_ptr<boost::random::mt19937_64> randomGenerator;
  if (nullptr == randomGenerator.get()) {
    randomGenerator.reset(new boost::random::mt19937_64());
  }
  data.resize(size);
  auto end = data.end();
  for (auto it = data.begin(); it != end; ++it) {
    *it = (*randomGenerator)() % 255 + 1;
  }
}

DEFINE_int32(
  threads,
  1,
  "Number of threads"
);

} // namespace

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::SetCommandLineOption("logtostderr", "true");
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  rocksdb::DB* db = nullptr;
  std::unique_ptr<rocksdb::DB> dbGuard;
  rocksdb::Options options;
  options.create_if_missing = true;
  options.compression = rocksdb::kSnappyCompression;
  options.max_open_files = 1000;
  std::string dbPath;
  dbPath.append(getenv("HOME"));
  dbPath.append("/tmp/test_rocksdb");
  rocksdb::Status status = rocksdb::DB::Open(options, dbPath, &db);
  dbGuard.reset(db);
  CHECK(status.ok()) << "Failed to open database: " << status.ToString();
  CHECK(nullptr != db) << "Null db pointer";
  uint32_t numberOfKeys = 3000000;
  uint32_t numberOfThreads = FLAGS_threads;
  std::vector<std::thread> threads;
  std::vector<folly::Histogram<int64_t>> histograms;
  uint32_t step = numberOfKeys / numberOfThreads;
  histograms.resize(
    numberOfThreads,
    folly::Histogram<int64_t>(5, 0, 10000000)
  );
  for (uint32_t threadIndex = 0; threadIndex < numberOfThreads; ++threadIndex) {
    uint32_t startIndex = threadIndex * step;
    uint32_t length = step;
    if (threadIndex == numberOfThreads - 1) {
      length += numberOfKeys % numberOfThreads;
    }
    auto& histogram = histograms[threadIndex];
    threads.push_back(
      std::move(
        std::thread(
          [startIndex, length, db, threadIndex, &histogram] () mutable {
            LOG(INFO)
              << "tid #" << threadIndex << " "
              << "s: " << startIndex << " "
              << "l: " << length;
            for (uint32_t i = startIndex; i < startIndex + length; ++i) {
              std::string randomData;
              generateRandomData(randomData, 10000);
              auto t0 = std::chrono::high_resolution_clock::now();
              rocksdb::Status status = db->Put(
                rocksdb::WriteOptions(),
                std::to_string(i),
                randomData
              );
              auto t1 = std::chrono::high_resolution_clock::now();
              auto durationInMicroseconds =
                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
              histogram.addValue(durationInMicroseconds.count());
              CHECK(status.ok()) << status.ToString();
            }
          }
        )
      )
    );
  }
  for (uint32_t i = 0; i < threads.size(); ++i) {
    threads[i].join();
    LOG(INFO) << "tid #" << i << " finished";
  }
  db = nullptr;
  dbGuard.reset(nullptr);
  folly::Histogram<int64_t> histogram(5, 0, 10000000);
  while (!histograms.empty()) {
    histogram.merge(histograms.back());
    histograms.pop_back();
  }
  LOG(INFO) << "50: " << histogram.getPercentileEstimate(0.50);
  LOG(INFO) << "75: " << histogram.getPercentileEstimate(0.75);
  LOG(INFO) << "90: " << histogram.getPercentileEstimate(0.90);
  LOG(INFO) << "95: " << histogram.getPercentileEstimate(0.95);
  LOG(INFO) << "99: " << histogram.getPercentileEstimate(0.99);
  LOG(INFO) << "99.9: "       << histogram.getPercentileEstimate(0.999);
  LOG(INFO) << "99.999: "     << histogram.getPercentileEstimate(0.99999);
  LOG(INFO) << "99.99999: "   << histogram.getPercentileEstimate(0.9999999);
  LOG(INFO) << "99.9999999: " << histogram.getPercentileEstimate(0.999999999);
  return 0;
}
