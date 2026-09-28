#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <memory>
#include <stdexcept>

template <typename T> struct node {
    std::shared_ptr<T> data;
    node *next;
    node(const T &value) : data(std::make_shared<T>(value)), next(nullptr) {}
};

class hazard_pointer_domain {
    static constexpr std::size_t max_hazards = 100;

    struct alignas(64) hazard_pointer {
        std::atomic<bool> active{false};
        std::atomic<void *> pointer{nullptr};
    };

    hazard_pointer_domain() = default;

  public:
    hazard_pointer_domain(const hazard_pointer_domain &) = delete;
    hazard_pointer_domain &operator=(const hazard_pointer_domain &) = delete;
    class owner {
      public:
        owner()
            : domain(hazard_pointer_domain::instance()),
              hazard(domain.acquire()) {}

        ~owner() { domain.release(hazard); }

        owner(owner const &) = delete;
        owner &operator=(owner const &) = delete;

        std::atomic<void *> &pointer() { return hazard->pointer; }

      private:
        hazard_pointer_domain &domain;
        hazard_pointer *hazard;
    };

    static hazard_pointer_domain &instance() {
        static hazard_pointer_domain domain;
        return domain;
    }

    bool protects(void *pointer) const {
        for (auto const &hazard : hazards) {
            if (hazard.pointer.load() == pointer)
                return true;
        }
        return false;
    }

  private:
    std::array<hazard_pointer, max_hazards> hazards{};

    hazard_pointer *acquire() {
        for (auto &hazard : hazards) {
            bool expected = false;
            if (hazard.active.compare_exchange_strong(expected, true))
                return &hazard;
        }
        throw std::runtime_error("no hazard pointers available");
    }

    void release(hazard_pointer *hazard) {
        hazard->pointer.store(nullptr);
        hazard->active.store(false);
    }
};

inline std::atomic<void *> &get_hazard_pointer_for_current_thread() {
    static thread_local hazard_pointer_domain::owner hazard;
    return hazard.pointer();
}

inline bool outstanding_hazard_pointers_for(void *pointer) {
    return hazard_pointer_domain::instance().protects(pointer);
}

struct retired_node {
    void *ptr;
    void (*deleter)(void *);
    retired_node *next;
    void release() { deleter(ptr); }
};

template <typename T> void delete_node_func(void *p) {
    delete static_cast<node<T> *>(p);
}

inline std::atomic<retired_node *> &retired_nodes_head() {
    static std::atomic<retired_node *> head{nullptr};
    return head;
}

inline void add_to_retired_nodes(retired_node *node) {
    auto &head = retired_nodes_head();
    node->next = head.load();

    while (!head.compare_exchange_weak(node->next, node))
        ;
}

template <typename T> void reclaim_later(node<T> *p) {
    add_to_retired_nodes(new retired_node{p, &delete_node_func<T>, nullptr});
}

inline void delete_nodes_with_no_hazards() {
    retired_node *current = retired_nodes_head().exchange(nullptr);

    while (current) {
        retired_node *next = current->next;

        if (!outstanding_hazard_pointers_for(current->ptr)) {
            current->release();
            delete current;
        } else {
            add_to_retired_nodes(current);
        }

        current = next;
    }
}

template <typename T> class lock_free_stack {
    std::atomic<node<T> *> head{nullptr};

  public:
    void push(T const &data) {
        node<T> *new_node = new node<T>(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node))
            ;
    }

    std::shared_ptr<T> pop() {
        std::atomic<void *> &hp = get_hazard_pointer_for_current_thread();
        node<T> *old_head = head.load();
        do {
            node<T> *temp;
            do {
                temp = old_head;
                hp.store(old_head);
                old_head = head.load();
            } while (old_head != temp);
        } while (old_head &&
                 !head.compare_exchange_strong(old_head, old_head->next));

        hp.store(nullptr);
        std::shared_ptr<T> res;
        if (old_head) {
            res.swap(old_head->data);
            if (outstanding_hazard_pointers_for(old_head)) {
                reclaim_later(old_head);
            } else {
                delete old_head;
            }
        }
        delete_nodes_with_no_hazards();
        return res;
    }

    ~lock_free_stack() {
        node<T> *current = head.exchange(nullptr);

        while (current) {
            node<T> *next = current->next;
            delete current;
            current = next;
        }

        delete_nodes_with_no_hazards();
    }
};