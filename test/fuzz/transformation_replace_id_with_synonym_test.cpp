// Copyright (c) 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/fuzz/transformation_replace_id_with_synonym.h"
#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/id_use_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

// The following shader was obtained from this GLSL, which was then optimized
// with spirv-opt -O and manually edited to include some uses of OpCopyObject
// (to introduce id synonyms).
//
// #version 310 es
//
// precision highp int;
// precision highp float;
//
// layout(set = 0, binding = 0) uniform buf {
//   int a;
//   int b;
//   int c;
// };
//
// layout(location = 0) out vec4 color;
//
// void main() {
//   int x = a;
//   float f = 0.0;
//   while (x < b) {
//     switch(x % 4) {
//       case 0:
//         color[0] = f;
//         break;
//       case 1:
//         color[1] = f;
//         break;
//       case 2:
//         color[2] = f;
//         break;
//       case 3:
//         color[3] = f;
//         break;
//       default:
//         break;
//     }
//     if (x > c) {
//       x++;
//     } else {
//       x += 2;
//     }
//   }
//   color[0] += color[1] + float(x);
// }
const std::string kComplexShader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %42
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "buf"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpMemberName %9 2 "c"
               OpName %11 ""
               OpName %42 "color"
               OpMemberDecorate %9 0 Offset 0
               OpMemberDecorate %9 1 Offset 4
               OpMemberDecorate %9 2 Offset 8
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpDecorate %42 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6 %6 %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpConstant %6 0
         %13 = OpTypePointer Uniform %6
         %16 = OpTypeFloat 32
         %19 = OpConstant %16 0
         %26 = OpConstant %6 1
         %29 = OpTypeBool
         %32 = OpConstant %6 4
         %40 = OpTypeVector %16 4
         %41 = OpTypePointer Output %40
         %42 = OpVariable %41 Output
         %44 = OpTypeInt 32 0
         %45 = OpConstant %44 0
         %46 = OpTypePointer Output %16
         %50 = OpConstant %44 1
         %54 = OpConstant %44 2
         %58 = OpConstant %44 3
         %64 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %209 = OpCopyObject %6 %12
         %14 = OpAccessChain %13 %11 %12
         %15 = OpLoad %6 %14
        %200 = OpCopyObject %6 %15
               OpBranch %20
         %20 = OpLabel
         %84 = OpPhi %6 %15 %5 %86 %69
         %27 = OpAccessChain %13 %11 %26
         %28 = OpLoad %6 %27
        %207 = OpCopyObject %6 %84
        %201 = OpCopyObject %6 %15
         %30 = OpSLessThan %29 %84 %28
               OpLoopMerge %22 %69 None
               OpBranchConditional %30 %21 %22
         %21 = OpLabel
         %33 = OpSMod %6 %84 %32
        %208 = OpCopyObject %6 %33
               OpSelectionMerge %39 None
               OpSwitch %33 %38 0 %34 1 %35 2 %36 3 %37
         %38 = OpLabel
        %202 = OpCopyObject %6 %15
               OpBranch %39
         %34 = OpLabel
        %210 = OpCopyObject %16 %19
         %47 = OpAccessChain %46 %42 %45
               OpStore %47 %19
               OpBranch %39
         %35 = OpLabel
         %51 = OpAccessChain %46 %42 %50
               OpStore %51 %19
               OpBranch %39
         %36 = OpLabel
        %204 = OpCopyObject %44 %54
         %55 = OpAccessChain %46 %42 %54
        %203 = OpCopyObject %46 %55
               OpStore %55 %19
               OpBranch %39
         %37 = OpLabel
         %59 = OpAccessChain %46 %42 %58
               OpStore %59 %19
               OpBranch %39
         %39 = OpLabel
        %300 = OpIAdd %6 %15 %15
         %65 = OpAccessChain %13 %11 %64
         %66 = OpLoad %6 %65
         %67 = OpSGreaterThan %29 %84 %66
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %72
         %68 = OpLabel
         %71 = OpIAdd %6 %84 %26
               OpBranch %69
         %72 = OpLabel
         %74 = OpIAdd %6 %84 %64
        %205 = OpCopyObject %6 %74
               OpBranch %69
         %69 = OpLabel
         %86 = OpPhi %6 %71 %68 %74 %72
        %301 = OpPhi %6 %71 %68 %15 %72
               OpBranch %20
         %22 = OpLabel
         %75 = OpAccessChain %46 %42 %50
         %76 = OpLoad %16 %75
         %78 = OpConvertSToF %16 %84
         %80 = OpAccessChain %46 %42 %45
        %206 = OpCopyObject %16 %78
         %81 = OpLoad %16 %80
         %79 = OpFAdd %16 %76 %78
         %82 = OpFAdd %16 %81 %79
               OpStore %80 %82
               OpReturn
               OpFunctionEnd
)";

protobufs::Fact MakeSynonymFact(uint32_t id, uint32_t synonym_object,
                                std::vector<uint32_t> indices = {}) {
  protobufs::FactIdSynonym id_synonym_fact;
  id_synonym_fact.set_id(id);
  id_synonym_fact.mutable_data_descriptor()->set_object(synonym_object);
  for (auto index : indices) {
    id_synonym_fact.mutable_data_descriptor()->add_index(index);
  }
  protobufs::Fact result;
  *result.mutable_id_synonym_fact() = id_synonym_fact;
  return result;
}

// Equips the fact manager with synonym facts for the above shader.
void SetUpIdSynonyms(FactManager* fact_manager, opt::IRContext* context) {
  fact_manager->AddFact(MakeSynonymFact(15, 200), context);
  fact_manager->AddFact(MakeSynonymFact(15, 201), context);
  fact_manager->AddFact(MakeSynonymFact(15, 202), context);
  fact_manager->AddFact(MakeSynonymFact(55, 203), context);
  fact_manager->AddFact(MakeSynonymFact(54, 204), context);
  fact_manager->AddFact(MakeSynonymFact(74, 205), context);
  fact_manager->AddFact(MakeSynonymFact(78, 206), context);
  fact_manager->AddFact(MakeSynonymFact(84, 207), context);
  fact_manager->AddFact(MakeSynonymFact(33, 208), context);
  fact_manager->AddFact(MakeSynonymFact(12, 209), context);
  fact_manager->AddFact(MakeSynonymFact(19, 210), context);
}

TEST(TransformationReplaceIdWithSynonymTest, IllegalTransformations) {
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, kComplexShader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  SetUpIdSynonyms(&fact_manager, context.get());

  // %202 cannot replace %15 as in-operand 0 of %300, since %202 does not
  // dominate %300.
  auto synonym_does_not_dominate_use = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(15, SpvOpIAdd, 0, 300, 0),
      MakeDataDescriptor(202, {}), 0);
  ASSERT_FALSE(
      synonym_does_not_dominate_use.IsApplicable(context.get(), fact_manager));

  // %202 cannot replace %15 as in-operand 2 of %301, since this is the OpPhi's
  // incoming value for block %72, and %202 does not dominate %72.
  auto synonym_does_not_dominate_use_op_phi =
      TransformationReplaceIdWithSynonym(
          transformation::MakeIdUseDescriptor(15, SpvOpPhi, 2, 301, 0),
          MakeDataDescriptor(202, {}), 0);
  ASSERT_FALSE(synonym_does_not_dominate_use_op_phi.IsApplicable(context.get(),
                                                                 fact_manager));

  // %200 is not a synonym for %84
  auto id_in_use_is_not_synonymous = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(84, SpvOpSGreaterThan, 0, 67, 0),
      MakeDataDescriptor(200, {}), 0);
  ASSERT_FALSE(
      id_in_use_is_not_synonymous.IsApplicable(context.get(), fact_manager));

  // %86 is not a synonym for anything (and in particular not for %74)
  auto id_has_no_synonyms = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(86, SpvOpPhi, 2, 84, 0),
      MakeDataDescriptor(74, {}), 0);
  ASSERT_FALSE(id_has_no_synonyms.IsApplicable(context.get(), fact_manager));

  // This would lead to %207 = 'OpCopyObject %type %207' if it were allowed
  auto synonym_use_is_in_synonym_definition =
      TransformationReplaceIdWithSynonym(
          transformation::MakeIdUseDescriptor(84, SpvOpCopyObject, 0, 207, 0),
          MakeDataDescriptor(207, {}), 0);
  ASSERT_FALSE(synonym_use_is_in_synonym_definition.IsApplicable(context.get(),
                                                                 fact_manager));

  // The id use descriptor does not lead to a use (%84 is not used in the
  // definition of %207)
  auto bad_id_use_descriptor = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(84, SpvOpCopyObject, 0, 200, 0),
      MakeDataDescriptor(207, {}), 0);
  ASSERT_FALSE(bad_id_use_descriptor.IsApplicable(context.get(), fact_manager));

  // This replacement would lead to an access chain into a struct using a
  // non-constant index.
  auto bad_access_chain = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(12, SpvOpAccessChain, 1, 14, 0),
      MakeDataDescriptor(209, {}), 0);
  ASSERT_FALSE(bad_access_chain.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationReplaceIdWithSynonymTest, LegalTransformations) {
  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context =
      BuildModule(env, consumer, kComplexShader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  SetUpIdSynonyms(&fact_manager, context.get());

  auto global_constant_synonym = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(19, SpvOpStore, 1, 47, 0),
      MakeDataDescriptor(210, {}), 0);
  ASSERT_TRUE(
      global_constant_synonym.IsApplicable(context.get(), fact_manager));
  global_constant_synonym.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  auto replace_vector_access_chain_index = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(54, SpvOpAccessChain, 1, 55, 0),
      MakeDataDescriptor(204, {}), 0);
  ASSERT_TRUE(replace_vector_access_chain_index.IsApplicable(context.get(),
                                                             fact_manager));
  replace_vector_access_chain_index.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // This is an interesting case because it replaces something that is being
  // copied with something that is already a synonym.
  auto regular_replacement = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(15, SpvOpCopyObject, 0, 202, 0),
      MakeDataDescriptor(201, {}), 0);
  ASSERT_TRUE(regular_replacement.IsApplicable(context.get(), fact_manager));
  regular_replacement.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  auto regular_replacement2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(55, SpvOpStore, 0, 203, 0),
      MakeDataDescriptor(203, {}), 0);
  ASSERT_TRUE(regular_replacement2.IsApplicable(context.get(), fact_manager));
  regular_replacement2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  auto good_op_phi = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(74, SpvOpPhi, 2, 86, 0),
      MakeDataDescriptor(205, {}), 0);
  ASSERT_TRUE(good_op_phi.IsApplicable(context.get(), fact_manager));
  good_op_phi.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %42
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "buf"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpMemberName %9 2 "c"
               OpName %11 ""
               OpName %42 "color"
               OpMemberDecorate %9 0 Offset 0
               OpMemberDecorate %9 1 Offset 4
               OpMemberDecorate %9 2 Offset 8
               OpDecorate %9 Block
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 0
               OpDecorate %42 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpTypeStruct %6 %6 %6
         %10 = OpTypePointer Uniform %9
         %11 = OpVariable %10 Uniform
         %12 = OpConstant %6 0
         %13 = OpTypePointer Uniform %6
         %16 = OpTypeFloat 32
         %19 = OpConstant %16 0
         %26 = OpConstant %6 1
         %29 = OpTypeBool
         %32 = OpConstant %6 4
         %40 = OpTypeVector %16 4
         %41 = OpTypePointer Output %40
         %42 = OpVariable %41 Output
         %44 = OpTypeInt 32 0
         %45 = OpConstant %44 0
         %46 = OpTypePointer Output %16
         %50 = OpConstant %44 1
         %54 = OpConstant %44 2
         %58 = OpConstant %44 3
         %64 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %209 = OpCopyObject %6 %12
         %14 = OpAccessChain %13 %11 %12
         %15 = OpLoad %6 %14
        %200 = OpCopyObject %6 %15
               OpBranch %20
         %20 = OpLabel
         %84 = OpPhi %6 %15 %5 %86 %69
         %27 = OpAccessChain %13 %11 %26
         %28 = OpLoad %6 %27
        %207 = OpCopyObject %6 %84
        %201 = OpCopyObject %6 %15
         %30 = OpSLessThan %29 %84 %28
               OpLoopMerge %22 %69 None
               OpBranchConditional %30 %21 %22
         %21 = OpLabel
         %33 = OpSMod %6 %84 %32
        %208 = OpCopyObject %6 %33
               OpSelectionMerge %39 None
               OpSwitch %33 %38 0 %34 1 %35 2 %36 3 %37
         %38 = OpLabel
        %202 = OpCopyObject %6 %201
               OpBranch %39
         %34 = OpLabel
        %210 = OpCopyObject %16 %19
         %47 = OpAccessChain %46 %42 %45
               OpStore %47 %210
               OpBranch %39
         %35 = OpLabel
         %51 = OpAccessChain %46 %42 %50
               OpStore %51 %19
               OpBranch %39
         %36 = OpLabel
        %204 = OpCopyObject %44 %54
         %55 = OpAccessChain %46 %42 %204
        %203 = OpCopyObject %46 %55
               OpStore %203 %19
               OpBranch %39
         %37 = OpLabel
         %59 = OpAccessChain %46 %42 %58
               OpStore %59 %19
               OpBranch %39
         %39 = OpLabel
        %300 = OpIAdd %6 %15 %15
         %65 = OpAccessChain %13 %11 %64
         %66 = OpLoad %6 %65
         %67 = OpSGreaterThan %29 %84 %66
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %72
         %68 = OpLabel
         %71 = OpIAdd %6 %84 %26
               OpBranch %69
         %72 = OpLabel
         %74 = OpIAdd %6 %84 %64
        %205 = OpCopyObject %6 %74
               OpBranch %69
         %69 = OpLabel
         %86 = OpPhi %6 %71 %68 %205 %72
        %301 = OpPhi %6 %71 %68 %15 %72
               OpBranch %20
         %22 = OpLabel
         %75 = OpAccessChain %46 %42 %50
         %76 = OpLoad %16 %75
         %78 = OpConvertSToF %16 %84
         %80 = OpAccessChain %46 %42 %45
        %206 = OpCopyObject %16 %78
         %81 = OpLoad %16 %80
         %79 = OpFAdd %16 %76 %78
         %82 = OpFAdd %16 %81 %79
               OpStore %80 %82
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, SynonymsOfVariables) {
  // The following SPIR-V comes from this GLSL, with object copies added:
  //
  // #version 310 es
  //
  // precision highp int;
  //
  // int g;
  //
  // void main() {
  //   int l;
  //   l = g;
  //   g = l;
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "l"
               OpName %10 "g"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypePointer Private %6
         %10 = OpVariable %9 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
        %100 = OpCopyObject %9 %10
        %101 = OpCopyObject %7 %8
         %11 = OpLoad %6 %10
               OpStore %8 %11
         %12 = OpLoad %6 %8
               OpStore %10 %12
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  fact_manager.AddFact(MakeSynonymFact(10, 100), context.get());
  fact_manager.AddFact(MakeSynonymFact(8, 101), context.get());

  // Replace %10 with %100 in:
  // %11 = OpLoad %6 %10
  auto replacement1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(10, SpvOpLoad, 0, 11, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_TRUE(replacement1.IsApplicable(context.get(), fact_manager));
  replacement1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %8 with %101 in:
  // OpStore %8 %11
  auto replacement2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(8, SpvOpStore, 0, 11, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_TRUE(replacement2.IsApplicable(context.get(), fact_manager));
  replacement2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %8 with %101 in:
  // %12 = OpLoad %6 %8
  auto replacement3 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(8, SpvOpLoad, 0, 12, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_TRUE(replacement3.IsApplicable(context.get(), fact_manager));
  replacement3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %10 with %100 in:
  // OpStore %10 %12
  auto replacement4 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(10, SpvOpStore, 0, 12, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), fact_manager));
  replacement4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "l"
               OpName %10 "g"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpTypePointer Private %6
         %10 = OpVariable %9 Private
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
        %100 = OpCopyObject %9 %10
        %101 = OpCopyObject %7 %8
         %11 = OpLoad %6 %100
               OpStore %101 %11
         %12 = OpLoad %6 %101
               OpStore %100 %12
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest,
     SynonymOfVariableNoGoodInFunctionCall) {
  // The following SPIR-V comes from this GLSL, with an object copy added:
  //
  // #version 310 es
  //
  // precision highp int;
  //
  // void foo(int x) { }
  //
  // void main() {
  //   int a;
  //   a = 2;
  //   foo(a);
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "foo(i1;"
               OpName %9 "x"
               OpName %12 "a"
               OpName %14 "param"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %8 = OpTypeFunction %2 %7
         %13 = OpConstant %6 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %12 = OpVariable %7 Function
         %14 = OpVariable %7 Function
               OpStore %12 %13
         %15 = OpLoad %6 %12
               OpStore %14 %15
        %100 = OpCopyObject %7 %14
         %16 = OpFunctionCall %2 %10 %14
               OpReturn
               OpFunctionEnd
         %10 = OpFunction %2 None %8
          %9 = OpFunctionParameter %7
         %11 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  fact_manager.AddFact(MakeSynonymFact(14, 100), context.get());

  // Replace %14 with %100 in:
  // %16 = OpFunctionCall %2 %10 %14
  auto replacement = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(14, SpvOpFunctionCall, 1, 16, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_FALSE(replacement.IsApplicable(context.get(), fact_manager));
}

TEST(TransformationReplaceIdWithSynonymTest, SynonymsOfAccessChainIndices) {
  // The following SPIR-V comes from this GLSL, with object copies added:
  //
  // #version 310 es
  //
  // precision highp float;
  // precision highp int;
  //
  // struct S {
  //   int[3] a;
  //   vec4 b;
  //   bool c;
  // } d;
  //
  // float[20] e;
  //
  // struct T {
  //   float f;
  //   S g;
  // } h;
  //
  // T[4] i;
  //
  // void main() {
  //   d.a[2] = 10;
  //   d.b[3] = 11.0;
  //   d.c = false;
  //   e[17] = 12.0;
  //   h.f = 13.0;
  //   h.g.a[1] = 14;
  //   h.g.b[0] = 15.0;
  //   h.g.c = true;
  //   i[0].f = 16.0;
  //   i[1].g.a[0] = 17;
  //   i[2].g.b[1] = 18.0;
  //   i[3].g.c = true;
  // }
  const std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %13 "S"
               OpMemberName %13 0 "a"
               OpMemberName %13 1 "b"
               OpMemberName %13 2 "c"
               OpName %15 "d"
               OpName %31 "e"
               OpName %35 "T"
               OpMemberName %35 0 "f"
               OpMemberName %35 1 "g"
               OpName %37 "h"
               OpName %50 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypeFloat 32
         %11 = OpTypeVector %10 4
         %12 = OpTypeBool
         %13 = OpTypeStruct %9 %11 %12
         %14 = OpTypePointer Private %13
         %15 = OpVariable %14 Private
         %16 = OpConstant %6 0
         %17 = OpConstant %6 2
         %18 = OpConstant %6 10
         %19 = OpTypePointer Private %6
         %21 = OpConstant %6 1
         %22 = OpConstant %10 11
         %23 = OpTypePointer Private %10
         %25 = OpConstantFalse %12
         %26 = OpTypePointer Private %12
         %28 = OpConstant %7 20
         %29 = OpTypeArray %10 %28
         %30 = OpTypePointer Private %29
         %31 = OpVariable %30 Private
         %32 = OpConstant %6 17
         %33 = OpConstant %10 12
         %35 = OpTypeStruct %10 %13
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpConstant %10 13
         %40 = OpConstant %6 14
         %42 = OpConstant %10 15
         %43 = OpConstant %7 0
         %45 = OpConstantTrue %12
         %47 = OpConstant %7 4
         %48 = OpTypeArray %35 %47
         %49 = OpTypePointer Private %48
         %50 = OpVariable %49 Private
         %51 = OpConstant %10 16
         %54 = OpConstant %10 18
         %55 = OpConstant %7 1
         %57 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel

         %100 = OpCopyObject %6 %16 ; 0
         %101 = OpCopyObject %6 %21 ; 1
         %102 = OpCopyObject %6 %17 ; 2
         %103 = OpCopyObject %6 %57 ; 3
         %104 = OpCopyObject %6 %18 ; 10
         %105 = OpCopyObject %6 %40 ; 14
         %106 = OpCopyObject %6 %32 ; 17
         %107 = OpCopyObject %7 %43 ; 0
         %108 = OpCopyObject %7 %55 ; 1
         %109 = OpCopyObject %7  %8 ; 3
         %110 = OpCopyObject %7 %47 ; 4
         %111 = OpCopyObject %7 %28 ; 20
         %112 = OpCopyObject %12 %45 ; true

         %20 = OpAccessChain %19 %15 %16 %17
               OpStore %20 %18
         %24 = OpAccessChain %23 %15 %21 %8
               OpStore %24 %22
         %27 = OpAccessChain %26 %15 %17
               OpStore %27 %25
         %34 = OpAccessChain %23 %31 %32
               OpStore %34 %33
         %39 = OpAccessChain %23 %37 %16
               OpStore %39 %38
         %41 = OpAccessChain %19 %37 %21 %16 %21
               OpStore %41 %40
         %44 = OpAccessChain %23 %37 %21 %21 %43
               OpStore %44 %42
         %46 = OpAccessChain %26 %37 %21 %17
               OpStore %46 %45
         %52 = OpAccessChain %23 %50 %16 %16
               OpStore %52 %51
         %53 = OpAccessChain %19 %50 %21 %21 %16 %16
               OpStore %53 %32
         %56 = OpAccessChain %23 %50 %17 %21 %21 %55
               OpStore %56 %54
         %58 = OpAccessChain %26 %50 %57 %21 %17
               OpStore %58 %45
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  // Add synonym facts corresponding to the OpCopyObject operations that have
  // been applied to all constants in the module.
  fact_manager.AddFact(MakeSynonymFact(16, 100), context.get());
  fact_manager.AddFact(MakeSynonymFact(21, 101), context.get());
  fact_manager.AddFact(MakeSynonymFact(17, 102), context.get());
  fact_manager.AddFact(MakeSynonymFact(57, 103), context.get());
  fact_manager.AddFact(MakeSynonymFact(18, 104), context.get());
  fact_manager.AddFact(MakeSynonymFact(40, 105), context.get());
  fact_manager.AddFact(MakeSynonymFact(32, 106), context.get());
  fact_manager.AddFact(MakeSynonymFact(43, 107), context.get());
  fact_manager.AddFact(MakeSynonymFact(55, 108), context.get());
  fact_manager.AddFact(MakeSynonymFact(8, 109), context.get());
  fact_manager.AddFact(MakeSynonymFact(47, 110), context.get());
  fact_manager.AddFact(MakeSynonymFact(28, 111), context.get());
  fact_manager.AddFact(MakeSynonymFact(45, 112), context.get());

  // Replacements of the form %16 -> %100

  // %20 = OpAccessChain %19 %15 *%16* %17
  // Corresponds to d.*a*[2]
  // The index %16 used for a cannot be replaced
  auto replacement1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 1, 20, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_FALSE(replacement1.IsApplicable(context.get(), fact_manager));

  // %39 = OpAccessChain %23 %37 *%16*
  // Corresponds to h.*f*
  // The index %16 used for f cannot be replaced
  auto replacement2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 1, 39, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_FALSE(replacement2.IsApplicable(context.get(), fact_manager));

  // %41 = OpAccessChain %19 %37 %21 *%16* %21
  // Corresponds to h.g.*a*[1]
  // The index %16 used for a cannot be replaced
  auto replacement3 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 2, 41, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_FALSE(replacement3.IsApplicable(context.get(), fact_manager));

  // %52 = OpAccessChain %23 %50 *%16* %16
  // Corresponds to i[*0*].f
  // The index %16 used for 0 *can* be replaced
  auto replacement4 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 1, 52, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_TRUE(replacement4.IsApplicable(context.get(), fact_manager));
  replacement4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %52 = OpAccessChain %23 %50 %16 *%16*
  // Corresponds to i[0].*f*
  // The index %16 used for f cannot be replaced
  auto replacement5 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 2, 52, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_FALSE(replacement5.IsApplicable(context.get(), fact_manager));

  // %53 = OpAccessChain %19 %50 %21 %21 *%16* %16
  // Corresponds to i[1].g.*a*[0]
  // The index %16 used for a cannot be replaced
  auto replacement6 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 3, 53, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_FALSE(replacement6.IsApplicable(context.get(), fact_manager));

  // %53 = OpAccessChain %19 %50 %21 %21 %16 *%16*
  // Corresponds to i[1].g.a[*0*]
  // The index %16 used for 0 *can* be replaced
  auto replacement7 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(16, SpvOpAccessChain, 4, 53, 0),
      MakeDataDescriptor(100, {}), 0);
  ASSERT_TRUE(replacement7.IsApplicable(context.get(), fact_manager));
  replacement7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %21 -> %101

  // %24 = OpAccessChain %23 %15 *%21* %8
  // Corresponds to d.*b*[3]
  // The index %24 used for b cannot be replaced
  auto replacement8 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 1, 24, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement8.IsApplicable(context.get(), fact_manager));

  // %41 = OpAccessChain %19 %37 *%21* %16 %21
  // Corresponds to h.*g*.a[1]
  // The index %24 used for g cannot be replaced
  auto replacement9 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 1, 41, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement9.IsApplicable(context.get(), fact_manager));

  // %41 = OpAccessChain %19 %37 %21 %16 *%21*
  // Corresponds to h.g.a[*1*]
  // The index %24 used for 1 *can* be replaced
  auto replacement10 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 3, 41, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_TRUE(replacement10.IsApplicable(context.get(), fact_manager));
  replacement10.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %44 = OpAccessChain %23 %37 *%21* %21 %43
  // Corresponds to h.*g*.b[0]
  // The index %24 used for g cannot be replaced
  auto replacement11 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 1, 44, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement11.IsApplicable(context.get(), fact_manager));

  // %44 = OpAccessChain %23 %37 %21 *%21* %43
  // Corresponds to h.g.*b*[0]
  // The index %24 used for b cannot be replaced
  auto replacement12 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 2, 44, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement12.IsApplicable(context.get(), fact_manager));

  // %46 = OpAccessChain %26 %37 *%21* %17
  // Corresponds to h.*g*.c
  // The index %24 used for g cannot be replaced
  auto replacement13 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 1, 46, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement13.IsApplicable(context.get(), fact_manager));

  // %53 = OpAccessChain %19 %50 *%21* %21 %16 %16
  // Corresponds to i[*1*].g.a[0]
  // The index %24 used for 1 *can* be replaced
  auto replacement14 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 1, 53, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_TRUE(replacement14.IsApplicable(context.get(), fact_manager));
  replacement14.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %53 = OpAccessChain %19 %50 %21 *%21* %16 %16
  // Corresponds to i[1].*g*.a[0]
  // The index %24 used for g cannot be replaced
  auto replacement15 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 2, 53, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement15.IsApplicable(context.get(), fact_manager));

  // %56 = OpAccessChain %23 %50 %17 *%21* %21 %55
  // Corresponds to i[2].*g*.b[1]
  // The index %24 used for g cannot be replaced
  auto replacement16 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 2, 56, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement16.IsApplicable(context.get(), fact_manager));

  // %56 = OpAccessChain %23 %50 %17 %21 *%21* %55
  // Corresponds to i[2].g.*b*[1]
  // The index %24 used for b cannot be replaced
  auto replacement17 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 3, 56, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement17.IsApplicable(context.get(), fact_manager));

  // %58 = OpAccessChain %26 %50 %57 *%21* %17
  // Corresponds to i[3].*g*.c
  // The index %24 used for g cannot be replaced
  auto replacement18 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(21, SpvOpAccessChain, 2, 58, 0),
      MakeDataDescriptor(101, {}), 0);
  ASSERT_FALSE(replacement18.IsApplicable(context.get(), fact_manager));

  // Replacements of the form %17 -> %102

  // %20 = OpAccessChain %19 %15 %16 %17
  // Corresponds to d.a[*2*]
  // The index %17 used for 2 *can* be replaced
  auto replacement19 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(17, SpvOpAccessChain, 2, 20, 0),
      MakeDataDescriptor(102, {}), 0);
  ASSERT_TRUE(replacement19.IsApplicable(context.get(), fact_manager));
  replacement19.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %27 = OpAccessChain %26 %15 %17
  // Corresponds to d.c
  // The index %17 used for c cannot be replaced
  auto replacement20 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(17, SpvOpAccessChain, 1, 27, 0),
      MakeDataDescriptor(102, {}), 0);
  ASSERT_FALSE(replacement20.IsApplicable(context.get(), fact_manager));

  // %46 = OpAccessChain %26 %37 %21 %17
  // Corresponds to h.g.*c*
  // The index %17 used for c cannot be replaced
  auto replacement21 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(17, SpvOpAccessChain, 2, 46, 0),
      MakeDataDescriptor(102, {}), 0);
  ASSERT_FALSE(replacement21.IsApplicable(context.get(), fact_manager));

  // %56 = OpAccessChain %23 %50 %17 %21 %21 %55
  // Corresponds to i[*2*].g.b[1]
  // The index %17 used for 2 *can* be replaced
  auto replacement22 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(17, SpvOpAccessChain, 1, 56, 0),
      MakeDataDescriptor(102, {}), 0);
  ASSERT_TRUE(replacement22.IsApplicable(context.get(), fact_manager));
  replacement22.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // %58 = OpAccessChain %26 %50 %57 %21 %17
  // Corresponds to i[3].g.*c*
  // The index %17 used for c cannot be replaced
  auto replacement23 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(17, SpvOpAccessChain, 3, 58, 0),
      MakeDataDescriptor(102, {}), 0);
  ASSERT_FALSE(replacement23.IsApplicable(context.get(), fact_manager));

  // Replacements of the form %57 -> %103

  // %58 = OpAccessChain %26 %50 *%57* %21 %17
  // Corresponds to i[*3*].g.c
  // The index %57 used for 3 *can* be replaced
  auto replacement24 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(57, SpvOpAccessChain, 1, 58, 0),
      MakeDataDescriptor(103, {}), 0);
  ASSERT_TRUE(replacement24.IsApplicable(context.get(), fact_manager));
  replacement24.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %32 -> %106

  // %34 = OpAccessChain %23 %31 *%32*
  // Corresponds to e[*17*]
  // The index %32 used for 17 *can* be replaced
  auto replacement25 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(32, SpvOpAccessChain, 1, 34, 0),
      MakeDataDescriptor(106, {}), 0);
  ASSERT_TRUE(replacement25.IsApplicable(context.get(), fact_manager));
  replacement25.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %43 -> %107

  // %44 = OpAccessChain %23 %37 %21 %21 *%43*
  // Corresponds to h.g.b[*0*]
  // The index %43 used for 0 *can* be replaced
  auto replacement26 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(43, SpvOpAccessChain, 3, 44, 0),
      MakeDataDescriptor(107, {}), 0);
  ASSERT_TRUE(replacement26.IsApplicable(context.get(), fact_manager));
  replacement26.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %55 -> %108

  // %56 = OpAccessChain %23 %50 %17 %21 %21 *%55*
  // Corresponds to i[2].g.b[*1*]
  // The index %55 used for 1 *can* be replaced
  auto replacement27 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(55, SpvOpAccessChain, 4, 56, 0),
      MakeDataDescriptor(108, {}), 0);
  ASSERT_TRUE(replacement27.IsApplicable(context.get(), fact_manager));
  replacement27.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replacements of the form %8 -> %109

  // %24 = OpAccessChain %23 %15 %21 *%8*
  // Corresponds to d.b[*3*]
  // The index %8 used for 3 *can* be replaced
  auto replacement28 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(8, SpvOpAccessChain, 2, 24, 0),
      MakeDataDescriptor(109, {}), 0);
  ASSERT_TRUE(replacement28.IsApplicable(context.get(), fact_manager));
  replacement28.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %13 "S"
               OpMemberName %13 0 "a"
               OpMemberName %13 1 "b"
               OpMemberName %13 2 "c"
               OpName %15 "d"
               OpName %31 "e"
               OpName %35 "T"
               OpMemberName %35 0 "f"
               OpMemberName %35 1 "g"
               OpName %37 "h"
               OpName %50 "i"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypeFloat 32
         %11 = OpTypeVector %10 4
         %12 = OpTypeBool
         %13 = OpTypeStruct %9 %11 %12
         %14 = OpTypePointer Private %13
         %15 = OpVariable %14 Private
         %16 = OpConstant %6 0
         %17 = OpConstant %6 2
         %18 = OpConstant %6 10
         %19 = OpTypePointer Private %6
         %21 = OpConstant %6 1
         %22 = OpConstant %10 11
         %23 = OpTypePointer Private %10
         %25 = OpConstantFalse %12
         %26 = OpTypePointer Private %12
         %28 = OpConstant %7 20
         %29 = OpTypeArray %10 %28
         %30 = OpTypePointer Private %29
         %31 = OpVariable %30 Private
         %32 = OpConstant %6 17
         %33 = OpConstant %10 12
         %35 = OpTypeStruct %10 %13
         %36 = OpTypePointer Private %35
         %37 = OpVariable %36 Private
         %38 = OpConstant %10 13
         %40 = OpConstant %6 14
         %42 = OpConstant %10 15
         %43 = OpConstant %7 0
         %45 = OpConstantTrue %12
         %47 = OpConstant %7 4
         %48 = OpTypeArray %35 %47
         %49 = OpTypePointer Private %48
         %50 = OpVariable %49 Private
         %51 = OpConstant %10 16
         %54 = OpConstant %10 18
         %55 = OpConstant %7 1
         %57 = OpConstant %6 3
          %4 = OpFunction %2 None %3
          %5 = OpLabel

         %100 = OpCopyObject %6 %16 ; 0
         %101 = OpCopyObject %6 %21 ; 1
         %102 = OpCopyObject %6 %17 ; 2
         %103 = OpCopyObject %6 %57 ; 3
         %104 = OpCopyObject %6 %18 ; 10
         %105 = OpCopyObject %6 %40 ; 14
         %106 = OpCopyObject %6 %32 ; 17
         %107 = OpCopyObject %7 %43 ; 0
         %108 = OpCopyObject %7 %55 ; 1
         %109 = OpCopyObject %7  %8 ; 3
         %110 = OpCopyObject %7 %47 ; 4
         %111 = OpCopyObject %7 %28 ; 20
         %112 = OpCopyObject %12 %45 ; true

         %20 = OpAccessChain %19 %15 %16 %102
               OpStore %20 %18
         %24 = OpAccessChain %23 %15 %21 %109
               OpStore %24 %22
         %27 = OpAccessChain %26 %15 %17
               OpStore %27 %25
         %34 = OpAccessChain %23 %31 %106
               OpStore %34 %33
         %39 = OpAccessChain %23 %37 %16
               OpStore %39 %38
         %41 = OpAccessChain %19 %37 %21 %16 %101
               OpStore %41 %40
         %44 = OpAccessChain %23 %37 %21 %21 %107
               OpStore %44 %42
         %46 = OpAccessChain %26 %37 %21 %17
               OpStore %46 %45
         %52 = OpAccessChain %23 %50 %100 %16
               OpStore %52 %51
         %53 = OpAccessChain %19 %50 %101 %21 %16 %100
               OpStore %53 %32
         %56 = OpAccessChain %23 %50 %102 %21 %21 %108
               OpStore %56 %54
         %58 = OpAccessChain %26 %50 %103 %21 %17
               OpStore %58 %45
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, ArrayCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "A"
               OpName %20 "B"
               OpName %31 "g"
               OpName %35 "h"
               OpDecorate %11 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 0
         %13 = OpConstant %6 3
         %14 = OpTypePointer Function %6
         %16 = OpTypeFloat 32
         %17 = OpConstant %7 4
         %18 = OpTypeArray %16 %17
         %19 = OpTypePointer Function %18
         %24 = OpTypePointer Function %16
         %28 = OpConstant %16 42
         %30 = OpConstant %6 2
         %34 = OpConstant %6 1
         %38 = OpConstant %6 42
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %20 = OpVariable %19 Function
         %31 = OpVariable %24 Function
         %35 = OpVariable %14 Function
         %15 = OpAccessChain %14 %11 %12
         %21 = OpAccessChain %14 %11 %12
         %22 = OpLoad %6 %21
        %100 = OpCompositeConstruct %9 %12 %13 %22
               OpStore %15 %13
         %23 = OpConvertSToF %16 %22
         %25 = OpAccessChain %24 %20 %12
               OpStore %25 %23
         %26 = OpAccessChain %14 %11 %12
         %27 = OpLoad %6 %26
         %29 = OpAccessChain %24 %20 %27
               OpStore %29 %28
         %32 = OpLoad %16 %31
        %101 = OpCompositeConstruct %18 %28 %23 %32 %23
         %50 = OpCopyObject %16 %23
         %51 = OpCopyObject %16 %23
         %33 = OpAccessChain %24 %20 %30
               OpStore %33 %28
               OpStore %33 %32
         %36 = OpLoad %6 %35
         %37 = OpAccessChain %14 %11 %34
               OpStore %37 %36
         %39 = OpAccessChain %14 %11 %12
         %40 = OpLoad %6 %39
         %41 = OpIAdd %6 %38 %40
         %42 = OpAccessChain %14 %11 %30
               OpStore %42 %41
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  fact_manager.AddFact(MakeSynonymFact(12, 100, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(13, 100, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(22, 100, {2}), context.get());
  fact_manager.AddFact(MakeSynonymFact(28, 101, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(23, 101, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(32, 101, {2}), context.get());
  fact_manager.AddFact(MakeSynonymFact(23, 101, {3}), context.get());

  // Replace %12 with %100[0] in '%25 = OpAccessChain %24 %20 %12'
  auto good_replacement_1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(12, SpvOpAccessChain, 1, 25, 0),
      MakeDataDescriptor(100, {0}), 102);
  // Bad: id already in use
  auto bad_replacement_1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(12, SpvOpAccessChain, 1, 25, 0),
      MakeDataDescriptor(100, {0}), 25);
  ASSERT_TRUE(good_replacement_1.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_1.IsApplicable(context.get(), fact_manager));
  good_replacement_1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %13 with %100[1] in 'OpStore %15 %13'
  auto good_replacement_2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(13, SpvOpStore, 1, 100, 0),
      MakeDataDescriptor(100, {1}), 103);
  // Bad: too many indices
  auto bad_replacement_2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(13, SpvOpStore, 1, 100, 0),
      MakeDataDescriptor(100, {1, 0}), 103);
  ASSERT_TRUE(good_replacement_2.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_2.IsApplicable(context.get(), fact_manager));
  good_replacement_2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %22 with %100[2] in '%23 = OpConvertSToF %16 %22'
  auto good_replacement_3 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(22, SpvOpConvertSToF, 0, 23, 0),
      MakeDataDescriptor(100, {2}), 104);
  // Bad: wrong input operand index
  auto bad_replacement_3 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(22, SpvOpConvertSToF, 1, 23, 0),
      MakeDataDescriptor(100, {2}), 104);
  ASSERT_TRUE(good_replacement_3.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_3.IsApplicable(context.get(), fact_manager));
  good_replacement_3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %28 with %101[0] in 'OpStore %33 %28'
  auto good_replacement_4 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(28, SpvOpStore, 1, 33, 0),
      MakeDataDescriptor(101, {0}), 105);
  // Bad: id use descriptor does not identify an appropriate instruction
  auto bad_replacement_4 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(28, SpvOpCopyObject, 1, 33, 0),
      MakeDataDescriptor(101, {0}), 105);
  ASSERT_TRUE(good_replacement_4.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_4.IsApplicable(context.get(), fact_manager));
  good_replacement_4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %23 with %101[1] in '%50 = OpCopyObject %16 %23'
  auto good_replacement_5 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(23, SpvOpCopyObject, 0, 50, 0),
      MakeDataDescriptor(101, {1}), 106);
  // Bad: wrong synonym fact being used
  auto bad_replacement_5 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(23, SpvOpCopyObject, 0, 50, 0),
      MakeDataDescriptor(101, {0}), 106);
  ASSERT_TRUE(good_replacement_5.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_5.IsApplicable(context.get(), fact_manager));
  good_replacement_5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %32 with %101[2] in 'OpStore %33 %32'
  auto good_replacement_6 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(32, SpvOpStore, 1, 33, 1),
      MakeDataDescriptor(101, {2}), 107);
  // Bad: id 1001 does not exist
  auto bad_replacement_6 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(32, SpvOpStore, 1, 33, 1),
      MakeDataDescriptor(1001, {2}), 107);
  ASSERT_TRUE(good_replacement_6.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_6.IsApplicable(context.get(), fact_manager));
  good_replacement_6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %23 with %101[3] in '%51 = OpCopyObject %16 %23'
  auto good_replacement_7 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(23, SpvOpCopyObject, 0, 51, 0),
      MakeDataDescriptor(101, {3}), 108);
  // Bad: id 0 is invalid
  auto bad_replacement_7 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(0, SpvOpCopyObject, 0, 51, 0),
      MakeDataDescriptor(101, {3}), 108);
  ASSERT_TRUE(good_replacement_7.IsApplicable(context.get(), fact_manager));
  ASSERT_FALSE(bad_replacement_7.IsApplicable(context.get(), fact_manager));
  good_replacement_7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %11 "A"
               OpName %20 "B"
               OpName %31 "g"
               OpName %35 "h"
               OpDecorate %11 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 3
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 0
         %13 = OpConstant %6 3
         %14 = OpTypePointer Function %6
         %16 = OpTypeFloat 32
         %17 = OpConstant %7 4
         %18 = OpTypeArray %16 %17
         %19 = OpTypePointer Function %18
         %24 = OpTypePointer Function %16
         %28 = OpConstant %16 42
         %30 = OpConstant %6 2
         %34 = OpConstant %6 1
         %38 = OpConstant %6 42
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %20 = OpVariable %19 Function
         %31 = OpVariable %24 Function
         %35 = OpVariable %14 Function
         %15 = OpAccessChain %14 %11 %12
         %21 = OpAccessChain %14 %11 %12
         %22 = OpLoad %6 %21
        %100 = OpCompositeConstruct %9 %12 %13 %22
        %103 = OpCompositeExtract %6 %100 1
               OpStore %15 %103
        %104 = OpCompositeExtract %6 %100 2
         %23 = OpConvertSToF %16 %104
        %102 = OpCompositeExtract %6 %100 0
         %25 = OpAccessChain %24 %20 %102
               OpStore %25 %23
         %26 = OpAccessChain %14 %11 %12
         %27 = OpLoad %6 %26
         %29 = OpAccessChain %24 %20 %27
               OpStore %29 %28
         %32 = OpLoad %16 %31
        %101 = OpCompositeConstruct %18 %28 %23 %32 %23
        %106 = OpCompositeExtract %16 %101 1
         %50 = OpCopyObject %16 %106
        %108 = OpCompositeExtract %16 %101 3
         %51 = OpCopyObject %16 %108
         %33 = OpAccessChain %24 %20 %30
        %105 = OpCompositeExtract %16 %101 0
               OpStore %33 %105
        %107 = OpCompositeExtract %16 %101 2
               OpStore %33 %107
         %36 = OpLoad %6 %35
         %37 = OpAccessChain %14 %11 %34
               OpStore %37 %36
         %39 = OpAccessChain %14 %11 %12
         %40 = OpLoad %6 %39
         %41 = OpIAdd %6 %38 %40
         %42 = OpAccessChain %14 %11 %30
               OpStore %42 %41
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, MatrixCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "m"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
         %50 = OpUndef %7
          %8 = OpTypeMatrix %7 3
          %9 = OpTypePointer Function %8
         %11 = OpTypeInt 32 1
         %12 = OpConstant %11 0
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %13 %13 %13 %13
         %15 = OpTypePointer Function %7
         %17 = OpConstant %11 1
         %18 = OpConstant %6 2
         %19 = OpConstantComposite %7 %18 %18 %18 %18
         %21 = OpConstant %11 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
         %16 = OpAccessChain %15 %10 %12
               OpStore %16 %14
         %20 = OpAccessChain %15 %10 %17
               OpStore %20 %19
         %22 = OpAccessChain %15 %10 %12
         %23 = OpLoad %7 %22
         %24 = OpAccessChain %15 %10 %17
         %25 = OpLoad %7 %24
        %100 = OpCompositeConstruct %8 %23 %25 %50
         %26 = OpFAdd %7 %23 %25
         %27 = OpAccessChain %15 %10 %21
               OpStore %27 %26
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  fact_manager.AddFact(MakeSynonymFact(23, 100, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(25, 100, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(50, 100, {2}), context.get());

  // Replace %23 with %100[0] in '%26 = OpFAdd %7 %23 %25'
  auto replacement_1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(23, SpvOpFAdd, 0, 26, 0),
      MakeDataDescriptor(100, {0}), 101);
  ASSERT_TRUE(replacement_1.IsApplicable(context.get(), fact_manager));
  replacement_1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %25 with %100[1] in '%26 = OpFAdd %7 %23 %25'
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(25, SpvOpFAdd, 1, 26, 0),
      MakeDataDescriptor(100, {1}), 102);
  ASSERT_TRUE(replacement_2.IsApplicable(context.get(), fact_manager));
  replacement_2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %10 "m"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
         %50 = OpUndef %7
          %8 = OpTypeMatrix %7 3
          %9 = OpTypePointer Function %8
         %11 = OpTypeInt 32 1
         %12 = OpConstant %11 0
         %13 = OpConstant %6 1
         %14 = OpConstantComposite %7 %13 %13 %13 %13
         %15 = OpTypePointer Function %7
         %17 = OpConstant %11 1
         %18 = OpConstant %6 2
         %19 = OpConstantComposite %7 %18 %18 %18 %18
         %21 = OpConstant %11 2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %10 = OpVariable %9 Function
         %16 = OpAccessChain %15 %10 %12
               OpStore %16 %14
         %20 = OpAccessChain %15 %10 %17
               OpStore %20 %19
         %22 = OpAccessChain %15 %10 %12
         %23 = OpLoad %7 %22
         %24 = OpAccessChain %15 %10 %17
         %25 = OpLoad %7 %24
        %100 = OpCompositeConstruct %8 %23 %25 %50
        %101 = OpCompositeExtract %7 %100 0
        %102 = OpCompositeExtract %7 %100 1
         %26 = OpFAdd %7 %101 %102
         %27 = OpAccessChain %15 %10 %21
               OpStore %27 %26
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, StructCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "Inner"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpName %11 "i1"
               OpName %17 "i2"
               OpName %31 "Point"
               OpMemberName %31 0 "x"
               OpMemberName %31 1 "y"
               OpMemberName %31 2 "z"
               OpName %32 "Outer"
               OpMemberName %32 0 "c"
               OpMemberName %32 1 "d"
               OpName %34 "o1"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 2
          %9 = OpTypeStruct %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %7 2
         %14 = OpConstant %7 3
         %15 = OpConstantComposite %8 %13 %14
         %16 = OpConstantComposite %9 %12 %15
         %18 = OpConstant %6 0
         %19 = OpTypePointer Function %6
         %24 = OpTypePointer Function %8
         %27 = OpConstant %7 4
         %31 = OpTypeStruct %7 %7 %7
         %32 = OpTypeStruct %9 %31
         %33 = OpTypePointer Function %32
         %36 = OpConstant %7 10
         %37 = OpTypeInt 32 0
         %38 = OpConstant %37 0
         %39 = OpTypePointer Function %7
         %42 = OpConstant %37 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %17 = OpVariable %10 Function
         %34 = OpVariable %33 Function
        %101 = OpCompositeConstruct %31 %27 %36 %27
               OpStore %11 %16
         %20 = OpAccessChain %19 %11 %18
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %21 %12
        %102 = OpCompositeConstruct %9 %22 %15
         %23 = OpAccessChain %19 %17 %18
               OpStore %23 %22
         %25 = OpAccessChain %24 %17 %12
         %26 = OpLoad %8 %25
         %28 = OpCompositeConstruct %8 %27 %27
         %29 = OpFAdd %8 %26 %28
         %30 = OpAccessChain %24 %17 %12
               OpStore %30 %29
         %35 = OpLoad %9 %11
         %40 = OpAccessChain %39 %11 %12 %38
         %41 = OpLoad %7 %40
         %43 = OpAccessChain %39 %11 %12 %42
         %44 = OpLoad %7 %43
         %45 = OpCompositeConstruct %31 %36 %41 %44
        %100 = OpCompositeConstruct %32 %16 %45
         %46 = OpCompositeConstruct %32 %35 %45
               OpStore %34 %46
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;

  fact_manager.AddFact(MakeSynonymFact(16, 100, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(45, 100, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(27, 101, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(36, 101, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(27, 101, {2}), context.get());
  fact_manager.AddFact(MakeSynonymFact(22, 102, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(15, 102, {1}), context.get());

  // Replace %45 with %100[1] in '%46 = OpCompositeConstruct %32 %35 %45'
  auto replacement_1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(45, SpvOpCompositeConstruct, 1, 46,
                                          0),
      MakeDataDescriptor(100, {1}), 201);
  ASSERT_TRUE(replacement_1.IsApplicable(context.get(), fact_manager));
  replacement_1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace second occurrence of %27 with %101[0] in '%28 =
  // OpCompositeConstruct %8 %27 %27'
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(27, SpvOpCompositeConstruct, 1, 28,
                                          0),
      MakeDataDescriptor(101, {0}), 202);
  ASSERT_TRUE(replacement_2.IsApplicable(context.get(), fact_manager));
  replacement_2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %36 with %101[1] in '%45 = OpCompositeConstruct %31 %36 %41 %44'
  auto replacement_3 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(36, SpvOpCompositeConstruct, 0, 45,
                                          0),
      MakeDataDescriptor(101, {1}), 203);
  ASSERT_TRUE(replacement_3.IsApplicable(context.get(), fact_manager));
  replacement_3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace first occurrence of %27 with %101[2] in '%28 = OpCompositeConstruct
  // %8 %27 %27'
  auto replacement_4 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(27, SpvOpCompositeConstruct, 0, 28,
                                          0),
      MakeDataDescriptor(101, {2}), 204);
  ASSERT_TRUE(replacement_4.IsApplicable(context.get(), fact_manager));
  replacement_4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %22 with %102[0] in 'OpStore %23 %22'
  auto replacement_5 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(22, SpvOpStore, 1, 23, 0),
      MakeDataDescriptor(102, {0}), 205);
  ASSERT_TRUE(replacement_5.IsApplicable(context.get(), fact_manager));
  replacement_5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %9 "Inner"
               OpMemberName %9 0 "a"
               OpMemberName %9 1 "b"
               OpName %11 "i1"
               OpName %17 "i2"
               OpName %31 "Point"
               OpMemberName %31 0 "x"
               OpMemberName %31 1 "y"
               OpMemberName %31 2 "z"
               OpName %32 "Outer"
               OpMemberName %32 0 "c"
               OpMemberName %32 1 "d"
               OpName %34 "o1"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 2
          %9 = OpTypeStruct %6 %8
         %10 = OpTypePointer Function %9
         %12 = OpConstant %6 1
         %13 = OpConstant %7 2
         %14 = OpConstant %7 3
         %15 = OpConstantComposite %8 %13 %14
         %16 = OpConstantComposite %9 %12 %15
         %18 = OpConstant %6 0
         %19 = OpTypePointer Function %6
         %24 = OpTypePointer Function %8
         %27 = OpConstant %7 4
         %31 = OpTypeStruct %7 %7 %7
         %32 = OpTypeStruct %9 %31
         %33 = OpTypePointer Function %32
         %36 = OpConstant %7 10
         %37 = OpTypeInt 32 0
         %38 = OpConstant %37 0
         %39 = OpTypePointer Function %7
         %42 = OpConstant %37 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %11 = OpVariable %10 Function
         %17 = OpVariable %10 Function
         %34 = OpVariable %33 Function
        %101 = OpCompositeConstruct %31 %27 %36 %27
               OpStore %11 %16
         %20 = OpAccessChain %19 %11 %18
         %21 = OpLoad %6 %20
         %22 = OpIAdd %6 %21 %12
        %102 = OpCompositeConstruct %9 %22 %15
         %23 = OpAccessChain %19 %17 %18
        %205 = OpCompositeExtract %6 %102 0
               OpStore %23 %205
         %25 = OpAccessChain %24 %17 %12
         %26 = OpLoad %8 %25
        %202 = OpCompositeExtract %7 %101 0
        %204 = OpCompositeExtract %7 %101 2
         %28 = OpCompositeConstruct %8 %204 %202
         %29 = OpFAdd %8 %26 %28
         %30 = OpAccessChain %24 %17 %12
               OpStore %30 %29
         %35 = OpLoad %9 %11
         %40 = OpAccessChain %39 %11 %12 %38
         %41 = OpLoad %7 %40
         %43 = OpAccessChain %39 %11 %12 %42
         %44 = OpLoad %7 %43
        %203 = OpCompositeExtract %7 %101 1
         %45 = OpCompositeConstruct %31 %203 %41 %44
        %100 = OpCompositeConstruct %32 %16 %45
        %201 = OpCompositeExtract %31 %100 1
         %46 = OpCompositeConstruct %32 %35 %201
               OpStore %34 %46
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

TEST(TransformationReplaceIdWithSynonymTest, VectorCompositeSynonyms) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "f"
               OpName %12 "v2"
               OpName %18 "v3"
               OpName %23 "v4"
               OpName %32 "b"
               OpName %36 "bv2"
               OpName %41 "bv3"
               OpName %50 "bv4"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 42
         %10 = OpTypeVector %6 2
         %11 = OpTypePointer Function %10
         %16 = OpTypeVector %6 3
         %17 = OpTypePointer Function %16
         %21 = OpTypeVector %6 4
         %22 = OpTypePointer Function %21
         %30 = OpTypeBool
         %31 = OpTypePointer Function %30
         %33 = OpConstantFalse %30
         %34 = OpTypeVector %30 2
         %35 = OpTypePointer Function %34
         %37 = OpConstantTrue %30
         %38 = OpConstantComposite %34 %37 %37
         %39 = OpTypeVector %30 3
         %40 = OpTypePointer Function %39
         %48 = OpTypeVector %30 4
         %49 = OpTypePointer Function %48
         %51 = OpTypeInt 32 0
         %52 = OpConstant %51 2
         %55 = OpConstant %6 0
         %57 = OpConstant %51 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %18 = OpVariable %17 Function
         %23 = OpVariable %22 Function
         %32 = OpVariable %31 Function
         %36 = OpVariable %35 Function
         %41 = OpVariable %40 Function
         %50 = OpVariable %49 Function
               OpStore %8 %9
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %8
         %15 = OpCompositeConstruct %10 %13 %14
               OpStore %12 %15
         %19 = OpLoad %10 %12
         %20 = OpVectorShuffle %16 %19 %19 0 0 1
               OpStore %18 %20
         %24 = OpLoad %16 %18
         %25 = OpLoad %6 %8
         %26 = OpCompositeExtract %6 %24 0
         %27 = OpCompositeExtract %6 %24 1
         %28 = OpCompositeExtract %6 %24 2
         %29 = OpCompositeConstruct %21 %26 %27 %28 %25
               OpStore %23 %29
               OpStore %32 %33
               OpStore %36 %38
         %42 = OpLoad %30 %32
         %43 = OpLoad %34 %36
         %44 = OpVectorShuffle %34 %43 %43 0 0
         %45 = OpCompositeExtract %30 %44 0
         %46 = OpCompositeExtract %30 %44 1
         %47 = OpCompositeConstruct %39 %42 %45 %46
               OpStore %41 %47
         %53 = OpAccessChain %7 %23 %52
         %54 = OpLoad %6 %53

        %100 = OpCompositeConstruct %21 %20 %54
        %101 = OpCompositeConstruct %21 %15 %19
        %102 = OpCompositeConstruct %16 %27 %15
        %103 = OpCompositeConstruct %48 %33 %47
        %104 = OpCompositeConstruct %34 %42 %45
        %105 = OpCompositeConstruct %39 %38 %46

         %86 = OpCopyObject %30 %33
         %56 = OpFOrdNotEqual %30 %54 %55
         %80 = OpCopyObject %16 %20
         %58 = OpAccessChain %7 %18 %57
         %59 = OpLoad %6 %58
         %60 = OpFOrdNotEqual %30 %59 %55
         %61 = OpLoad %34 %36
         %62 = OpLogicalAnd %30 %45 %46
         %63 = OpLogicalOr %30 %45 %46
         %64 = OpCompositeConstruct %48 %56 %60 %62 %63
               OpStore %12 %15
         %81 = OpVectorShuffle %16 %19 %19 0 0 1
         %82 = OpCompositeConstruct %21 %26 %27 %28 %25
         %83 = OpCopyObject %10 %15
         %84 = OpCopyObject %39 %47
               OpStore %50 %64
         %85 = OpCopyObject %30 %42
               OpStore %36 %38
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  ASSERT_TRUE(IsValid(env, context.get()));

  FactManager fact_manager;
  fact_manager.AddFact(MakeSynonymFact(20, 100, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(54, 100, {3}), context.get());
  fact_manager.AddFact(MakeSynonymFact(15, 101, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(19, 101, {2}), context.get());
  fact_manager.AddFact(MakeSynonymFact(27, 102, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(15, 102, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(33, 103, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(47, 103, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(42, 104, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(45, 104, {1}), context.get());
  fact_manager.AddFact(MakeSynonymFact(38, 105, {0}), context.get());
  fact_manager.AddFact(MakeSynonymFact(46, 105, {2}), context.get());

  // Replace %20 with %100[0] in '%80 = OpCopyObject %16 %20'
  auto replacement_1 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(20, SpvOpCopyObject, 0, 80, 0),
      MakeDataDescriptor(100, {0}), 200);
  ASSERT_TRUE(replacement_1.IsApplicable(context.get(), fact_manager));
  replacement_1.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %54 with %100[3] in '%56 = OpFOrdNotEqual %30 %54 %55'
  auto replacement_2 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(54, SpvOpFOrdNotEqual, 0, 56, 0),
      MakeDataDescriptor(100, {3}), 201);
  ASSERT_TRUE(replacement_2.IsApplicable(context.get(), fact_manager));
  replacement_2.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %15 with %101[0] in 'OpStore %12 %15'
  auto replacement_3 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(15, SpvOpStore, 1, 64, 0),
      MakeDataDescriptor(101, {0}), 202);
  ASSERT_TRUE(replacement_3.IsApplicable(context.get(), fact_manager));
  replacement_3.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %19 with %101[2] in '%81 = OpVectorShuffle %16 %19 %19 0 0 1'
  auto replacement_4 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(19, SpvOpVectorShuffle, 0, 81, 0),
      MakeDataDescriptor(101, {2}), 203);
  ASSERT_TRUE(replacement_4.IsApplicable(context.get(), fact_manager));
  replacement_4.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %27 with %102[0] in '%82 = OpCompositeConstruct %21 %26 %27 %28
  // %25'
  auto replacement_5 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(27, SpvOpCompositeConstruct, 1, 82,
                                          0),
      MakeDataDescriptor(102, {0}), 204);
  ASSERT_TRUE(replacement_5.IsApplicable(context.get(), fact_manager));
  replacement_5.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %15 with %102[1] in '%83 = OpCopyObject %10 %15'
  auto replacement_6 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(15, SpvOpCopyObject, 0, 83, 0),
      MakeDataDescriptor(102, {1}), 205);
  ASSERT_TRUE(replacement_6.IsApplicable(context.get(), fact_manager));
  replacement_6.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %33 with %103[0] in '%86 = OpCopyObject %30 %33'
  auto replacement_7 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(33, SpvOpCopyObject, 0, 86, 0),
      MakeDataDescriptor(103, {0}), 206);
  ASSERT_TRUE(replacement_7.IsApplicable(context.get(), fact_manager));
  replacement_7.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %47 with %103[1] in '%84 = OpCopyObject %39 %47'
  auto replacement_8 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(47, SpvOpCopyObject, 0, 84, 0),
      MakeDataDescriptor(103, {1}), 207);
  ASSERT_TRUE(replacement_8.IsApplicable(context.get(), fact_manager));
  replacement_8.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %42 with %104[0] in '%85 = OpCopyObject %30 %42'
  auto replacement_9 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(42, SpvOpCopyObject, 0, 85, 0),
      MakeDataDescriptor(104, {0}), 208);
  ASSERT_TRUE(replacement_9.IsApplicable(context.get(), fact_manager));
  replacement_9.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %45 with %104[1] in '%63 = OpLogicalOr %30 %45 %46'
  auto replacement_10 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(45, SpvOpLogicalOr, 0, 63, 0),
      MakeDataDescriptor(104, {1}), 209);
  ASSERT_TRUE(replacement_10.IsApplicable(context.get(), fact_manager));
  replacement_10.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %38 with %105[0] in 'OpStore %36 %38'
  auto replacement_11 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(38, SpvOpStore, 1, 85, 0),
      MakeDataDescriptor(105, {0}), 210);
  ASSERT_TRUE(replacement_11.IsApplicable(context.get(), fact_manager));
  replacement_11.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  // Replace %46 with %105[2] in '%62 = OpLogicalAnd %30 %45 %46'
  auto replacement_12 = TransformationReplaceIdWithSynonym(
      transformation::MakeIdUseDescriptor(46, SpvOpLogicalAnd, 1, 62, 0),
      MakeDataDescriptor(105, {2}), 211);
  ASSERT_TRUE(replacement_12.IsApplicable(context.get(), fact_manager));
  replacement_12.Apply(context.get(), &fact_manager);
  ASSERT_TRUE(IsValid(env, context.get()));

  const std::string after_transformation = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "f"
               OpName %12 "v2"
               OpName %18 "v3"
               OpName %23 "v4"
               OpName %32 "b"
               OpName %36 "bv2"
               OpName %41 "bv3"
               OpName %50 "bv4"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 42
         %10 = OpTypeVector %6 2
         %11 = OpTypePointer Function %10
         %16 = OpTypeVector %6 3
         %17 = OpTypePointer Function %16
         %21 = OpTypeVector %6 4
         %22 = OpTypePointer Function %21
         %30 = OpTypeBool
         %31 = OpTypePointer Function %30
         %33 = OpConstantFalse %30
         %34 = OpTypeVector %30 2
         %35 = OpTypePointer Function %34
         %37 = OpConstantTrue %30
         %38 = OpConstantComposite %34 %37 %37
         %39 = OpTypeVector %30 3
         %40 = OpTypePointer Function %39
         %48 = OpTypeVector %30 4
         %49 = OpTypePointer Function %48
         %51 = OpTypeInt 32 0
         %52 = OpConstant %51 2
         %55 = OpConstant %6 0
         %57 = OpConstant %51 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %18 = OpVariable %17 Function
         %23 = OpVariable %22 Function
         %32 = OpVariable %31 Function
         %36 = OpVariable %35 Function
         %41 = OpVariable %40 Function
         %50 = OpVariable %49 Function
               OpStore %8 %9
         %13 = OpLoad %6 %8
         %14 = OpLoad %6 %8
         %15 = OpCompositeConstruct %10 %13 %14
               OpStore %12 %15
         %19 = OpLoad %10 %12
         %20 = OpVectorShuffle %16 %19 %19 0 0 1
               OpStore %18 %20
         %24 = OpLoad %16 %18
         %25 = OpLoad %6 %8
         %26 = OpCompositeExtract %6 %24 0
         %27 = OpCompositeExtract %6 %24 1
         %28 = OpCompositeExtract %6 %24 2
         %29 = OpCompositeConstruct %21 %26 %27 %28 %25
               OpStore %23 %29
               OpStore %32 %33
               OpStore %36 %38
         %42 = OpLoad %30 %32
         %43 = OpLoad %34 %36
         %44 = OpVectorShuffle %34 %43 %43 0 0
         %45 = OpCompositeExtract %30 %44 0
         %46 = OpCompositeExtract %30 %44 1
         %47 = OpCompositeConstruct %39 %42 %45 %46
               OpStore %41 %47
         %53 = OpAccessChain %7 %23 %52
         %54 = OpLoad %6 %53

        %100 = OpCompositeConstruct %21 %20 %54
        %101 = OpCompositeConstruct %21 %15 %19
        %102 = OpCompositeConstruct %16 %27 %15
        %103 = OpCompositeConstruct %48 %33 %47
        %104 = OpCompositeConstruct %34 %42 %45
        %105 = OpCompositeConstruct %39 %38 %46

        %206 = OpCompositeExtract %30 %103 0
         %86 = OpCopyObject %30 %206
        %201 = OpCompositeExtract %6 %100 3
         %56 = OpFOrdNotEqual %30 %201 %55
        %200 = OpVectorShuffle %16 %100 %100 0 1 2
         %80 = OpCopyObject %16 %200
         %58 = OpAccessChain %7 %18 %57
         %59 = OpLoad %6 %58
         %60 = OpFOrdNotEqual %30 %59 %55
         %61 = OpLoad %34 %36
        %211 = OpCompositeExtract %30 %105 2
         %62 = OpLogicalAnd %30 %45 %211
        %209 = OpCompositeExtract %30 %104 1
         %63 = OpLogicalOr %30 %209 %46
         %64 = OpCompositeConstruct %48 %56 %60 %62 %63
        %202 = OpVectorShuffle %10 %101 %101 0 1
               OpStore %12 %202
        %203 = OpVectorShuffle %10 %101 %101 2 3
         %81 = OpVectorShuffle %16 %203 %19 0 0 1
        %204 = OpCompositeExtract %6 %102 0
         %82 = OpCompositeConstruct %21 %26 %204 %28 %25
        %205 = OpVectorShuffle %10 %102 %102 1 2
         %83 = OpCopyObject %10 %205
        %207 = OpVectorShuffle %39 %103 %103 1 2 3
         %84 = OpCopyObject %39 %207
               OpStore %50 %64
        %208 = OpCompositeExtract %30 %104 0
         %85 = OpCopyObject %30 %208
        %210 = OpVectorShuffle %34 %105 %105 0 1
               OpStore %36 %210
               OpReturn
               OpFunctionEnd
  )";

  ASSERT_TRUE(IsEqual(env, after_transformation, context.get()));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
