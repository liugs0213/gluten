/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <cstdlib>

#include "memory/HbwAllocator.h"
#include "memory/MemoryAllocator.h"

class TestHbwAllocator : public ::testing::Test {
 protected:
  void CheckBytesAndFree(void*& buf, int64_t size) {
    ASSERT_NE(buf, nullptr);
    ASSERT_EQ(allocator->GetBytes(), size);
    allocator->Free(buf, size);
    ASSERT_EQ(allocator->GetBytes(), 0);
    buf = nullptr;
  }

  std::shared_ptr<gluten::MemoryAllocator> allocator;
};

class TestHbwAllocatorEnabled : public TestHbwAllocator {
 protected:
  static void SetUpTestSuite() {
    setenv("MEMKIND_HBW_NODES", "0", 1);
  }

  TestHbwAllocatorEnabled() {
    allocator = gluten::DefaultMemoryAllocator();
  }
};

class TestHbwAllocatorDisabled : public TestHbwAllocator {
 protected:
  static void SetUpTestSuite() {
    unsetenv("MEMKIND_HBW_NODES");
  }

  TestHbwAllocatorDisabled() {
    allocator = gluten::DefaultMemoryAllocator();
  }
};

TEST_F(TestHbwAllocatorEnabled, TestHbwEnabled) {
  auto ptr = std::dynamic_pointer_cast<gluten::HbwMemoryAllocator>(allocator);
  ASSERT_NE(ptr, nullptr);
}

TEST_F(TestHbwAllocatorDisabled, TestHbwDisabled) {
  unsetenv("MEMKIND_HBW_NODES");
  allocator = gluten::DefaultMemoryAllocator();
  auto ptr = std::dynamic_pointer_cast<gluten::StdMemoryAllocator>(allocator);
  ASSERT_NE(ptr, nullptr);
}

TEST_F(TestHbwAllocatorEnabled, TestAllocateHbm) {
  setenv("MEMKIND_HBW_NODES", "0", 1);
  allocator = gluten::DefaultMemoryAllocator();

  const size_t size = 1024 * 1024; // 1M of data
  void* buf = nullptr;

  allocator->Allocate(size, &buf);
  CheckBytesAndFree(buf, size);

  allocator->AllocateAligned(64, size, &buf);
  CheckBytesAndFree(buf, size);

  allocator->AllocateZeroFilled(1, size, &buf);
  CheckBytesAndFree(buf, size);

  allocator->Allocate(size, &buf);
  allocator->Reallocate(buf, size, size << 1, &buf);
  CheckBytesAndFree(buf, size << 1);

  allocator->Allocate(size, &buf);
  allocator->ReallocateAligned(buf, 64, size, size << 1, &buf);
  CheckBytesAndFree(buf, size << 1);
}
