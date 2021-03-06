/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "shared_object_internal.hpp"
#include "sparse_storage_impl.hpp"
#include <typeinfo>

using namespace std;
namespace casadi {

  // Instantiate templates
  template class SparseStorage<WeakRef>;

  SharedObject::SharedObject() {
    node = 0;
  }

  SharedObject::SharedObject(const SharedObject& ref) {
    node = ref.node;
    count_up();
  }

  SharedObject::~SharedObject() {
    count_down();
  }

  void SharedObject::assignNode(SharedObjectInternal* node_) {
    count_down();
    node = node_;
    count_up();
  }

  void SharedObject::assignNodeNoCount(SharedObjectInternal* node_) {
    node = node_;
  }

  SharedObject& SharedObject::operator=(const SharedObject& ref) {
    // quick return if the old and new pointers point to the same object
    if (node == ref.node) return *this;

    // decrease the counter and delete if this was the last pointer
    count_down();

    // save the new pointer
    node = ref.node;
    count_up();
    return *this;
  }

  SharedObjectInternal* SharedObject::get() const {
    return node;
  }

  bool SharedObject::is_null() const {
    return node==0;
  }

  void SharedObject::count_up() {
    if (node) node->count++;
  }

  void SharedObject::count_down() {
    if (node && --node->count == 0) {
      delete node;
      node = 0;
    }
  }

  SharedObjectInternal* SharedObject::operator->() const {
    casadi_assert(!is_null());
    return node;
  }

  void SharedObject::repr(std::ostream &stream, bool trailing_newline) const {
    if (is_null()) {
      stream << 0;
    } else {
      (*this)->repr(stream);
    }
    if (trailing_newline) stream << std::endl;
  }

  void SharedObject::print(std::ostream &stream, bool trailing_newline) const {
    if (is_null()) {
      stream << "Null pointer of class \"" << typeid(this).name() << "\"";
    } else {
      (*this)->print(stream);
    }
    if (trailing_newline) stream << std::endl;
  }

  void SharedObject::printPtr(std::ostream &stream) const {
    stream << node;
  }

  void SharedObject::swap(SharedObject& other) {
    SharedObject temp = *this;
    *this = other;
    other = temp;
  }

  int SharedObject::getCount() const {
    return (*this)->getCount();
  }

  WeakRef* SharedObject::weak() {
    return (*this)->weak();
  }

  size_t SharedObject::__hash__() const {
    return reinterpret_cast<size_t>(get());
  }

  WeakRef::WeakRef(int dummy) {
    casadi_assert(dummy==0);
  }

  bool WeakRef::alive() const {
    return !is_null() && (*this)->raw_ != 0;
  }

  SharedObject WeakRef::shared() {
    SharedObject ret;
    if (alive()) {
      ret.assignNode((*this)->raw_);
    }
    return ret;
  }

  const WeakRefInternal* WeakRef::operator->() const {
    return static_cast<const WeakRefInternal*>(SharedObject::operator->());
  }

  WeakRefInternal* WeakRef::operator->() {
    return static_cast<WeakRefInternal*>(SharedObject::operator->());
  }

  WeakRef::WeakRef(SharedObject shared) {
    assignNode(shared.weak()->get());
  }

  WeakRef::WeakRef(SharedObjectInternal* raw) {
    assignNode(new WeakRefInternal(raw));
  }

  void WeakRef::kill() {
    (*this)->raw_ = 0;
  }

} // namespace casadi
