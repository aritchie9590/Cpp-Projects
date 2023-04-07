// Implementation of the templated Vector class
// ECE4893/8893 lab 3
// Alex Ritchie

#include <iostream> // debugging
#include <stdlib.h>
#include <sstream>

#include "Vector.h"
#include "String.h"

// Your implementation here
// Fill in all the necessary functions below
using namespace std;

// Default constructor
template <typename T>
Vector<T>::Vector()
{
  elements = NULL;
  count = 0;
  reserved = 0;
}

// Copy constructor
template <typename T>
Vector<T>::Vector(const Vector& rhs)
{
  count = rhs.count;
  reserved = rhs.reserved;
  
  // deep copy the elements member
  T* temp = (T*) malloc (reserved * sizeof(T));
  
  for (int i = 0; i<100; i++){
    cout << rhs.elements[i] << endl; // prints all the correct values
  }
  
  for (int i = 0; i < count; i++){
    cout << "here is v[i] = " << rhs.elements[i] << endl; // prints nothing at i = 63
    new (&temp[i]) T(rhs.elements[i]); // when I comment this line out the above line prints correctly
  }
  elements = temp;
}

// Assignment operator
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
  // if elements isn't already empty we have to empty it
  // before assigning anything to it
  if (elements != NULL)
  {
    for(int i = 0; i < count; i++){
      elements[i].~T();
    }
    free(elements);
  }

  // now that all the memory in elements has been freed
  // we can copy everything over
  count = rhs.count;
  reserved = rhs.reserved;
  elements = (T*) malloc (reserved * sizeof(T));
  for(int i = 0; i < count; i++)
  {
    new (elements + i) T(rhs.elements[i]);
  }
}

#ifdef GRAD_STUDENT
// Other constructors
template <typename T>
Vector<T>::Vector(size_t nReserved)
{ 
  // Initialize with reserved memory
  count = 0;
  reserved = nReserved;
  elements = (T*) malloc (nReserved * sizeof(T));
}

template <typename T>
Vector<T>::Vector(size_t n, const T& t)
{ 
  // Initialize with "n" copies of "t"
  count = n;
  reserved = n;
  elements = (T*) malloc (n * sizeof(T));
  for(int i = 0; i < count; i++)
  {
    new (elements + i) T(t);
  }
}


// possible error here
// may need to copy all the elements again
template <typename T>
void Vector<T>::Reserve(size_t n)
{ 
  // Reserve extra memory
  int oldReserved = reserved;
  int newReserved = oldReserved + n;
  reserved += n;
  // copy everything that was already there into the new memory
  T* temp = (T*) malloc (reserved * sizeof(T));
  for (int i = 0; i < count; i++){
    new (temp + i) T(elements[i]);
    elements[i].~T();
  }
  free(elements);
  elements = temp;
}

#endif

// Destructor
template <typename T>
Vector<T>::~Vector()
{

  if (elements != NULL)
  {
    for(int i = 0; i < count; i++){
      elements[i].~T();
    }
    free(elements);
  }

}

// Add and access front and back
template <typename T>
void Vector<T>::Push_Back(const T& rhs)
{
  // check if we have any unused reserved memory
  // if we don't add another space
  if (reserved == count){
    Reserve(1);
  }
  
  new (elements + count) T(rhs);
  count++;
}

template <typename T>
void Vector<T>::Push_Front(const T& rhs)
{
  T* temp = (T*) malloc ((count + 1) * sizeof(T));
  new (temp) T(rhs);
  for (int i = 0; i < count; i++){
    new (temp + i + 1) T(elements[i]);
    elements[i].~T();
  }
  count++;
  free(elements);
  elements = temp;
}

template <typename T>
void Vector<T>::Pop_Back()
{ 
  // Remove last element
  elements[count - 1].~T();
  count--;

}

template <typename T>
void Vector<T>::Pop_Front()
{ 
  // Remove the first element
  for (int i = 0; i < count - 1; i++){
    elements[i].~T();
    new (elements + i)  T(elements[i+1]);
  }
  elements[count - 1].~T();
  count--;
}

// Element Access
template <typename T>
T& Vector<T>::Front() const
{
  return elements[0];
}

// Element Access
template <typename T>
T& Vector<T>::Back() const
{
  return elements[count - 1];
}

template <typename T>
const T& Vector<T>::operator[](size_t i) const
{ // const element access
  if (i < 0 || i >= count)
  {
    cout << "Index Out of Bounds" << endl;
    abort();
  }
  return *(elements + i);
}

template <typename T>
T& Vector<T>::operator[](size_t i)
{
  // no idea how this is supposed to be any different
  //nonconst element access
  if (i < 0 || i >= count)
  {
    cout << "Index Out of Bounds" << endl;
    abort();
  }
  return *(elements + i);


}

template <typename T>
size_t Vector<T>::Size() const
{
  return count;
}


template <typename T>
bool Vector<T>::Empty() const
{
  if (count == 0)
      return true;
  else
      return false;
}

// Implement clear
template <typename T>
void Vector<T>::Clear()
{
  // same as destructor but don't free the memory
  for(int i = 0; i < count; i++){
    elements[i].~T();
  }
  count = 0;
}

// Iterator access functions
template <typename T>
VectorIterator<T> Vector<T>::Begin() const
{
  return VectorIterator<T>(elements);
}

template <typename T>
VectorIterator<T> Vector<T>::End() const
{
  return VectorIterator<T>(elements + count);
}

#ifdef GRAD_STUDENT
// Erase and insert
template <typename T>
void Vector<T>::Erase(const VectorIterator<T>& it)
{
  // just use pointer arithmetic to erase the element
  // with the given pointer and then scoot everything 
  // forward one position
  int dist = it.current - elements;
  if ( (dist >= 0) && (dist <= count) && (count != 0) ){
    elements[dist].~T();
    for (int i = dist; i < count - 1; i++){
      new (elements + i) T(elements[i + 1]);
      elements[i + 1].~T();
    }
    count--;
  } 
}

template <typename T>
void Vector<T>::Insert(const T& rhs, const VectorIterator<T>& it)
{
  // just use pointer arithmetic to find the position for insertion
  // reserve another space if there is no extra reserved space,
  // then shift everything after the given position one to the right,
  // then slide the new element in the proper position
  int dist = it.current - elements;
  if ( (dist >= 0) && (dist <= count) ){
    if ( reserved == count ) Reserve(1);  
    
    //elements[dist].~T();
    for (int i = count; i > dist; i--){
      new (elements + i) T(elements[i - 1]);
      elements[i - 1].~T();
    }
    new (elements + dist) T(rhs);
    count++;
  }  
}
#endif

// Implement the iterators

// Constructors
template <typename T>
VectorIterator<T>::VectorIterator()
{
  current = NULL;
}

template <typename T>
VectorIterator<T>::VectorIterator(T* c)
{
  current = c;
}

// Copy constructor
template <typename T>
VectorIterator<T>::VectorIterator(const VectorIterator<T>& rhs)
{
  current = rhs.current;
}

// Iterator defeferencing operator
template <typename T>
T& VectorIterator<T>::operator*() const
{
  return *current;
}

// Prefix increment
template <typename T>
VectorIterator<T>  VectorIterator<T>::operator++()
{
  current = current + 1;
  return *this;
}

// Postfix increment
template <typename T>
VectorIterator<T> VectorIterator<T>::operator++(int)
{
  VectorIterator<T> copy = *this;
  current = current + 1;
  return copy;
}

// Comparison operators
template <typename T>
bool VectorIterator<T>::operator !=(const VectorIterator<T>& rhs) const
{
  return (current != rhs.current);
}

template <typename T>
bool VectorIterator<T>::operator ==(const VectorIterator<T>& rhs) const
{
  return (current == rhs.current);
}




