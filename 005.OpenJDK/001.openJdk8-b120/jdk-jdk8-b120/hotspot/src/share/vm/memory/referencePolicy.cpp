/*
 * Copyright (c) 2000, 2011, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "classfile/javaClasses.hpp"
#include "memory/referencePolicy.hpp"
#include "memory/universe.hpp"
#include "runtime/arguments.hpp"
#include "runtime/globals.hpp"

LRUCurrentHeapPolicy::LRUCurrentHeapPolicy() {
  setup();
}

/**
 * LRUCurrentHeapPolicy 与 LRUMaxHeapPolicy 的不同在于setup方法的逻辑
 * 即： 
 *   在LRUCurrentHeapPolicy中，_max_interval计算 = 上次GC时堆空间的容量 - 上次GC后堆空间的使用值
 *   在LRUMaxHeapPolicy中，_max_interval计算 = 堆的最大值(虚拟地址) - 上次GC后堆空间的使用值
 *   
 * 因此，在LRUMaxHeapPolicy中，“引用”对象"可能"(当物理内存与虚拟内存等大的时候，存活的时间就一样了)存活的更久.
 */

// Capture state (of-the-VM) information needed to evaluate the policy
void LRUCurrentHeapPolicy::setup() {
   // LRU: Least Recently Used的缩写，即最近最少使用
   // SoftRefLRUPolicyMSPerMB  定义于globals.hpp，值为1000，意为Number of milliseconds per MB of free space in the heap（即 堆中每MB空闲空间允许存活的毫秒数）
  _max_interval = (Universe::get_heap_free_at_last_gc() / M) * SoftRefLRUPolicyMSPerMB;
  assert(_max_interval >= 0,"Sanity check");
}

// The oop passed in is the SoftReference object, and not
// the object the SoftReference points to.
bool LRUCurrentHeapPolicy::should_clear_reference(oop p,
                                                  jlong timestamp_clock) {

                                                     /**
   * timestamp_clock:  java.lang.ref.SoftReference#clock , 即上次GC的时间
   * java_lang_ref_SoftReference::timestamp(p): java.lang.ref.SoftReference#timestamp , 即最近使用的时间
   */ 
  jlong interval = timestamp_clock - java_lang_ref_SoftReference::timestamp(p);
  assert(interval >= 0, "Sanity check");

  // The interval will be zero if the ref was accessed since the last scavenge/gc.
  // // 如果上次清除/gc之后访问了ref，那么时间间隔将为零
  if(interval <= _max_interval) {
    return false;
  }

  return true;
}

/////////////////////// MaxHeap //////////////////////

LRUMaxHeapPolicy::LRUMaxHeapPolicy() {
  setup();
}

// Capture state (of-the-VM) information needed to evaluate the policy
void LRUMaxHeapPolicy::setup() {
  // 获取最大堆大小
  size_t max_heap = MaxHeapSize;

  // 从最大堆中减去上次GC时使用的堆的大小
  max_heap -= Universe::get_heap_used_at_last_gc();
  
  // 单位转换，即转换为MB
  max_heap /= M;

   // LRU: Least Recently Used的缩写，即最近最少使用
   // SoftRefLRUPolicyMSPerMB  定义于globals.hpp，值为1000，意为Number of milliseconds per MB of free space in the heap（即 堆中每MB空闲空间允许存活的毫秒数）
  _max_interval = max_heap * SoftRefLRUPolicyMSPerMB;
  assert(_max_interval >= 0,"Sanity check");
}

// The oop passed in is the SoftReference object, and not
// the object the SoftReference points to.
bool LRUMaxHeapPolicy::should_clear_reference(oop p,
                                             jlong timestamp_clock) {
  jlong interval = timestamp_clock - java_lang_ref_SoftReference::timestamp(p);
  assert(interval >= 0, "Sanity check");

  // The interval will be zero if the ref was accessed since the last scavenge/gc.
  if(interval <= _max_interval) {
    return false;
  }

  return true;
}
