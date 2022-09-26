#ifndef COROUTINE_MYGENERATOR_H__
#define COROUTINE_MYGENERATOR_H__

#include <coroutine>
#include <assert.h>
#include <iterator>
using namespace std;
//编译加： -fcoroutines

template<class T>
struct MyGenerator {
    // inner types
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    MyGenerator(handle_type h) : handle(h) {}
    MyGenerator(const MyGenerator & s) = delete;
    MyGenerator & operator=(const MyGenerator &) = delete;
    MyGenerator(MyGenerator && s) : handle(s.handle) {s.handle = nullptr;}
    MyGenerator & operator=(MyGenerator && s) { 
        handle = s.handle; 
        s.handle = nullptr;
        return *this;
    }
    ~MyGenerator() {
        if(handle) {
             handle.destroy();
        }
    }
// ====================================================
    struct MyGenerator_Iterator : public iterator<input_iterator_tag, T>  {
        handle_type handle_;
        MyGenerator_Iterator(handle_type handle = nullptr) : handle_(handle) {}

        // MyGenerator_Iterator() == default;
        // MyGenerator_Iterator & operator=(const MyGenerator_Iterator &x) =default;
        // MyGenerator_Iterator(const MyGenerator_Iterator &) = default;

        bool operator==(const MyGenerator_Iterator & other) const {
            return (handle_ == other.handle_) || 
                   (handle_ == nullptr && other.handle_.done()) ||
                   (other.handle_ == nullptr && handle_.done());
        }

        bool operator!=(const MyGenerator_Iterator & other) const {
            return !(*this == other);
        }

        T & operator * () const {
            return handle_.promise().value;
            // 这样可以多次获得此数据，按理，改为move的才应该更合理。
        }

        // T * operator ->() const {
        //     return &handle_.promise().value;
        // }

        MyGenerator_Iterator & operator ++ () {
            if(!(handle_.done())) {
                handle_.resume();
            }
            return *this;
        }

        // MyGenerator_Iterator & operator ++ (int) = delete;
        // 生成器，是foward iterator，理论上只能遍历一次，后缀++返回的是已经无效的iterator
        // 所以，理论上这个接口不可以有，或者它应该返回void
        MyGenerator_Iterator operator ++ (int) {
            auto x(*this);
            ++*this;
            return x;
        }

    };

    typedef MyGenerator_Iterator iterator;
    typedef MyGenerator_Iterator const_iterator; // 要能和boost range库配合，这个必须有！！！

    MyGenerator_Iterator begin() const { return this->handle; }
    MyGenerator_Iterator end()   const { return MyGenerator_Iterator(); }

// =================================
    T get() const {
        if (!(this->handle.done())) {
            // 如果initial_suspend直接挂起，那第一次进来肯定没有done
            // 但是，resume后，可能没有数据，直接到final_suspend状态
            auto r = std::move(handle.promise().value); // 就需要2次判断，是否done
            handle.resume(); // resume                  // 所以，调整为 inital_suspend不挂起，直接停到yield处或者final_suspend
            return r;                                   // 这样，一次判断done就够了
        }                                               // 缺点就是永远预生成一次

        assert(!"boom!在done状态下继续访问协程了！");
        return {};                                      //  理论上不可以到这个return
    }

    struct promise_type {
        promise_type() {}
        ~promise_type() {}

        auto get_return_object() { return MyGenerator<T>{handle_type::from_promise(*this)};}

        auto initial_suspend() {
            return std::suspend_never{}; // dont suspend it
            // return std::suspend_always{};
        }
        // called when just before final_suspend, conflict with return_value
        auto return_void() {
            return std::suspend_never{}; // dont suspend it
            // return std::suspend_always{};
        } 

        template<typename TT>
        auto yield_value(TT && t) //called by co_yiled(),可以认为也是std::forward完美转发进来的
        {
            value = std::forward<TT> (t);
            return std::suspend_always{};
        }

        auto final_suspend() noexcept(true) { // called at the end of coroutine body
            // 如果 final_suspend 真的挂起了协程，
            // 那么作为协程的调用者，你需要手动的调用 destroy 来释放协程；
            // 如果 final_suspend 没有挂起协程，那么协程将自动销毁
            // 但是，请注意协程保存的value可能会失效

            // return std::suspend_never{};
            return std::suspend_always{};
        }
        
        auto unhandled_exception() { // exception handler
            std::exit(1);
        } 

        // T wait_transform() {}
        T value;

    };

    handle_type handle;
};

#endif
