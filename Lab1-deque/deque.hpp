#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
namespace sjtu { 
const size_t chunk_size = 512;
template<class T>
class deque {
public:
    class map_node;
private:
    //deque 的参数：头（虚节点），尾（虚节点）和当前数据的个数
    map_node* head;
    map_node* tail;
    size_t map_size;
public:
    class chunk_node {
    public:
        //chunk_node 是一个 block（也叫chunk）上的结点的类型
        chunk_node* prev;
        chunk_node* next;
        T* data;
        chunk_node():prev(nullptr), next(nullptr), data(nullptr) {}
    };
    class map_node {
    public:
        //map_node 记录其指向的 chunk 的第一个结点和最后一个结点（虚节点）
        map_node* prev;
        map_node* next;
        chunk_node* fir_node;
        chunk_node* las_node;
        //chunk 的长度
        size_t length;
        //chunk 的 index（第几个chunk）
        size_t index;
        map_node():prev(nullptr), next(nullptr), fir_node(nullptr), las_node(nullptr), length(0), index(0) {}
    };
    class const_iterator;
    class iterator {
    private:
        //指向 iterator 所在 deque 的指针
        deque<T> *deq;
        //当前 iterator 指向的 chunk_node（存放了data）
        chunk_node* cur;
        //cur 在这个 chunk 上的 index
        size_t cur_ind;
        //指向这个 chunk 所在 map_node
        map_node* node;
    public:
        iterator():deq(nullptr), cur(nullptr), cur_ind(0), node(nullptr) {}
        iterator(deque<T> *host_deq, chunk_node* current,
                 size_t ind, map_node* cur_node):
        deq(host_deq), cur(current), cur_ind(ind), node(cur_node) {}
        iterator(const iterator &other):
        deq(other.deq), cur(other.cur), cur_ind(other.cur_ind), node(other.node) {}
        iterator(const const_iterator &other):
        deq(other.deq), cur(other.cur), cur_ind(other.cur_ind), node(other.node) {}
        /**
         * return a new iterator which pointer n-next elements
         * even if there are not enough elements, the behaviour is **undefined**.
         * as well as operator-
         * however if the iterator exceed only before begin(), or after end(),
         * the program should still run **without causing an error**
         * notice that n can be negative!!!
         */
        iterator operator+(const int &n) const {
            if (n > 0) {
                int n_tmp = n;
                iterator tmp(deq, cur, cur_ind, node);
                while (tmp.node->next != deq->tail && tmp.cur_ind + n_tmp >= tmp.node->length) {
                    n_tmp -= tmp.node->length - tmp.cur_ind;
                    tmp.node = tmp.node->next;
                    tmp.cur = tmp.node->fir_node;
                    tmp.cur_ind = 0;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->next;
                    tmp.cur_ind++;
                }
                return tmp;
            } else if (n < 0) {
            int n_tmp = -n;
                iterator tmp(deq, cur, cur_ind, node);
                while (tmp.cur_ind < n_tmp && tmp.node->prev != tmp.deq->head) {
                    n_tmp -= tmp.cur_ind + 1;
                    tmp.node = tmp.node->prev;
                    tmp.cur = tmp.node->las_node;
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind = tmp.node->length - 1;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind--;
                }
                return tmp;
            } else {
                return *this;
            }
        }
        iterator operator-(const int &n) const {
            if (n < 0) {
                int n_tmp = -n;
                iterator tmp(deq, cur, cur_ind, node);
                while (tmp.node->next != deq->tail && tmp.cur_ind + n_tmp >= tmp.node->length) {
                    n_tmp -= tmp.node->length - tmp.cur_ind;
                    tmp.node = tmp.node->next;
                    tmp.cur = tmp.node->fir_node;
                    tmp.cur_ind = 0;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->next;
                    tmp.cur_ind++;
                }
                return tmp;
            } else if (n > 0) {
            int n_tmp = n;
                iterator tmp(deq, cur, cur_ind, node);
                while (tmp.cur_ind < n_tmp && tmp.node->prev != tmp.deq->head) {
                    n_tmp -= tmp.cur_ind + 1;
                    tmp.node = tmp.node->prev;
                    tmp.cur = tmp.node->las_node;
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind = tmp.node->length - 1;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind--;
                }
                return tmp;
            } else {
                return *this;
            }
        }
        /**
         *  return the signed distance between two iterator,
         *  if these two iterators points to different vectors, throw invaild_iterator.
         *  notice size_t is a very dangerous type which should mainly used by comparison
         */
        int operator-(const iterator &rhs) const {
            if (deq != rhs.deq) throw invalid_iterator();
            int tmp = 0;
            iterator tmp_it = *this;
            if (node->index > rhs.node->index) {
                map_node* tmp_node = rhs.node;
                tmp += rhs.node->length - rhs.cur_ind;
                tmp_node = tmp_node->next;
                while (tmp_node->index < node->index) {
                    tmp += tmp_node->length;
                    tmp_node = tmp_node->next;
                }
                tmp += cur_ind;
                //you can't write: (node->index - rhs.node->index < 0)
            } else if (node->index < rhs.node->index) {
                map_node* tmp_node = node;
                tmp -= int(node->length - cur_ind);
                tmp_node = tmp_node->next;
                while (tmp_node->index < rhs.node->index) {
                    tmp -= int(tmp_node->length);
                    tmp_node = tmp_node->next;
                }
                tmp -= int(rhs.cur_ind);
            } else {
                tmp = int(cur_ind) - int(rhs.cur_ind);
            }
            return tmp;
        }
        iterator& operator+=(const int &n) {
            if (n > 0) {
                int n_tmp = n;
                while (node->next != deq->tail && cur_ind + n_tmp >= node->length) {
                    n_tmp -= node->length - cur_ind;
                    node = node->next;
                    cur = node->fir_node;
                    cur_ind = 0;
                }
                while (n_tmp--) {
                    cur = cur->next;
                    cur_ind++;
                }
            } else if (n < 0) {
                int n_tmp = -n;
                while (cur_ind < n_tmp && node->prev != deq->head) {
                    n_tmp -= cur_ind + 1;
                    node = node->prev;
                    cur = node->las_node;
                    cur = cur->prev;
                    cur_ind = node->length - 1;
                }
                while (n_tmp--) {
                    cur = cur->prev;
                    cur_ind--;
                }
            }
            return *this;
        }
        iterator& operator-=(const int &n) {
            if (n < 0) {
                int n_tmp = -n;
                while (node->next != deq->tail && cur_ind + n_tmp >= node->length) {
                    n_tmp -= node->length - cur_ind;
                    node = node->next;
                    cur = node->fir_node;
                    cur_ind = 0;
                }
                while (n_tmp--) {
                    cur = cur->next;
                    cur_ind++;
                }
            } else if (n > 0) {
                int n_tmp = n;
                while (cur_ind < n_tmp && node->prev != deq->head) {
                    n_tmp -= cur_ind + 1;
                    node = node->prev;
                    cur = node->las_node;
                    cur = cur->prev;
                    cur_ind = node->length - 1;
                }
                while (n_tmp--) {
                    cur = cur->prev;
                    cur_ind--;
                }
            }
            return *this;
        }
        /**
         * TODO iter++
         */
        iterator operator++(int) {
            iterator tmp(deq, cur, cur_ind, node);
            if (cur_ind + 1 < node->length || (cur_ind + 1 >= node->length && node->next == deq->tail)) {
                cur_ind++;
                cur = cur->next;
            } else {
                cur_ind = 0;
                node = node->next;
                cur = node->fir_node;
            }
            return tmp;
        }
        /**
         * TODO ++iter
         */
        iterator& operator++() {
            if (cur_ind + 1 < node->length || (cur_ind + 1 >= node->length && node->next == deq->tail)) {
                cur_ind++;
                cur = cur->next;
            } else {
                cur_ind = 0;
                node = node->next;
                cur = node->fir_node;
            }
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int) {
            iterator tmp(deq, cur, cur_ind, node);
            if (cur_ind == 0 && node->prev != deq->head) {
                node = node->prev;
                cur_ind = node->length - 1;
                cur = node->las_node->prev;
            } else {
                cur_ind--;
                cur = cur->prev;
            }
            return tmp;
        }
        /**
         * TODO --iter
         */
        iterator& operator--() {
            if (cur_ind == 0 && node->prev != deq->head) {
                node = node->prev;
                cur_ind = node->length - 1;
                cur = node->las_node->prev;
            } else {
                cur_ind--;
                cur = cur->prev;
            }
            return *this;
        }
        /**
         * TODO *it
         * throw invalid_iterator
         */
        T& operator*() const {
            if (cur == nullptr || cur->data == nullptr) throw invalid_iterator();
            return *cur->data;
        }
        /**
         * TODO it->field
         */
        T* operator->() const noexcept { return cur->data; }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {
            return (cur == rhs.cur);
        }
        bool operator==(const const_iterator &rhs) const {
            return (cur == rhs.cur);
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return (cur != rhs.cur);
        }
        bool operator!=(const const_iterator &rhs) const {
            return (cur != rhs.cur);
        }
    friend class deque<T>;
    friend class const_iterator;
    };
    class const_iterator {
        /** it should has similar member method as iterator.
         * and it should be able to construct from an iterator.
         */
    private:
        //指向常量的指针不能改变常量到地址中存放的数据，但是可以改变指向哪个常量
        const deque<T> *deq;
        chunk_node* cur;
        size_t cur_ind;
        map_node* node;        
    public:
        const_iterator():deq(nullptr), cur(nullptr), cur_ind(0), node(nullptr) {}
        const_iterator(const deque<T> *host_deq, chunk_node* current,
                       size_t ind, map_node* cur_node):
        deq(host_deq), cur(current), cur_ind(ind), node(cur_node) {}
        const_iterator(const const_iterator &other):
        deq(other.deq), cur(other.cur), cur_ind(other.cur_ind), node(other.node) {}
        const_iterator(const iterator &other):
        deq(other.deq), cur(other.cur), cur_ind(other.cur_ind), node(other.node) {}
        const_iterator operator+(const int &n) const {
            if (n > 0) {
                int n_tmp = n;
                const_iterator tmp(deq, cur, cur_ind, node);
                while (tmp.node->next != deq->tail && tmp.cur_ind + n_tmp >= tmp.node->length) {
                    n_tmp -= tmp.node->length - tmp.cur_ind;
                    tmp.node = tmp.node->next;
                    tmp.cur = tmp.node->fir_node;
                    tmp.cur_ind = 0;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->next;
                    tmp.cur_ind++;
                }
                return tmp;
            } else if (n < 0) {
            int n_tmp = -n;
                const_iterator tmp(deq, cur, cur_ind, node);
                while (tmp.cur_ind < n_tmp && tmp.node->prev != tmp.deq->head) {
                    n_tmp -= tmp.cur_ind + 1;
                    tmp.node = tmp.node->prev;
                    tmp.cur = tmp.node->las_node;
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind = tmp.node->length - 1;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind--;
                }
                return tmp;
            } else {
                return *this;
            }
        }
        const_iterator operator-(const int &n) const {
            if (n < 0) {
                int n_tmp = -n;
                const_iterator tmp(deq, cur, cur_ind, node);
                while (tmp.node->next != deq->tail && tmp.cur_ind + n_tmp >= tmp.node->length) {
                    n_tmp -= tmp.node->length - tmp.cur_ind;
                    tmp.node = tmp.node->next;
                    tmp.cur = tmp.node->fir_node;
                    tmp.cur_ind = 0;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->next;
                    tmp.cur_ind++;
                }
                return tmp;
            } else if (n > 0) {
            int n_tmp = n;
                const_iterator tmp(deq, cur, cur_ind, node);
                while (tmp.cur_ind < n_tmp && tmp.node->prev != tmp.deq->head) {
                    n_tmp -= tmp.cur_ind + 1;
                    tmp.node = tmp.node->prev;
                    tmp.cur = tmp.node->las_node;
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind = tmp.node->length - 1;
                }
                while (n_tmp--) {
                    tmp.cur = tmp.cur->prev;
                    tmp.cur_ind--;
                }
                return tmp;
            } else {
                return *this;
            }
        }
        // return the distance between two iterator,
        // if these two iterators points to different vectors, throw invaild_iterator.
        int operator-(const const_iterator &rhs) const {
            if (deq != rhs.deq) throw invalid_iterator();
            int tmp = 0;
            const_iterator tmp_it = *this;
            if (node->index > rhs.node->index) {
                map_node* tmp_node = rhs.node;
                tmp += rhs.node->length - rhs.cur_ind;
                tmp_node = tmp_node->next;
                while (tmp_node->index < node->index) {
                    tmp += tmp_node->length;
                    tmp_node = tmp_node->next;
                }
                tmp += cur_ind;
            } else if (node->index < rhs.node->index) {
                map_node* tmp_node = node;
                tmp -= node->length - cur_ind;
                tmp_node = tmp_node->next;
                while (tmp_node->index < rhs.node->index) {
                    tmp -= tmp_node->length;
                    tmp_node = tmp_node->next;
                }
                tmp -= rhs.cur_ind;
            } else {
                tmp = int(cur_ind) - int(rhs.cur_ind);
            }
            return tmp;
        }
        const_iterator& operator+=(const int &n) {
            if (n > 0) {
                int n_tmp = n;
                while (node->next != deq->tail && cur_ind + n_tmp >= node->length) {
                    n_tmp -= node->length - cur_ind;
                    node = node->next;
                    cur = node->fir_node;
                    cur_ind = 0;
                }
                while (n_tmp--) {
                    cur = cur->next;
                    cur_ind++;
                }
            } else if (n < 0) {
                int n_tmp = -n;
                while (cur_ind < n_tmp && node->prev != deq->head) {
                    n_tmp -= cur_ind + 1;
                    node = node->prev;
                    cur = node->las_node;
                    cur = cur->prev;
                    cur_ind = node->length - 1;
                }
                while (n_tmp--) {
                    cur = cur->prev;
                    cur_ind--;
                }
            }
            return *this;
        }
        const_iterator& operator-=(const int &n) {
            if (n < 0) {
                int n_tmp = -n;
                while (node->next != deq->tail && cur_ind + n_tmp >= node->length) {
                    n_tmp -= node->length - cur_ind;
                    node = node->next;
                    cur = node->fir_node;
                    cur_ind = 0;
                }
                while (n_tmp--) {
                    cur = cur->next;
                    cur_ind++;
                }
            } else if (n > 0) {
                int n_tmp = n;
                while (cur_ind < n_tmp && node->prev != deq->head) {
                    n_tmp -= cur_ind + 1;
                    node = node->prev;
                    cur = node->las_node;
                    cur = cur->prev;
                    cur_ind = node->length - 1;
                }
                while (n_tmp--) {
                    cur = cur->prev;
                    cur_ind--;
                }
            }
            return *this;
        }
        /**
         * TODO iter++
         */
        const_iterator operator++(int) {
            const_iterator tmp(deq, cur, cur_ind, node);
            if (cur_ind + 1 < node->length || (cur_ind + 1 >= node->length && node->next == deq->tail)) {
                cur_ind++;
                cur = cur->next;
            } else {
                cur_ind = 0;
                node = node->next;
                cur = node->fir_node;
            }
            return tmp;
        }
        /**
         * TODO ++iter
         */
        const_iterator& operator++() {
            if (cur_ind + 1 < node->length || (cur_ind + 1 >= node->length && node->next == deq->tail)) {
                cur_ind++;
                cur = cur->next;
            } else {
                cur_ind = 0;
                node = node->next;
                cur = node->fir_node;
            }
            return *this;
        }
        /**
         * TODO iter--
         */
        const_iterator operator--(int) {
            const_iterator tmp(deq, cur, cur_ind, node);
            if (cur_ind < 1 && node->prev != deq->head) {
                node = node->prev;
                cur_ind = node->length - 1;
                cur = node->las_node->prev;
            } else {
                cur_ind--;
                cur = cur->prev;
            }
            return tmp;
        }
        /**
         * TODO --iter
         */
        const_iterator& operator--() {
            if (cur_ind < 1 && node->prev != deq->head) {
                node = node->prev;
                cur_ind = node->length - 1;
                cur = node->las_node->prev;
            } else {
                cur_ind--;
                cur = cur->prev;
            }
            return *this;
        }
        /**
         * TODO *it
         * remember to throw
         */
        const T& operator*() const {
            if (cur == nullptr || cur->data == nullptr) throw invalid_iterator();
            return *cur->data;
        }
        /**
         * TODO it->field
         */
        const T* operator->() const noexcept { return cur->data; }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {
            return (cur == rhs.cur);
        }
        bool operator==(const const_iterator &rhs) const {
            return (cur == rhs.cur);
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return (cur != rhs.cur);
        }
        bool operator!=(const const_iterator &rhs) const {
            return (cur != rhs.cur);
        }
    friend class deque<T>;
    friend class iterator;
    };
    /**
     * TODO Constructors
     */
    deque():head(new map_node), tail(new map_node), map_size(0) {
        map_node* new_node = new map_node;
        new_node->index = 1;
        new_node->length = 0;
        new_node->next = tail;
        new_node->prev = head;
        head->next = new_node;
        tail->prev = new_node;
        chunk_node* new_chunk_node = new chunk_node;
        new_node->fir_node = new_chunk_node;
        new_node->las_node = new_chunk_node;
    }
    deque(const deque &other):head(new map_node), tail(new map_node), map_size(other.map_size) {
        map_node* other_ptr = other.head->next;
        map_node* ptr = head;
        while (other_ptr != other.tail) {
            //新建 map_node
            map_node* new_node = new map_node;
            new_node->length = other_ptr->length;
            new_node->index = other_ptr->index;
            new_node->prev = ptr;
            ptr->next = new_node;
            //开始构建 block
            chunk_node* other_chunk_node = other_ptr->fir_node;
            if (other_ptr->fir_node == other_ptr->las_node) {
                chunk_node* new_chunk_node = new chunk_node;
                new_node->fir_node = new_chunk_node;
                new_node->las_node = new_chunk_node;
            } else {
                //确定 fir_node
                chunk_node* new_chunk_node = new chunk_node;
                new_node->fir_node = new_chunk_node;
                new_chunk_node->data = new T(*other_chunk_node->data);
                other_chunk_node = other_chunk_node->next;
                while (other_chunk_node != other_ptr->las_node)
                {
                    chunk_node* tmp_chunk_node = new chunk_node;
                    new_chunk_node->next = tmp_chunk_node;
                    tmp_chunk_node->prev = new_chunk_node;
                    tmp_chunk_node->data = new T(*other_chunk_node->data);
                    new_chunk_node = tmp_chunk_node;
                    other_chunk_node = other_chunk_node->next;
                }
                //set las_node
                chunk_node* las_node = new chunk_node;
                new_node->las_node = las_node;
                las_node->prev = new_chunk_node;
                new_chunk_node->next = las_node;
            }
            other_ptr = other_ptr->next;
            ptr = new_node;
        }
        tail->prev = ptr;
        ptr->next = tail;
    }
    /**
     * TODO Deconstructor
     */
    ~deque() {
        map_node* ptr = head->next;
        delete head;
        while (ptr != tail) {
            chunk_node* tmp_node = ptr->fir_node;
            while (tmp_node != ptr->las_node) {
                delete tmp_node->data;
                tmp_node = tmp_node->next;
                delete tmp_node->prev;
            }
            delete ptr->las_node;
            ptr = ptr->next;
            delete ptr->prev;
        }
        delete tail;
    }
    /**
     * TODO assignment operator
     */
    deque &operator=(const deque &other) {
        if (this == &other) return *this; 
        map_node* ptr = head->next;
        delete head;
        while (ptr != tail) {
            chunk_node* tmp_node = ptr->fir_node;
            while (tmp_node != ptr->las_node) {
                delete tmp_node->data;
                tmp_node = tmp_node->next;
                delete tmp_node->prev;
            }
            delete ptr->las_node;
            ptr = ptr->next;
            delete ptr->prev;
        }
        delete tail;
        head = new map_node;
        tail = new map_node;
        map_size = other.map_size;
        map_node* other_ptr = other.head->next;
        ptr = head;
        while (other_ptr != other.tail) {
            //新建 map_node
            map_node* new_node = new map_node;
            new_node->length = other_ptr->length;
            new_node->index = other_ptr->index;
            new_node->prev = ptr;
            ptr->next = new_node;
            //开始构建 block
            chunk_node* other_chunk_node = other_ptr->fir_node;
            if (other_ptr->fir_node == other_ptr->las_node) {
                chunk_node* new_chunk_node = new chunk_node;
                new_node->fir_node = new_chunk_node;
                new_node->las_node = new_chunk_node;
            } else {
                //确定 fir_node
                chunk_node* new_chunk_node = new chunk_node;
                new_node->fir_node = new_chunk_node;
                new_chunk_node->data = new T(*other_chunk_node->data);
                other_chunk_node = other_chunk_node->next;
                while (other_chunk_node != other_ptr->las_node)
                {
                    chunk_node* tmp_chunk_node = new chunk_node;
                    new_chunk_node->next = tmp_chunk_node;
                    tmp_chunk_node->prev = new_chunk_node;
                    tmp_chunk_node->data = new T(*other_chunk_node->data);
                    new_chunk_node = tmp_chunk_node;
                    other_chunk_node = other_chunk_node->next;
                }
                //set las_node
                chunk_node* las_node = new chunk_node;
                new_node->las_node = las_node;
                las_node->prev = new_chunk_node;
                new_chunk_node->next = las_node;
            }
            other_ptr = other_ptr->next;
            ptr = new_node;
        }
        tail->prev = ptr;
        ptr->next = tail;
        return *this;
    }
    /**
     * access specified element with bounds checking
     * throw index_out_of_bound if out of bound.
     */
    T & at(const size_t &pos) {
        if (pos >= map_size) throw index_out_of_bound();
        int ind = pos;
        iterator tmp(this, head->next->fir_node, 0, head->next);
            while (tmp.cur_ind + ind >= tmp.node->length) {
                ind -= tmp.node->length - tmp.cur_ind;
                tmp.node = tmp.node->next;
                tmp.cur = tmp.node->fir_node;
                tmp.cur_ind = 0;
            }
            while (ind--) {
                tmp.cur = tmp.cur->next;
                tmp.cur_ind++;
            }
            return *tmp.cur->data;
    }
    const T & at(const size_t &pos) const {
        if (pos >= map_size) throw index_out_of_bound();
        int ind = pos;
        const_iterator tmp(this, head->next->fir_node, 0, head->next);
            while (tmp.cur_ind + ind >= tmp.node->length) {
                ind -= tmp.node->length - tmp.cur_ind;
                tmp.node = tmp.node->next;
                tmp.cur = tmp.node->fir_node;
                tmp.cur_ind = 0;
            }
            while (ind--) {
                tmp.cur = tmp.cur->next;
                tmp.cur_ind++;
            }
            return *tmp.cur->data;
    }
    T & operator[](const size_t &pos) {
        if (pos >= map_size) throw index_out_of_bound();
        int ind = pos;
        iterator tmp(this, head->next->fir_node, 0, head->next);
            while (tmp.cur_ind + ind >= tmp.node->length) {
                ind -= tmp.node->length - tmp.cur_ind;
                tmp.node = tmp.node->next;
                tmp.cur = tmp.node->fir_node;
                tmp.cur_ind = 0;
            }
            while (ind--) {
                tmp.cur = tmp.cur->next;
                tmp.cur_ind++;
            }
            return *tmp.cur->data;
    }
    const T & operator[](const size_t &pos) const {
        if (pos >= map_size) throw index_out_of_bound();
        int ind = pos;
        const_iterator tmp(this, head->next->fir_node, 0, head->next);
            while (tmp.cur_ind + ind >= tmp.node->length) {
                ind -= tmp.node->length - tmp.cur_ind;
                tmp.node = tmp.node->next;
                tmp.cur = tmp.node->fir_node;
                tmp.cur_ind = 0;
            }
            while (ind--) {
                tmp.cur = tmp.cur->next;
                tmp.cur_ind++;
            }
            return *tmp.cur->data;
    }
    /**
     * access the first element
     * throw container_is_empty when the container is empty.
     */
    const T & front() const {
        if (map_size == 0) throw container_is_empty();
        return *(head->next->fir_node->data);
    }
    /**
     * access the last element
     * throw container_is_empty when the container is empty.
     */
    const T & back() const {
        if (map_size == 0) throw container_is_empty();
        return *(tail->prev->las_node->prev->data);
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() {
        iterator tmp(this, head->next->fir_node, 0, head->next);
        return tmp;
    }
    /**
     * if you don't intend to modify the thing that ptr points to,
     * then ptr should be declared as a pointer-to-const
     * in this case a const ptr must be assigned to another ptr otherwise there will be an error
     */
    const_iterator cbegin() const {
        const_iterator tmp(this, head->next->fir_node, 0, head->next);
        return tmp;
    }
    /**
     * returns an iterator to the end.
     */
    iterator end() {
        iterator tmp(this, tail->prev->las_node, tail->prev->length, tail->prev);
        return tmp;
    }
    const_iterator cend() const {
        const_iterator tmp(this, tail->prev->las_node, tail->prev->length, tail->prev);
        return tmp;
    }
    /**
     * checks whether the container is empty.
     */
    bool empty() const { return (map_size == 0); }
    /**
     * returns the number of elements
     */
    size_t size() const { return map_size; }
    /**
     * clears the contents
     */
    void clear() {
        map_node* ptr = head->next;
        while (ptr != tail) {
            chunk_node* tmp_node = ptr->fir_node;
            while (tmp_node != ptr->las_node) {
                delete tmp_node->data;
                tmp_node = tmp_node->next;
                delete tmp_node->prev;
            }
            delete ptr->las_node;
            ptr = ptr->next;
            delete ptr->prev;
        }
        map_node* new_node = new map_node;
        new_node->index = 1;
        new_node->length = 0;
        new_node->next = tail;
        new_node->prev = head;
        head->next = new_node;
        tail->prev = new_node;
        chunk_node* new_chunk_node = new chunk_node;
        new_node->fir_node = new_chunk_node;
        new_node->las_node = new_chunk_node;
        map_size = 0;
    }
    /**
     * inserts elements at the specified location on in the container.
     * inserts value before pos
     * returns an iterator pointing to the inserted value
     *     throw if the iterator is invalid or it point to a wrong place.
     */
private:
    void merge(map_node* cur_block, map_node* next_block) {
        //链接两个block
        cur_block->las_node->prev->next = next_block->fir_node;
        next_block->fir_node->prev = cur_block->las_node->prev;
        //删掉前面一个的last_node
        delete cur_block->las_node;
        //重新设置cur_block的参数
        cur_block->las_node = next_block->las_node;
        cur_block->length += next_block->length;
        //链接新的两个map_node
        cur_block->next = next_block->next;
        next_block->next->prev = cur_block;
        delete next_block;
        map_node* tmp = cur_block->next;
        while (tmp != tail) {
            tmp->index--;
            tmp = tmp->next;
        }
    }
    //检查有哪些 chunk 需要 merge 或者有 chunk 已经只有0个元素了，需要删掉或者不删（只剩一个 chunk 的情况）
    void maintainList() {
        map_node* tmp = head->next;
        while (tmp->next != tail)
        {
            if (tmp->length == 0) {
                delete tmp->las_node;
                tmp->prev->next = tmp->next;
                tmp->next->prev = tmp->prev;
                map_node* tmp_node = tmp->next;
                delete tmp;
                while (tmp_node != tail) {
                    tmp_node->index--;
                    tmp_node = tmp_node->next;
                }
                return;
            }
            if (tmp->length + tmp->next->length <= (chunk_size >> 1)) {
                merge(tmp, tmp->next);
                //合并后可能新的 tail->next == tail，需要特判
                if (tmp->next == tail) return;
            }
            tmp = tmp->next;
        }
        if (tmp->next == tail && tmp->prev != head) {
            if (tmp->length == 0) {
                delete tmp->las_node;
                tmp->prev->next = tmp->next;
                tmp->next->prev = tmp->prev;
                map_node* tmp_node = tmp->next;
                delete tmp;
                while (tmp_node != tail) {
                    tmp_node->index--;
                    tmp_node = tmp_node->next;
                }
                return;
            }
        }
        return;
    }
    //将 cur 及以后的 chunk_node 装到一个新的 block 里面
    void spilt(chunk_node* cur, map_node* cur_block) {
        //new_block is the map_node of the new block
        map_node* new_block = new map_node;
        new_block->index = cur_block->index + 1;
        //把新的 map_node 和前后连起来
        new_block->prev = cur_block;
        new_block->next = cur_block->next;
        cur_block->next->prev = new_block;
        cur_block->next = new_block;
        //暂时只有 cur 这一个 chunk_node 被存到新的 block 里
        new_block->length = 1;
        //TODO:这里 cur->prev 还未设置
        new_block->fir_node = cur;
        new_block->las_node = cur_block->las_node;
        //下面开始设置 cur_block (老的 block)
        //cur_block_las 是 cur_block 新设置的 las_node
        chunk_node* cur_block_las = new chunk_node;
        cur_block->las_node = cur_block_las;
        cur_block_las->prev = cur->prev;
        cur->prev->next = cur_block_las;
        //TODO:这里设置 cur->prev(new_block->fir_node->prev)
        cur->prev = nullptr;
        //下面开始设置 new_block->length
        cur = cur->next;
        while (cur != new_block->las_node) {
            new_block->length++;
            cur = cur->next;
        }
        cur_block->length -= new_block->length;
        //更新 map_bode 的 index
        while (new_block->next != tail)
        {
            new_block = new_block->next;
            new_block->index++;
        }
    }
    //判断是否是 end() 以外的 iterator
    bool pointer_not_exist(iterator pos) {
        if (pos.deq == nullptr || pos.cur == nullptr || pos.node == nullptr || pos.cur_ind < 0) return true;
        map_node* tmp_map_node = head;
        while (tmp_map_node->next != tail) {
            tmp_map_node = tmp_map_node->next;
            if (tmp_map_node->index == pos.node->index) {
                if (pos.cur_ind < tmp_map_node->length) return false; else return true;
            }
        }
        return true;
    }
    //判断是否是 iterator (including end())
    bool iterator_not_exist(iterator pos) {
        if (pos == end()) return false;
        if (pos.deq == nullptr || pos.cur == nullptr || pos.node == nullptr || pos.cur_ind < 0) return true;
        map_node* tmp_map_node = head;
        while (tmp_map_node->next != tail) {
            tmp_map_node = tmp_map_node->next;
            if (tmp_map_node->index == pos.node->index) {
                if (pos.cur_ind < tmp_map_node->length) return false; else return true;
            }
        }
        return true;
    }
public:
    iterator insert(iterator pos, const T &value) {
        if (pos.deq != this || iterator_not_exist(pos)) throw invalid_iterator();
        chunk_node* new_node = new chunk_node;
        pos.node->length++;
        map_size++;
        //insert at thr first node of the block
        if (pos.cur_ind == 0) {
            new_node->next = pos.cur;
            pos.node->fir_node = new_node;
            new_node->data = new T(value);
            pos.cur->prev = new_node;
        } else {
            //ordinary insert
            new_node->next = pos.cur;
            new_node->prev = pos.cur->prev;
            new_node->data = new T(value);
            new_node->prev->next = new_node;
            new_node->next->prev = new_node;
        }
        if (pos.node->length >= chunk_size) {
            chunk_node* tmp_node = pos.node->fir_node;
            int tmp = 0;
            while (tmp < (chunk_size >> 1)) {
                tmp_node = tmp_node->next;
                tmp++;
            }
            spilt(tmp_node, pos.node);
            if (pos.cur_ind < (chunk_size >> 1)) {
                iterator ans(this, new_node, pos.cur_ind, pos.node);
                return ans;
            } else {
                iterator ans(this, new_node, pos.cur_ind - (chunk_size >> 1), pos.node->next);
                return ans;
            }
        } else {
            iterator ans(this, new_node, pos.cur_ind, pos.node);
            return ans;
        }
    }
    /**
     * removes specified element at pos.
     * removes the element at pos.
     * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
     * throw if the container is empty, the iterator is invalid or it points to a wrong place.
     */
    iterator erase(iterator pos) {
        if (map_size == 0) throw container_is_empty();
        if (pos.deq != this || pointer_not_exist(pos)) throw invalid_iterator();
        //record the position of the chunk_node of the next iterator (maybe the end())
        iterator tmp = pos + 1;
        int flag = 0;
        if (tmp == end()) flag = 1;
        map_node* min_node = pos.node->prev;
        map_size--;
        if (pos.cur_ind == 0) {
            pos.node->fir_node = pos.cur->next;
            pos.cur->next->prev = nullptr;
            pos.node->length--;
        } else {
            pos.cur->prev->next = pos.cur->next;
            pos.cur->next->prev = pos.cur->prev;
            pos.node->length--;
        }
        delete pos.cur->data;
        delete pos.cur;
        maintainList();
        //寻找返回的 iterator 在哪个 map_node 上，是 chunk 中第几个（iterator 含这些元素）
        if (flag) return end();
        size_t ind = 0;
        chunk_node* tmp_chunk_node = tmp.cur;
        while (tmp_chunk_node->prev != nullptr) {
            ind++;
            tmp_chunk_node = tmp_chunk_node->prev;
        }
        tmp.cur_ind = ind;
        map_node* tmp_map_node = head->next;
        while (tmp_map_node != tail) {
            if (tmp_map_node->fir_node == tmp_chunk_node) {
                tmp.node = tmp_map_node;
                return tmp;
            }
            tmp_map_node = tmp_map_node->next;
        }
    }
    /**
     * adds an element to the end
     */
    void push_back(const T &value) {
        iterator pos = end(); 
        chunk_node* new_node = new chunk_node;
        pos.node->length++;
        map_size++;
        //insert at thr first node of the block
        if (pos.cur_ind == 0) {
            new_node->next = pos.cur;
            pos.node->fir_node = new_node;
            new_node->data = new T(value);
            pos.cur->prev = new_node;
        } else {
            //ordinary insert
            new_node->next = pos.cur;
            new_node->prev = pos.cur->prev;
            new_node->data = new T(value);
            new_node->prev->next = new_node;
            new_node->next->prev = new_node;
        }
        if (pos.node->length >= chunk_size) {
            chunk_node* tmp_node = pos.node->fir_node;
            int tmp = 0;
            while (tmp < (chunk_size >> 1)) {
                tmp_node = tmp_node->next;
                tmp++;
            }
            spilt(tmp_node, pos.node);
        }
    }
    /**
     * removes the last element
     *     throw when the container is empty.
     */
    void pop_back() {
        if (map_size == 0) throw container_is_empty();
        iterator pos = end() - 1;
        map_size--;
        //考虑pop 后 chunk 空了后可能需要删除的情况
        if (pos.cur_ind == 0) {
            if (pos.node->prev == head) {
                pos.node->fir_node = pos.cur->next;
                pos.cur->next->prev = nullptr;
                pos.node->length--;
            } else {
                delete pos.node->las_node;
                pos.node->prev->next = tail;
                tail->prev = pos.node->prev;
                delete pos.node;
            }
        } else {
            pos.cur->prev->next = pos.cur->next;
            pos.cur->next->prev = pos.cur->prev;
            pos.node->length--;
            if (pos.node->prev != head && pos.node->prev->length + pos.node->length <= (chunk_size >> 1)) {
                merge(pos.node->prev, pos.node);
            }
        }
        delete pos.cur->data;
        delete pos.cur;
    }
    /**
     * inserts an element to the beginning.
     */
    void push_front(const T &value) {
        iterator pos = begin(); 
        chunk_node* new_node = new chunk_node;
        pos.node->length++;
        map_size++;
        new_node->next = pos.cur;
        pos.node->fir_node = new_node;
        new_node->data = new T(value);
        pos.cur->prev = new_node;
        //判断当前 chunk 是否已经达到 chunk_size 上限
        if (pos.node->length >= chunk_size) {
            chunk_node* tmp_node = pos.node->fir_node;
            int tmp = 0;
            while (tmp < (chunk_size >> 1)) {
                tmp_node = tmp_node->next;
                tmp++;
            }
            spilt(tmp_node, pos.node);
        }
    }
    /**
     * removes the first element.
     *     throw when the container is empty.
     */
    void pop_front() {
        if (map_size == 0) throw container_is_empty();
        iterator pos = begin();
        map_size--;
        pos.node->fir_node = pos.cur->next;
        pos.cur->next->prev = nullptr;
        pos.node->length--;
        delete pos.cur->data;
        delete pos.cur;
        //判断是否需要删除为0的 chunk
        if (pos.node->length == 0 && pos.node->next != tail) {
            delete pos.node->las_node;
            head->next = pos.node->next;
            pos.node->next->prev = head;
            delete pos.node;
            map_node* tmp_node = head->next;
            while (tmp_node != tail) {
                tmp_node->index--;
                tmp_node = tmp_node->next;
            }
            //只有chunk数量超过2个才可以合并
        } else if (pos.node->length != 0 && pos.node->next != tail) {
            if (pos.node->length + pos.node->next->length <= (chunk_size >> 1))
            merge(pos.node, pos.node->next);
        }
    }
};

}

#endif
