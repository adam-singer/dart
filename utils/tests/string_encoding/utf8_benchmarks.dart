#!/usr/bin/env dart
// Copyright (c) 2011, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#library("BenchmarkTests");
#import("../../string_encoding/utf8.dart", prefix: "SE");
#import("../../utf8/utf8.dart", prefix: "UT");
#source("benchmark_runner.dart");

void main() {

  final List<int> testEnglishUtf8 = const<int> [
      0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x69, 0x63,
      0x6b, 0x20, 0x62, 0x72, 0x6f, 0x77, 0x6e, 0x20,
      0x66, 0x6f, 0x78, 0x20, 0x6a, 0x75, 0x6d, 0x70,
      0x73, 0x20, 0x6f, 0x76, 0x65, 0x72, 0x20, 0x74,
      0x68, 0x65, 0x20, 0x6c, 0x61, 0x7a, 0x79, 0x20,
      0x64, 0x6f, 0x67, 0x2e];

  final List<int> testDanishUtf8 = const<int>[
      0x51, 0x75, 0x69, 0x7a, 0x64, 0x65, 0x6c, 0x74,
      0x61, 0x67, 0x65, 0x72, 0x6e, 0x65, 0x20, 0x73,
      0x70, 0x69, 0x73, 0x74, 0x65, 0x20, 0x6a, 0x6f,
      0x72, 0x64, 0x62, 0xc3, 0xa6, 0x72, 0x20, 0x6d,
      0x65, 0x64, 0x20, 0x66, 0x6c, 0xc3, 0xb8, 0x64,
      0x65, 0x20, 0x6d, 0x65, 0x6e, 0x73, 0x20, 0x63,
      0x69, 0x72, 0x6b, 0x75, 0x73, 0x6b, 0x6c, 0x6f,
      0x76, 0x6e, 0x65, 0x6e, 0x20, 0x57, 0x6f, 0x6c,
      0x74, 0x68, 0x65, 0x72, 0x20, 0x73, 0x70, 0x69,
      0x6c, 0x6c, 0x65, 0x64, 0x65, 0x20, 0x70, 0xc3,
      0xa5, 0x20, 0x78, 0x79, 0x6c, 0x6f, 0x66, 0x6f,
      0x6e, 0x2e];

  final List<int> testHebrewUtf8 = const<int>[
      0xd7, 0x93, 0xd7, 0x92, 0x20, 0xd7, 0xa1, 0xd7,
      0xa7, 0xd7, 0xa8, 0xd7, 0x9f, 0x20, 0xd7, 0xa9,
      0xd7, 0x98, 0x20, 0xd7, 0x91, 0xd7, 0x99, 0xd7,
      0x9d, 0x20, 0xd7, 0x9e, 0xd7, 0x90, 0xd7, 0x95,
      0xd7, 0x9b, 0xd7, 0x96, 0xd7, 0x91, 0x20, 0xd7,
      0x95, 0xd7, 0x9c, 0xd7, 0xa4, 0xd7, 0xaa, 0xd7,
      0xa2, 0x20, 0xd7, 0x9e, 0xd7, 0xa6, 0xd7, 0x90,
      0x20, 0xd7, 0x9c, 0xd7, 0x95, 0x20, 0xd7, 0x97,
      0xd7, 0x91, 0xd7, 0xa8, 0xd7, 0x94, 0x20, 0xd7,
      0x90, 0xd7, 0x99, 0xd7, 0x9a, 0x20, 0xd7, 0x94,
      0xd7, 0xa7, 0xd7, 0x9c, 0xd7, 0x99, 0xd7, 0x98,
      0xd7, 0x94];

  final List<int> testRussianUtf8 = const<int>[
      0xd0, 0xa1, 0xd1, 0x8a, 0xd0, 0xb5, 0xd1, 0x88,
      0xd1, 0x8c, 0x20, 0xd0, 0xb6, 0xd0, 0xb5, 0x20,
      0xd0, 0xb5, 0xd1, 0x89, 0xd1, 0x91, 0x20, 0xd1,
      0x8d, 0xd1, 0x82, 0xd0, 0xb8, 0xd1, 0x85, 0x20,
      0xd0, 0xbc, 0xd1, 0x8f, 0xd0, 0xb3, 0xd0, 0xba,
      0xd0, 0xb8, 0xd1, 0x85, 0x20, 0xd1, 0x84, 0xd1,
      0x80, 0xd0, 0xb0, 0xd0, 0xbd, 0xd1, 0x86, 0xd1,
      0x83, 0xd0, 0xb7, 0xd1, 0x81, 0xd0, 0xba, 0xd0,
      0xb8, 0xd1, 0x85, 0x20, 0xd0, 0xb1, 0xd1, 0x83,
      0xd0, 0xbb, 0xd0, 0xbe, 0xd0, 0xba, 0x20, 0xd0,
      0xb4, 0xd0, 0xb0, 0x20, 0xd0, 0xb2, 0xd1, 0x8b,
      0xd0, 0xbf, 0xd0, 0xb5, 0xd0, 0xb9, 0x20, 0xd1,
      0x87, 0xd0, 0xb0, 0xd1, 0x8e];

  final List<int> testGreekUtf8 = const<int>[
      0xce, 0x93, 0xce, 0xb1, 0xce, 0xb6, 0xce, 0xad,
      0xce, 0xb5, 0xcf, 0x82, 0x20, 0xce, 0xba, 0xce,
      0xb1, 0xe1, 0xbd, 0xb6, 0x20, 0xce, 0xbc, 0xcf,
      0x85, 0xcf, 0x81, 0xcf, 0x84, 0xce, 0xb9, 0xe1,
      0xbd, 0xb2, 0xcf, 0x82, 0x20, 0xce, 0xb4, 0xe1,
      0xbd, 0xb2, 0xce, 0xbd, 0x20, 0xce, 0xb8, 0xe1,
      0xbd, 0xb0, 0x20, 0xce, 0xb2, 0xcf, 0x81, 0xe1,
      0xbf, 0xb6, 0x20, 0xcf, 0x80, 0xce, 0xb9, 0xe1,
      0xbd, 0xb0, 0x20, 0xcf, 0x83, 0xcf, 0x84, 0xe1,
      0xbd, 0xb8, 0x20, 0xcf, 0x87, 0xcf, 0x81, 0xcf,
      0x85, 0xcf, 0x83, 0xce, 0xb1, 0xcf, 0x86, 0xe1,
      0xbd, 0xb6, 0x20, 0xce, 0xbe, 0xce, 0xad, 0xcf,
      0x86, 0xcf, 0x89, 0xcf, 0x84, 0xce, 0xbf];

  final List<int> testKatakanaUtf8 = const<int>[
      0xe3, 0x82, 0xa4, 0xe3, 0x83, 0xad, 0xe3, 0x83,
      0x8f, 0xe3, 0x83, 0x8b, 0xe3, 0x83, 0x9b, 0xe3,
      0x83, 0x98, 0xe3, 0x83, 0x88, 0x20, 0xe3, 0x83,
      0x81, 0xe3, 0x83, 0xaa, 0xe3, 0x83, 0x8c, 0xe3,
      0x83, 0xab, 0xe3, 0x83, 0xb2, 0x20, 0xe3, 0x83,
      0xaf, 0xe3, 0x82, 0xab, 0xe3, 0x83, 0xa8, 0xe3,
      0x82, 0xbf, 0xe3, 0x83, 0xac, 0xe3, 0x82, 0xbd,
      0x20, 0xe3, 0x83, 0x84, 0xe3, 0x83, 0x8d, 0xe3,
      0x83, 0x8a, 0xe3, 0x83, 0xa9, 0xe3, 0x83, 0xa0,
      0x0a, 0xe3, 0x82, 0xa6, 0xe3, 0x83, 0xb0, 0xe3,
      0x83, 0x8e, 0xe3, 0x82, 0xaa, 0xe3, 0x82, 0xaf,
      0xe3, 0x83, 0xa4, 0xe3, 0x83, 0x9e, 0x20, 0xe3,
      0x82, 0xb1, 0xe3, 0x83, 0x95, 0xe3, 0x82, 0xb3,
      0xe3, 0x82, 0xa8, 0xe3, 0x83, 0x86, 0x20, 0xe3,
      0x82, 0xa2, 0xe3, 0x82, 0xb5, 0xe3, 0x82, 0xad,
      0xe3, 0x83, 0xa6, 0xe3, 0x83, 0xa1, 0xe3, 0x83,
      0x9f, 0xe3, 0x82, 0xb7, 0x20, 0xe3, 0x83, 0xb1,
      0xe3, 0x83, 0x92, 0xe3, 0x83, 0xa2, 0xe3, 0x82,
      0xbb, 0xe3, 0x82, 0xb9, 0xe3, 0x83, 0xb3];

  TimedTestConfig testConfig_1sec =
      new TimedTestConfig(100, 1 * 1000, blocksize: 1000);

  BenchmarkRunner.runTimed("SE_EN1","string_encoding/decodeUtf8-English1",
      testConfig_1sec, () =>
          (new SE.Utf8Decoder(testEnglishUtf8)).decodeRest());

  BenchmarkRunner.runTimed("SE_DA1","string_encoding/decodeUtf8-Danish1",
      testConfig_1sec, () => (new SE.Utf8Decoder(testDanishUtf8)).decodeRest());

  BenchmarkRunner.runTimed("SE_HE1","string_encoding/decodeUtf8-Hebrew1",
      testConfig_1sec, () => (new SE.Utf8Decoder(testHebrewUtf8)).decodeRest());

  BenchmarkRunner.runTimed("SE_RU1","string_encoding/decodeUtf8-Russian1",
      testConfig_1sec, () =>
          (new SE.Utf8Decoder(testRussianUtf8)).decodeRest());

  BenchmarkRunner.runTimed("SE_EL1","string_encoding/decodeUtf8-Greek",
      testConfig_1sec, () =>
          (new SE.Utf8Decoder(testGreekUtf8)).decodeRest());

  BenchmarkRunner.runTimed("SE_JA1","string_encoding/decodeUtf8-Katakana",
      testConfig_1sec, () =>
          (new SE.Utf8Decoder(testKatakanaUtf8)).decodeRest());

  BenchmarkRunner.runTimed("UT_EN1","Utf8Decoder/decodeUtf8-English1",
      testConfig_1sec, () =>
          (new UT.Utf8Decoder(testEnglishUtf8, 0, testEnglishUtf8.length))
          .decodeRest());

  BenchmarkRunner.runTimed("UT_DA1","Utf8Decoder/decodeUtf8-Danish1",
      testConfig_1sec, () =>
          (new UT.Utf8Decoder(testDanishUtf8, 0, testDanishUtf8.length))
          .decodeRest());

  BenchmarkRunner.runTimed("UT_HE1","Utf8Decoder/decodeUtf8-Hebrew1",
      testConfig_1sec, () =>
          (new UT.Utf8Decoder(testHebrewUtf8, 0, testHebrewUtf8.length))
          .decodeRest());

  BenchmarkRunner.runTimed("UT_RU1","Utf8Decoder/decodeUtf8-Russian1",
      testConfig_1sec,
          () => (new UT.Utf8Decoder(testRussianUtf8, 0, testRussianUtf8.length))
          .decodeRest());

  BenchmarkRunner.runTimed("UT_EL1","Utf8Decoder/decodeUtf8-Greek",
      testConfig_1sec, () =>
          (new UT.Utf8Decoder(testGreekUtf8, 0, testGreekUtf8.length))
          .decodeRest());

  BenchmarkRunner.runTimed("UT_JA1","Utf8Decoder/decodeUtf8-Katakana",
      testConfig_1sec,() =>
          (new UT.Utf8Decoder(testKatakanaUtf8, 0, testKatakanaUtf8.length))
          .decodeRest());
}

