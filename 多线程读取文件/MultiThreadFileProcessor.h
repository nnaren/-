
// #include <set>
#include <deque>
#include <string>
#include <functional>
#include <fstream>

#include <iostream>

#include <boost/filesystem/path.hpp>


#include "MultiThreadController.h"

#include "FindFiles.h"
#include "ReadFileUtils.h"

using namespace std;
namespace fs = std::filesystem;
namespace boostfs = boost::filesystem;

template<typename Product>
struct ProductController {
    deque<Product>      products;

    int                 readedNum = 0;
    int                 processedNum = 0;
    mutex               productMutex;
    mutex               finalMutex;

    condition_variable  goProduce;
    condition_variable  goConSume;

    atomic<bool>        produceStopped = false;
};

namespace MT_FP 
{
    template<typename Product>
    void ReadFile(const fs::path & path,
                  const MultiThread_Controller & threadController,
                  ProductController<Product> & productController) {
        // cout << "readFile"  << path.string() << endl;
        string fileName = path.string();
        auto txt = ReadFileFast(fileName);

        unique_lock<mutex> lock(productController.productMutex);
        // productController.goProduce.wait(lock, [&threadController, &productController] { return productController.products.size() < threadController.maxWaitNum; });
        while(!productController.goProduce.wait_for(lock, 10ms, 
                    [&threadController, &productController]
                    {
                        return productController.products.size() < threadController.maxWaitNum;  // 最大生产个数
                    })) {
            if (threadController.IsUserStopped()) {  // 优雅的退出
                return;
            }
        }

        productController.products.emplace_back(std::move(fileName), std::move(txt));
        productController.readedNum ++;
        productController.goConSume.notify_one();
    }
    // typedef pair<string,ifstream> SingleFileText;
    template<typename InputPathListContainer, typename SingleFileText>
    void ReadFiles(const InputPathListContainer & paths, const PathFilter & pathFilter,
                       const MultiThread_Controller & threadController,
                       ProductController<SingleFileText> & productController) {  // SingleFileText
        for (auto & path : paths) {
            // cout << "read files: " << path.string() << endl;
            if (threadController.IsUserStopped()) {
                return;
            }

            if ( fs::is_directory(path) && pathFilter.first(path)) {
                for (fs::recursive_directory_iterator dir_itr(path); dir_itr != fs::recursive_directory_iterator(); ++dir_itr ) {
                    if(threadController.IsUserStopped()) {
                        return;
                    }
                    if(fs::is_directory(dir_itr->status()) && !pathFilter.first(dir_itr->path())) {
                        dir_itr.disable_recursion_pending();
                    } else if (fs::is_regular_file(dir_itr->status()) && pathFilter.second(dir_itr->path())) {
                        ReadFile(dir_itr->path(), threadController, productController);
                    }
                }
            } else if (fs::is_regular_file(path) && pathFilter.second(path)) {
                ReadFile(path, threadController, productController);
            }
        }

    }

    template<typename Product>
    void ConsumeProducts(const MultiThread_Controller & threadController, 
                         ProductController<Product> & productController,
                         const std::function<bool (Product *, mutex &)> & TextProcessor) {
        Product product;
        while (!threadController.IsUserStopped()) {
            bool isGot = false;
            bool isFinished = false;
            // cout << "等待结束" << std::endl;
            // cout << "empty: " << productController.products.empty() << endl;
            // cout << "produceStopped: " << productController.produceStopped << endl;
            {
                unique_lock<mutex> lock(productController.productMutex);
                // ProductController.goConSume.wait(lock, [&productController] {return productController.produceStopped || !productController.products.empty();});
                if (productController.goConSume.wait_for(lock, 10ms, 
                    [&productController] { return productController.produceStopped || !productController.products.empty(); })) {
                    
                    if (!productController.products.empty()) {
                        product.swap(productController.products.front());
                        isGot = true;
                        productController.products.pop_front();
                        productController.processedNum++;
                        productController.goProduce.notify_one();
                    }
                    isFinished = productController.produceStopped && productController.products.empty();
                }
            }

            if(isGot) {
                TextProcessor(&product, productController.finalMutex);
                threadController.UpdateProcess(1, "processing");

            }
            
            if(isFinished) {
                TextProcessor(nullptr, productController.finalMutex);
                break;
            }
        }
    }
    template<typename Product>
    void JoinThreads(const auto & threadController,
                     const auto & paths, const string & dirFilterPattern, const string & fileFilterPattern,
                     const auto & ProductProducer, const auto & ProductProcessor)
    {
        ProductController<Product> productController;
        // cout << "创建线程： product" << endl;
        auto productThread = thread([&threadController, &productController,
                                     &paths, pathFilter = CreatePathFilter(dirFilterPattern, fileFilterPattern),
                                     &ProductProducer]() {
            ProductProducer(paths, pathFilter, threadController, productController);
            // cout << "生产完成！" << endl;
        });
        auto consumeFunctor = [ &threadController, &productController, &ProductProcessor ] () {
                                    ConsumeProducts(threadController, productController, ProductProcessor);
                              };
        vector<thread> consumerThreads;
        // cout << "创建线程： consumer: " << threadController.consumerCount << endl;
        for(size_t i = 0; i < threadController.consumerCount; ++i) {
            consumerThreads.emplace_back(consumeFunctor);
        }

        threadController.ConfigProcess(0, 0);

        productThread.join();
        // 生产者已经结束了 通知所有消费者 stopped
        productController.produceStopped = true;
        productController.goConSume.notify_all();

        for (auto & f: consumerThreads) { f.join(); }

        threadController.Finish(productController.readedNum, productController.processedNum);
    }
}

template<typename PathContainer, typename Product/* = SingleFileText */ >
    void MultiThreadFileProcessor(MultiThread_Controller threadController,
                                 PathContainer paths, string dirFilterPattern /* = "-.git;-.svn" */, string fileFilterPattern /* = "*.*" */,
                                 std::function<bool (Product *, mutex &)> ProductProcessor,
                                 std::function<void (const PathContainer &, const PathFilter &, const MultiThread_Controller &,
                                                     ProductController<Product> &)> ProductProducer = MT_FP::ReadFiles<PathContainer, Product>
                                 ) {
    // 是否阻塞调用方
    if (threadController.blockCaller) {
        cout << "阻塞调用方" << endl;
        MT_FP::JoinThreads<Product>(threadController, 
                                    paths, dirFilterPattern, fileFilterPattern,
                                    ProductProducer, ProductProcessor);
        
    } else {
        cout << "不阻塞" << endl;
        thread([threadController = std::move(threadController),
                paths = std::move(paths), dirFilterPattern = std::move(dirFilterPattern), fileFilterPattern = std::move(fileFilterPattern),
                ProductProducer = std::move(ProductProducer), ProductProcessor = std::move(ProductProcessor)] {
                    MT_FP::JoinThreads<Product>(threadController, paths, dirFilterPattern, fileFilterPattern, ProductProducer, ProductProcessor);
                }
              ).detach();
    }
}