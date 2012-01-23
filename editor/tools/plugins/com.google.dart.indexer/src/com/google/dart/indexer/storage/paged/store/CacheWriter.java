/*
 * Copyright (c) 2011, the Dart project authors.
 * 
 * Licensed under the Eclipse Public License v1.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * 
 * http://www.eclipse.org/legal/epl-v10.html
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */
package com.google.dart.indexer.storage.paged.store;

import com.google.dart.indexer.pagedstorage.exceptions.PagedStorageException;

/**
 * The cache writer is called by the cache to persist changed data that needs to be removed from the
 * cache.
 */
public interface CacheWriter {
  /**
   * Persist a record.
   * 
   * @param entry the cache entry
   */
  void writeBack(CacheObject entry) throws PagedStorageException;
}
