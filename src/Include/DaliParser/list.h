/* %W%  %G%     %R% */
#ifndef LIST_H
#define LIST_H 

// -*- C++ -*- 
#include <iostream>

using namespace std;

template <class Type> class List;
template <class Type> class Listiter;

template <class Type>
class ListNode {
    Type val;
    ListNode<Type> *next;
    ListNode<Type> *prev;
    friend class List<Type>;
    friend class Listiter<Type>;
};

template <class Type>
class List{
  ListNode<Type> *head, *last;
//  Listiter<Type> *list_iter_ptr;
  void *list_iter_ptr;

  friend class Listiter<Type>;

 public:
  List() { head = NULL; last = NULL; list_iter_ptr = NULL;}

  int destroy() {
    ListNode<Type> *ptr = head;
    while(ptr){
      ListNode<Type> *tmp = ptr;
      ptr = ptr->next;
      free(tmp);
    }
    head = last = 0;
    list_iter_ptr = 0; 
    return 1;
  }
  ~List(){
      destroy();
  }
  void init(){ head = NULL; last = NULL; list_iter_ptr = NULL;}

  inline void put(const Type &_val);

  inline int length(void){
    ListNode<Type> *ptr = head;
    int count =0;
    while(ptr != NULL){
      count++;
      ptr = ptr->next;
    }
    return count;
  }

};

template <class Type>
class Listiter{
  ListNode<Type> *next_node_ptr;
  ListNode<Type> *prev_node_ptr;
  List<Type> *list_ptr;
  Listiter<Type> *nxt;

  friend class List<Type>;

 public:
  Listiter(List<Type> &_list){
    list_ptr = &_list;
    next_node_ptr = _list.head;
    prev_node_ptr = NULL;
    nxt = (Listiter<Type> *) _list.list_iter_ptr;
    _list.list_iter_ptr = this;
  }
  
  ~Listiter(){
    Listiter<Type> *_iter_ptr = (Listiter<Type> *) list_ptr->list_iter_ptr;
    Listiter<Type> *_prv_ptr = NULL;
    while(_iter_ptr){
      if(_iter_ptr == this)
        break;
      _prv_ptr = _iter_ptr;
      _iter_ptr = _iter_ptr->nxt;
    }
    if(_iter_ptr){
      if(_prv_ptr == NULL)
        list_ptr->list_iter_ptr = _iter_ptr->nxt;
      else
        _prv_ptr->nxt = _iter_ptr->nxt;
    }
  }

  inline void reset(){next_node_ptr = list_ptr->head; prev_node_ptr = NULL;}
  inline int at_end(){
    return next_node_ptr ? 0 : 1;
  }
  inline void next(Type &val){
    ListNode<Type> *tmp = next_node_ptr;
    if(next_node_ptr){
      next_node_ptr = next_node_ptr->next;
      val = tmp->val;
      prev_node_ptr = tmp;
    }
    else
      cout << "error - end of list reached" << endl;
  }
  inline void next(Type * &val_ptr){
    ListNode<Type> *tmp = next_node_ptr;
    if(next_node_ptr){
      next_node_ptr = next_node_ptr->next;
      val_ptr = &tmp->val;
      prev_node_ptr = tmp;
    }
    else
      cout << "error - end of list reached" << endl;
  }

  inline int remove_prev(void){
    if(!prev_node_ptr)
      return 0;
    if(prev_node_ptr){
      if(prev_node_ptr->prev)
        prev_node_ptr->prev->next = prev_node_ptr->next;
      else
        list_ptr->head = prev_node_ptr->next;
      if(prev_node_ptr->next)
        prev_node_ptr->next->prev = prev_node_ptr->prev;
      else
        list_ptr->last = prev_node_ptr->prev;
      
      ListNode<Type> *prv_ptr = prev_node_ptr;

      Listiter<Type> *_iter_ptr = (Listiter<Type> *) list_ptr->list_iter_ptr;
      while(_iter_ptr){
        if(_iter_ptr->next_node_ptr == prv_ptr)
          _iter_ptr->next_node_ptr = prv_ptr->next;
        if(_iter_ptr->prev_node_ptr == prv_ptr)
          _iter_ptr->prev_node_ptr = NULL;
        
        _iter_ptr = _iter_ptr->nxt;
      }
      delete prv_ptr;
    }
    return 1;
  }
};

template <class Type>
inline void List<Type>::put(const Type &_val)
{ 
    ListNode<Type> *ptr  = new ListNode<Type> ;  
    ptr->val = _val; 
    ptr->next = NULL;
    if(last){
      last->next = ptr;
      ptr->prev = last;
      last = ptr;
    }
    else{
      last = head = ptr;
      ptr->prev = NULL;
    }
    Listiter<Type> *_iter_ptr = (Listiter<Type> *) list_iter_ptr;
    while(_iter_ptr){
      if(_iter_ptr->next_node_ptr == NULL)
        _iter_ptr->next_node_ptr = ptr;
      _iter_ptr = _iter_ptr->nxt;
    }  
}

#endif
