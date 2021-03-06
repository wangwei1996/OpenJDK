/*
 * Copyright (c) 2007, 2013, Oracle and/or its affiliates. All rights reserved.
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
#include "gc_implementation/concurrentMarkSweep/cmsAdaptiveSizePolicy.hpp"
#include "gc_implementation/concurrentMarkSweep/cmsCollectorPolicy.hpp"
#include "gc_implementation/concurrentMarkSweep/cmsGCAdaptivePolicyCounters.hpp"
#include "gc_implementation/parNew/parNewGeneration.hpp"
#include "gc_implementation/shared/gcPolicyCounters.hpp"
#include "gc_implementation/shared/vmGCOperations.hpp"
#include "memory/cardTableRS.hpp"
#include "memory/collectorPolicy.hpp"
#include "memory/gcLocker.inline.hpp"
#include "memory/genCollectedHeap.hpp"
#include "memory/generationSpec.hpp"
#include "memory/space.hpp"
#include "memory/universe.hpp"
#include "runtime/arguments.hpp"
#include "runtime/globals_extension.hpp"
#include "runtime/handles.inline.hpp"
#include "runtime/java.hpp"
#include "runtime/thread.inline.hpp"
#include "runtime/vmThread.hpp"

// 日志
#include "wei_log/WeiLog.hpp"

//
// ConcurrentMarkSweepPolicy methods
//

void ConcurrentMarkSweepPolicy::initialize_alignments() {
  _space_alignment = _gen_alignment = (uintx)Generation::GenGrain;
  _heap_alignment = compute_heap_alignment();
}


/**
 * 初始化分代信息 
 * 
 * 即创建两个分代，并指定大小，没有实质申请内存
 */
void ConcurrentMarkSweepPolicy::initialize_generations() {
  wei_log_info(1,"正在执行 ConcurrentMarkSweepPolicy::initialize_generations 即初始化分代信息");

   // 创建两个指针(>> typedef GenerationSpec* GenerationSpecPtr;)
  _generations = NEW_C_HEAP_ARRAY3(GenerationSpecPtr, number_of_generations(),
                                   mtGC, 0, AllocFailStrategy::RETURN_NULL);
  if (_generations == NULL){
    vm_exit_during_initialization("Unable to allocate gen spec");
  } 

  // CMS是老年代收集器，需要搭配新生代收集器使用
  if (UseParNewGC) { //  ParNew + CMS的组合 ,重点
    if (UseAdaptiveSizePolicy) { // AdaptiveSizePolicy(自适应大小策略),非重点，看else
      _generations[0] = new GenerationSpec(Generation::ASParNew,
                                           _initial_gen0_size, _max_gen0_size);
    } else {
      // 初始化新生代空间
      _generations[0] = new GenerationSpec(Generation::ParNew,
                                           _initial_gen0_size, _max_gen0_size);
    }
  } else {
    _generations[0] = new GenerationSpec(Generation::DefNew, _initial_gen0_size,
                                         _max_gen0_size);
  }
  if (UseAdaptiveSizePolicy) {
    _generations[1] = new GenerationSpec(Generation::ASConcurrentMarkSweep,
                                         _initial_gen1_size, _max_gen1_size);
  } else {
     /**
      * 初始化老年代空间,但是注意，这里只是创建了GenerationSpec类型的对象，并设置堆名称和大小
      * 并没有实质分配内存。
      * 
      * Debug发现: 这里是先调用 CHeapObj 中被重载的new操作符，先分配空间: 这里的 “_generations[1]” 与 new操作符重载方法中的返回值"*p" 值是一致的
      */ 
    _generations[1] = new GenerationSpec(Generation::ConcurrentMarkSweep,
                                         _initial_gen1_size, _max_gen1_size);
  }

  if (_generations[0] == NULL || _generations[1] == NULL) {
    vm_exit_during_initialization("Unable to allocate gen spec");
  }

  wei_log_info(1,"ConcurrentMarkSweepPolicy::initialize_generations 初始化分代信息完成");
}

void ConcurrentMarkSweepPolicy::initialize_size_policy(size_t init_eden_size,
                                               size_t init_promo_size,
                                               size_t init_survivor_size) {
  double max_gc_minor_pause_sec = ((double) MaxGCMinorPauseMillis)/1000.0;
  double max_gc_pause_sec = ((double) MaxGCPauseMillis)/1000.0;
  _size_policy = new CMSAdaptiveSizePolicy(init_eden_size,
                                           init_promo_size,
                                           init_survivor_size,
                                           max_gc_minor_pause_sec,
                                           max_gc_pause_sec,
                                           GCTimeRatio);
}

void ConcurrentMarkSweepPolicy::initialize_gc_policy_counters() {
  // initialize the policy counters - 2 collectors, 3 generations
  if (UseParNewGC) {
    _gc_policy_counters = new GCPolicyCounters("ParNew:CMS", 2, 3);
  }
  else {
    _gc_policy_counters = new GCPolicyCounters("Copy:CMS", 2, 3);
  }
}

// Returns true if the incremental mode is enabled.
bool ConcurrentMarkSweepPolicy::has_soft_ended_eden()
{
  return CMSIncrementalMode;
}


//
// ASConcurrentMarkSweepPolicy methods
//

void ASConcurrentMarkSweepPolicy::initialize_gc_policy_counters() {

  assert(size_policy() != NULL, "A size policy is required");
  // initialize the policy counters - 2 collectors, 3 generations
  if (UseParNewGC) {
    _gc_policy_counters = new CMSGCAdaptivePolicyCounters("ParNew:CMS", 2, 3,
      size_policy());
  }
  else {
    _gc_policy_counters = new CMSGCAdaptivePolicyCounters("Copy:CMS", 2, 3,
      size_policy());
  }
}
