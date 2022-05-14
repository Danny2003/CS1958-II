#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <cstddef>
// only for std::less<T>
#include <functional>

#include "exceptions.hpp"
#include "utility.hpp"

/**
 * reference: Introduction to Algorithms (pseudocode and graph), https://github.com/q4x3 (the design of structure)
 * The code is similar to the code on Introduction to Algorithms, cause the code there is too classic to change
 */
namespace sjtu {
enum color_type { RED, BLACK };
/**
 * notice that the Compare class may be some **self-defined** order function
 * object instead of std::less<Key>
 */
template <class Key, class T, class Compare = std::less<Key> >
class map {
 public:
  typedef pair<const Key, T> value_type;
  class Node {
    public:
    value_type *data;
    Node *left, *right, *parent;
    color_type color;
    Node()
        : data(nullptr),
          left(nullptr),
          right(nullptr),
          parent(nullptr),
          color(RED) {}
    Node(const value_type &obj, 
         Node *l = nullptr, Node *r = nullptr,
         Node *p = nullptr, color_type c = RED)
        : left(l), right(r), parent(p), color(c) {
      data = new value_type(obj);
    }
    ~Node() {
      if (data) delete data;
    }
  };
  Node *root, *nil;
  /**
   * compare_function is an instance of class Compare i.e. std::less<Key> by
   * default.
   * std::less is a function object implementing x < y deducing argument and
   * return types. It has a public member function operator() that checks
   * whether the first argument is less than the second.
   * This instance compare_function needs to be copied during construction since
   * the obj.compare_function may **not** be std::less<Key> but some
   * **self-defined** order function object.
   */
  Compare compare_function;
  size_t mapSize;
  Node *pred(Node *p, Node *_root) const {
    if (mapSize == 0) return nullptr;
    if (p == nil) {
      while (_root->right != nil) _root = _root->right;
      return _root;
    }
    if (p->left != nil) {  // go left
      p = p->left;
      while (p->right != nil) p = p->right;
      return p;
    } else {
      // go up
      while (p->parent->left == p)
        p = p->parent;        // root->parent->left != root
      if (p->parent == nil)  // p is root now
        return nullptr;
      else
        return p->parent;
    }
  }
  Node *succ(Node *p) const {
    if (mapSize == 0) return nullptr;
    if (p == nil) return nullptr;
    if (p->right != nil) {  // go right
      p = p->right;
      while (p->left != nil) p = p->left;
      return p;
    } else {
      // go up
      while (p->parent->right == p)
        p = p->parent;   // root->parent->left != root
      return p->parent;  // return nil if p == root
    }
  }
  // compare function for class Key which can replace < > ==
  int compare(const Key &left, const Key &right) const {
    if (compare_function(left, right)) {
      return 1;  // left < right
    } else if (compare_function(right, left)) {
      return -1;  // left > right
    } else {
      return 0;  // left == right
    }
  }
  /**
   * utility for at, [], find
   * if not found, return nil.
   */
  Node *search(Node *_root, const Key &key) const {
    int sign;
    while (true) {
      if (_root == nil) return nil;
      sign = compare(key, _root->data->first);
      if (sign == 0) {
        return _root;
      } else if (sign == 1) {
        _root = _root->left;
      } else {
        _root = _root->right;
      }
    }
  }
  void leftRotate(Node *x) {
    // if x == nil, since x->right == nil the function will also return
    Node *y = x->right;
    if (y == nil) return;
    x->right = y->left;
    if (y->left != nil) y->left->parent = x;
    y->parent = x->parent;
    // in case x->parent doesn't exist and root is changed
    if (x->parent == nil) {
      root = y;
    } else if (x == x->parent->left) {
      x->parent->left = y;
    } else {
      x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
  }
  void rightRotate(Node *x) {
    // if x == nil, since x->left == nil the function will also return
    Node *y = x->left;
    if (y == nil) return;
    x->left = y->right;
    if (y->right != nil) y->right->parent = x;
    y->parent = x->parent;
    // in case x->parent doesn't exist and root is changed
    if (x->parent == nil) {
      root = y;
    } else if (x == x->parent->left) {
      x->parent->left = y;
    } else {
      x->parent->right = y;
    }
    y->right = x;
    x->parent = y;
  }
  /**
   * utility for _insert
   */
  void insertFixUp(Node *z) {
    while (z->parent->color ==
           RED) {  // z->parent->color != BLACK thus z->parent != root
      if (z->parent == z->parent->parent->left) {
        Node *y = z->parent->parent->right;
        if (y->color == RED) {
          z->parent->color = BLACK;        // case 1
          z->parent->parent->color = RED;  // case 1
          y->color = BLACK;                // case 1
          z = z->parent->parent;           // case 1
        } else {
          if (z == z->parent->right) {
            z = z->parent;  // case 2
            leftRotate(z);  // case 2
          }
          z->parent->color = BLACK;               // case 3
          z->parent->parent->color = RED;         // case 3
          rightRotate(z->parent->parent);  // case 3
        }
      } else {
        Node *y = z->parent->parent->left;
        if (y->color == RED) {
          z->parent->color = BLACK;        // case 1
          z->parent->parent->color = RED;  // case 1
          y->color = BLACK;                // case 1
          z = z->parent->parent;           // case 1
        } else {
          if (z == z->parent->left) {
            z = z->parent;   // case 2
            rightRotate(z);  // case 2
          }
          z->parent->color = BLACK;              // case 3
          z->parent->parent->color = RED;        // case 3
          leftRotate(z->parent->parent);  // case 3
        }
      }
    }
    root->color = BLACK;
  }
  /**
   * utility for insert
   * pair.first: return the Node that prevent the insertion
   * or the Node that is inserted
   * pair.second: return whether insertion is successful
   */
  pair<Node *, bool> _insert(const value_type &obj) {
    Node *y = nil, *x = root;
    Key key = obj.first;
    while (x != nil) {
      y = x;
      int sign = compare(key, x->data->first);
      if (sign == 1) {
        x = x->left;
      } else if (sign == -1) {
        x = x->right;
      } else {
        return pair<Node *, bool>(x, false);
      }
    }
    Node *z = new Node(obj, nullptr, nullptr, y, RED);
    if (y == nil)
      root = z;
    else {
      int sign = compare(key, y->data->first);
      if (sign == 1) {
        y->left = z;
      } else {
        y->right = z;
      }
    }
    z->left = z->right = nil;
    z->color = RED;
    insertFixUp(z);
    return pair<Node *, bool>(z, true);
  }
  void _clear(Node *u) {
    if (u == nil) return;
    _clear(u->left);
    _clear(u->right);
    delete u;
  }
  // utility for erase
  void transplant(Node *u, Node *v) {
    if (u->parent == nil) {
      root = v;
    } else {
      if (u == u->parent->left) 
        u->parent->left = v;
      else
        u->parent->right = v;
    }
    v->parent = u->parent;
  }
  /**
   * utility for _erase
   * if x == root all BLACK_length-- (no effects)
   * if x == nil && x->parent == nil, root has been changed in transplant
   *   so x == root and nothing wrong will happen
   */
  void eraseFixUp(Node *x) {
    while (x != root && x->color == BLACK) {
      if (x == x->parent->left) {
        Node *w = x->parent->right;
        if (w->color == RED) {
          x->parent->color = RED;
          w->color = BLACK;
          leftRotate(x->parent);
        } else {
          if (w->right->color == BLACK) {
            if (w->left->color == BLACK) {
              w->color = RED;
              x = x->parent;
            } else {
              w->color = RED;
              w->left->color = BLACK;
              rightRotate(w);
            }
          } else {
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->right->color = BLACK;
            leftRotate(x->parent);
            x = root;
          }
        }
      } else {
        Node *w = x->parent->left;
        if (w->color == RED) {
          x->parent->color = RED;
          w->color = BLACK;
          rightRotate(x->parent);
        } else {
          if (w->left->color == BLACK) {
            if (w->right->color == BLACK) {
              w->color = RED;
              x = x->parent;
            } else {
              w->color = RED;
              w->right->color = BLACK;
              leftRotate(w);
            }
          } else {
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->left->color = BLACK;
            rightRotate(x->parent);
            x = root;
          }
        }
      }
    }
    x->color = BLACK;  
  }
  // utility for erase
  void _erase(Node *z) {
    Node *y = z;  // y is the only Node that may be lose its original color
    color_type y_original_color = z->color;
    Node *x; // x is the Node that will take y's original position
    if (z->left == nil) {  // <= 1 son
      x = z->right;
      transplant(z, z->right);
    } else if (z->right == nil) {  // <= 1 son
      x = z->left;
      transplant(z, z->left);
    } else {  // 2 sons
			y = z->right;
			while (y->left != nil) y = y->left;
      y_original_color = y->color;
      x = y->right;
      if (y == z->right) {
        x->parent = y;
      } else {
        transplant(y, y->right);
        y->right = z->right;
        y->right->parent = y;
      }
      transplant(z, y);
      y->left = z->left;
      y->left->parent = y;
      y->color = z->color;
    }
    delete z;
    if (y_original_color == BLACK) 
      eraseFixUp(x);
  }
  /**
   * utility for copy constructor
   * used when obj_root != obj_nil
   */
  void copy(Node *self_root, Node *obj_root, Node *obj_nil) {
    if (obj_root->left) {
      if (obj_root->left == obj_nil)
        self_root->left = nil;
      else {
        self_root->left = new Node(*(obj_root->left->data), nullptr, nullptr,
                                   self_root, obj_root->left->color);
        copy(self_root->left, obj_root->left, obj_nil);
      }
    }
    if (obj_root->right) {
      if (obj_root->right == obj_nil)
        self_root->right = nil;
      else {
        self_root->right = new Node(*(obj_root->right->data), nullptr, nullptr,
                                    self_root, obj_root->right->color);
        copy(self_root->right, obj_root->right, obj_nil);
      }
    }
  }
  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator {
   public:
    map<Key, T, Compare> *self;
    Node *pos;
    iterator() : self(nullptr), pos(nullptr) {}
    iterator(map *_self, Node *_pos) : self(_self), pos(_pos) {}
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      Node *tmp = pos;
      pos = self->succ(pos);
      if (pos == nullptr) throw invalid_iterator();
      return iterator(self, tmp);
    }
    /**
     * TODO ++iter
     */
    iterator &operator++() {
      pos = self->succ(pos);
      if (pos == nullptr) throw invalid_iterator();
      return *this;  // should be non-const lvalue
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      Node *tmp = pos;
      pos = self->pred(pos, self->root);
      if (pos == nullptr) throw invalid_iterator();
      return iterator(self, tmp);
    }
    /**
     * TODO --iter
     */
    iterator &operator--() {
      pos = self->pred(pos, self->root);
      if (pos == nullptr) throw invalid_iterator();
      return *this;  // should be non-const lvalue
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory).
     */
    value_type &operator*() const { return *(pos->data); }
    bool operator==(const iterator &rhs) const {
      return (self == rhs.self) && (pos == rhs.pos);
    }
    bool operator==(const const_iterator &rhs) const {
      return (self == rhs.self) && (pos == rhs.pos);
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return (self != rhs.self) || (pos != rhs.pos);
    }
    bool operator!=(const const_iterator &rhs) const {
      return (self != rhs.self) || (pos != rhs.pos);
    }
    value_type *operator->() const noexcept { return pos->data; }
    friend class const_iterator;
    friend class map<Key, T>;
  };
  /**
   * it should has similar member method as iterator,
   * and it should be able to construct from an iterator.
   */
  class const_iterator {
   public:
    const map<Key, T, Compare> *self;
    Node *pos;
    const_iterator() : self(nullptr), pos(nullptr) {}
    const_iterator(const iterator &other) : self(other.self), pos(other.pos) {}
    const_iterator(const map *_self, Node *_pos) : self(_self), pos(_pos) {}
    /**
     * TODO iter++
     */
    const_iterator operator++(int) {
      Node *tmp = pos;
      pos = self->succ(pos);
      if (pos == nullptr) throw invalid_iterator();
      return const_iterator(self, tmp);
    }
    /**
     * TODO ++iter
     */
    const_iterator &operator++() {
      pos = self->succ(pos);
      if (pos == nullptr) throw invalid_iterator();
      return *this;  // should be non-const lvalue
    }
    /**
     * TODO iter--
     */
    const_iterator operator--(int) {
      Node *tmp = pos;
      pos = self->pred(pos, self->root);
      if (pos == nullptr) throw invalid_iterator();
      return const_iterator(self, tmp);
    }
    /**
     * TODO --iter
     */
    const_iterator &operator--() {
      pos = self->pred(pos, self->root);
      if (pos == nullptr) throw invalid_iterator();
      return *this;  // should be non-const lvalue
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory).
     */
    const value_type &operator*() const { return *(pos->data); }
    bool operator==(const iterator &rhs) const {
      return (self == rhs.self) && (pos == rhs.pos);
    }
    bool operator==(const const_iterator &rhs) const {
      return (self == rhs.self) && (pos == rhs.pos);
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return (self != rhs.self) || (pos != rhs.pos);
    }
    bool operator!=(const const_iterator &rhs) const {
      return (self != rhs.self) || (pos != rhs.pos);
    }
    const value_type *operator->() const noexcept { return pos->data; }
    friend class iterator;
    friend class map<Key, T>;
  };
  /**
   * default constructor
   * root->parent = nil
   * no requirements for nil->parent
   * default version: root == nil
   */
  map() {
		root = nil = new Node;
    mapSize = 0;
    root->parent = nil;
    nil->left = nil->right = nil;
    // default is RED not BLACK
    root->color = BLACK;
    nil->color = BLACK;
  }
  /**
   * copy constructor
   * root->parent = nil
   * no requirements for nil->parent
   */
  map(const map &other)
      : mapSize(other.mapSize),
        compare_function(other.compare_function) {
		root = nil = new Node;
    root->parent = nil;
    nil->left = nil->right = nil;
    // copy the whole map if other isn't the default version
    if (other.root != other.nil) {
      // new space for root
      // root->parent = nil
      root = new Node(*(other.root->data), nullptr, nullptr, nil, BLACK);
      copy(root, other.root, other.nil);
    }
  }
  /**
   * operator=
   * nil == root->parent
   * no requirements for nil->parent
   */
  map &operator=(const map &other) {
    if (&other == this) return *this;
    // return to the default version
    if (root != nil) _clear(root);
    root = nil;
    root->parent = nil;
    nil->left = nil->right = nil;
    // copy the whole map if other isn't the default version
    if (other.root != other.nil) {
      root = new Node(*(other.root->data), nullptr, nullptr, nil, BLACK);
      copy(root, other.root, other.nil);
    }
    mapSize = other.mapSize;
    compare_function = other.compare_function;
    return *this;
  }
  ~map() {
    if (root != nil) _clear(root);
    delete nil;
  }
  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent
   * to key. If no such element exists, an exception of type
   * 'index_out_of_bound'
   */
  T &at(const Key &key) {
    Node *tmp = search(root, key);
    if (tmp == nil) throw index_out_of_bound();
    return tmp->data->second;
  }
  const T &at(const Key &key) const {
    Node *tmp = search(root, key);
    if (tmp == nil) throw index_out_of_bound();
    return tmp->data->second;
  }
  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   * performing an insertion if such key does not already exist.
   * In this context, T must have a default constructor
   */
  T &operator[](const Key &key) {
    Node *tmp_Node = search(root, key);
    if (tmp_Node == nil) {
      pair<Node *, bool> tmp = _insert(value_type(key, T()));
      ++mapSize;
      return (tmp.first)->data->second;
    } else
      return tmp_Node->data->second;
  }
  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T &operator[](const Key &key) const {
    Node *tmp = search(root, key);
    if (tmp == nil) throw index_out_of_bound();
    return tmp->data->second;
  }
  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    Node *p = root;  // nil->left == nil
    while (p->left != nil) p = p->left;
    return iterator(this, p);
  }
  const_iterator cbegin() const {
    Node *p = root;  // nil->left == nil
    while (p->left != nil) p = p->left;
    return const_iterator(this, p);
  }
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() { return iterator(this, nil); }
  const_iterator cend() const { return const_iterator(this, nil); }
  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const { return (mapSize == 0); }
  /**
   * returns the number of elements.
   */
  size_t size() const { return mapSize; }
  /**
   * clears the contents
   */
  void clear() {
    _clear(root);
    mapSize = 0;
    root = nil;
  }
  /**
   * insert an element.
   * return a pair, the first of the pair is
   * the iterator to the new element (or the element that prevented the
   * insertion), the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type &value) {
    pair<Node *, bool> tmp = _insert(value);
    if (tmp.second) ++mapSize;
    return pair<iterator, bool>(iterator(this, tmp.first), tmp.second);
  }
  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points an
   * element out of this i.e. p.self != this )
   */
  void erase(iterator p) {
    if (p.self != this || p.pos == nil) throw invalid_iterator();
    --mapSize;
    _erase(p.pos);
  }
  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    return ((search(root, key) != nil) ? 1 : 0);
  }
  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key &key) {
    Node *tmp = search(root, key);
    return iterator(this, tmp);  // don't use end()
  }
  const_iterator find(const Key &key) const {
    Node *tmp = search(root, key);
    return const_iterator(this, tmp);  //don't use end()
  }
};

}  // namespace sjtu

#endif
