#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>

template <typename FuncReturnType, typename... Args>
class Event {
public:
    using FuncType = std::function<FuncReturnType(Args...)>;

    Event& Attach(FuncType func) {
        this->m_Listeners.push_back(func);
        return *this;
    }

    std::vector<FuncReturnType> Invoke(Args... args) {
        std::vector<FuncReturnType> results;
        for (const auto& listener : this->m_Listeners) {
            results.push_back(listener(args...));
        }

        return results;
    }

    void DetachAll() {
        this->m_Listeners.clear();
    }

private:
    std::vector<FuncType> m_Listeners;
};

// Specialization for void return type
template <typename... Args>
class Event<void, Args...> {
public:
    using FuncType = std::function<void(Args...)>;

    void Attach(FuncType func) {
        this->m_Listeners.push_back(func);
    }

    void Invoke(Args... args) {
        for (const auto& func : this->m_Listeners) {
            func(args...);
        }
    }

    void DetachAll() {
        this->m_Listeners.clear();
    }

private:
    std::vector<FuncType> m_Listeners;
};