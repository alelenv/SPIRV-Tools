// Copyright (c) 2017 Google Inc.
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

// Tests for unique type declaration rules validator.

#include <string>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

using ValidateArithmetics = spvtest::ValidateBase<bool>;

std::string GenerateCode(const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability Int64
OpCapability Float64
OpCapability Matrix
%ext_inst = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1
%f64 = OpTypeFloat 64
%u64 = OpTypeInt 64 0
%s64 = OpTypeInt 64 1
%boolvec2 = OpTypeVector %bool 2
%s32vec2 = OpTypeVector %s32 2
%u32vec2 = OpTypeVector %u32 2
%u64vec2 = OpTypeVector %u64 2
%f32vec2 = OpTypeVector %f32 2
%f64vec2 = OpTypeVector %f64 2
%boolvec3 = OpTypeVector %bool 3
%u32vec3 = OpTypeVector %u32 3
%u64vec3 = OpTypeVector %u64 3
%s32vec3 = OpTypeVector %s32 3
%f32vec3 = OpTypeVector %f32 3
%f64vec3 = OpTypeVector %f64 3
%boolvec4 = OpTypeVector %bool 4
%u32vec4 = OpTypeVector %u32 4
%u64vec4 = OpTypeVector %u64 4
%s32vec4 = OpTypeVector %s32 4
%f32vec4 = OpTypeVector %f32 4
%f64vec4 = OpTypeVector %f64 4

%f32mat22 = OpTypeMatrix %f32vec2 2
%f32mat23 = OpTypeMatrix %f32vec2 3
%f32mat32 = OpTypeMatrix %f32vec3 2
%f32mat33 = OpTypeMatrix %f32vec3 3
%f64mat22 = OpTypeMatrix %f64vec2 2

%struct_f32_f32 = OpTypeStruct %f32 %f32
%struct_u32_u32 = OpTypeStruct %u32 %u32
%struct_u32_u32_u32 = OpTypeStruct %u32 %u32 %u32
%struct_s32_s32 = OpTypeStruct %s32 %s32
%struct_s32_u32 = OpTypeStruct %s32 %u32
%struct_u32vec2_u32vec2 = OpTypeStruct %u32vec2 %u32vec2
%struct_s32vec2_s32vec2 = OpTypeStruct %s32vec2 %s32vec2

%f32_0 = OpConstant %f32 0
%f32_1 = OpConstant %f32 1
%f32_2 = OpConstant %f32 2
%f32_3 = OpConstant %f32 3
%f32_4 = OpConstant %f32 4
%f32_pi = OpConstant %f32 3.14159

%s32_0 = OpConstant %s32 0
%s32_1 = OpConstant %s32 1
%s32_2 = OpConstant %s32 2
%s32_3 = OpConstant %s32 3
%s32_4 = OpConstant %s32 4
%s32_m1 = OpConstant %s32 -1

%u32_0 = OpConstant %u32 0
%u32_1 = OpConstant %u32 1
%u32_2 = OpConstant %u32 2
%u32_3 = OpConstant %u32 3
%u32_4 = OpConstant %u32 4

%f64_0 = OpConstant %f64 0
%f64_1 = OpConstant %f64 1
%f64_2 = OpConstant %f64 2
%f64_3 = OpConstant %f64 3
%f64_4 = OpConstant %f64 4

%s64_0 = OpConstant %s64 0
%s64_1 = OpConstant %s64 1
%s64_2 = OpConstant %s64 2
%s64_3 = OpConstant %s64 3
%s64_4 = OpConstant %s64 4
%s64_m1 = OpConstant %s64 -1

%u64_0 = OpConstant %u64 0
%u64_1 = OpConstant %u64 1
%u64_2 = OpConstant %u64 2
%u64_3 = OpConstant %u64 3
%u64_4 = OpConstant %u64 4

%u32vec2_01 = OpConstantComposite %u32vec2 %u32_0 %u32_1
%u32vec2_12 = OpConstantComposite %u32vec2 %u32_1 %u32_2
%u32vec3_012 = OpConstantComposite %u32vec3 %u32_0 %u32_1 %u32_2
%u32vec3_123 = OpConstantComposite %u32vec3 %u32_1 %u32_2 %u32_3
%u32vec4_0123 = OpConstantComposite %u32vec4 %u32_0 %u32_1 %u32_2 %u32_3
%u32vec4_1234 = OpConstantComposite %u32vec4 %u32_1 %u32_2 %u32_3 %u32_4

%s32vec2_01 = OpConstantComposite %s32vec2 %s32_0 %s32_1
%s32vec2_12 = OpConstantComposite %s32vec2 %s32_1 %s32_2
%s32vec3_012 = OpConstantComposite %s32vec3 %s32_0 %s32_1 %s32_2
%s32vec3_123 = OpConstantComposite %s32vec3 %s32_1 %s32_2 %s32_3
%s32vec4_0123 = OpConstantComposite %s32vec4 %s32_0 %s32_1 %s32_2 %s32_3
%s32vec4_1234 = OpConstantComposite %s32vec4 %s32_1 %s32_2 %s32_3 %s32_4

%f32vec2_01 = OpConstantComposite %f32vec2 %f32_0 %f32_1
%f32vec2_12 = OpConstantComposite %f32vec2 %f32_1 %f32_2
%f32vec3_012 = OpConstantComposite %f32vec3 %f32_0 %f32_1 %f32_2
%f32vec3_123 = OpConstantComposite %f32vec3 %f32_1 %f32_2 %f32_3
%f32vec4_0123 = OpConstantComposite %f32vec4 %f32_0 %f32_1 %f32_2 %f32_3
%f32vec4_1234 = OpConstantComposite %f32vec4 %f32_1 %f32_2 %f32_3 %f32_4

%f64vec2_01 = OpConstantComposite %f64vec2 %f64_0 %f64_1
%f64vec2_12 = OpConstantComposite %f64vec2 %f64_1 %f64_2
%f64vec3_012 = OpConstantComposite %f64vec3 %f64_0 %f64_1 %f64_2
%f64vec3_123 = OpConstantComposite %f64vec3 %f64_1 %f64_2 %f64_3
%f64vec4_0123 = OpConstantComposite %f64vec4 %f64_0 %f64_1 %f64_2 %f64_3
%f64vec4_1234 = OpConstantComposite %f64vec4 %f64_1 %f64_2 %f64_3 %f64_4

%f32mat22_1212 = OpConstantComposite %f32mat22 %f32vec2_12 %f32vec2_12
%f32mat23_121212 = OpConstantComposite %f32mat23 %f32vec2_12 %f32vec2_12 %f32vec2_12
%f32mat32_123123 = OpConstantComposite %f32mat32 %f32vec3_123 %f32vec3_123
%f32mat33_123123123 = OpConstantComposite %f32mat33 %f32vec3_123 %f32vec3_123 %f32vec3_123

%f64mat22_1212 = OpConstantComposite %f64mat22 %f64vec2_12 %f64vec2_12

%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + main_body + suffix;
}

TEST_F(ValidateArithmetics, F32Success) {
  const std::string body = R"(
%val1 = OpFMul %f32 %f32_0 %f32_1
%val2 = OpFSub %f32 %f32_2 %f32_0
%val3 = OpFAdd %f32 %val1 %val2
%val4 = OpFNegate %f32 %val3
%val5 = OpFDiv %f32 %val4 %val1
%val6 = OpFRem %f32 %val4 %f32_2
%val7 = OpFMod %f32 %val4 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, F64Success) {
  const std::string body = R"(
%val1 = OpFMul %f64 %f64_0 %f64_1
%val2 = OpFSub %f64 %f64_2 %f64_0
%val3 = OpFAdd %f64 %val1 %val2
%val4 = OpFNegate %f64 %val3
%val5 = OpFDiv %f64 %val4 %val1
%val6 = OpFRem %f64 %val4 %f64_2
%val7 = OpFMod %f64 %val4 %f64_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, Int32Success) {
  const std::string body = R"(
%val1 = OpIMul %u32 %s32_0 %u32_1
%val2 = OpIMul %s32 %s32_2 %u32_1
%val3 = OpIAdd %u32 %val1 %val2
%val4 = OpIAdd %s32 %val1 %val2
%val5 = OpISub %u32 %val3 %val4
%val6 = OpISub %s32 %val4 %val3
%val7 = OpSDiv %s32 %val4 %val3
%val8 = OpSNegate %s32 %val7
%val9 = OpSRem %s32 %val4 %val3
%val10 = OpSMod %s32 %val4 %val3
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, Int64Success) {
  const std::string body = R"(
%val1 = OpIMul %u64 %s64_0 %u64_1
%val2 = OpIMul %s64 %s64_2 %u64_1
%val3 = OpIAdd %u64 %val1 %val2
%val4 = OpIAdd %s64 %val1 %val2
%val5 = OpISub %u64 %val3 %val4
%val6 = OpISub %s64 %val4 %val3
%val7 = OpSDiv %s64 %val4 %val3
%val8 = OpSNegate %s64 %val7
%val9 = OpSRem %s64 %val4 %val3
%val10 = OpSMod %s64 %val4 %val3
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, F32Vec2Success) {
  const std::string body = R"(
%val1 = OpFMul %f32vec2 %f32vec2_01 %f32vec2_12
%val2 = OpFSub %f32vec2 %f32vec2_12 %f32vec2_01
%val3 = OpFAdd %f32vec2 %val1 %val2
%val4 = OpFNegate %f32vec2 %val3
%val5 = OpFDiv %f32vec2 %val4 %val1
%val6 = OpFRem %f32vec2 %val4 %f32vec2_12
%val7 = OpFMod %f32vec2 %val4 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, F64Vec2Success) {
  const std::string body = R"(
%val1 = OpFMul %f64vec2 %f64vec2_01 %f64vec2_12
%val2 = OpFSub %f64vec2 %f64vec2_12 %f64vec2_01
%val3 = OpFAdd %f64vec2 %val1 %val2
%val4 = OpFNegate %f64vec2 %val3
%val5 = OpFDiv %f64vec2 %val4 %val1
%val6 = OpFRem %f64vec2 %val4 %f64vec2_12
%val7 = OpFMod %f64vec2 %val4 %f64vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, U32Vec2Success) {
  const std::string body = R"(
%val1 = OpIMul %u32vec2 %u32vec2_01 %u32vec2_12
%val2 = OpISub %u32vec2 %u32vec2_12 %u32vec2_01
%val3 = OpIAdd %u32vec2 %val1 %val2
%val4 = OpSNegate %u32vec2 %val3
%val5 = OpSDiv %u32vec2 %val4 %val1
%val6 = OpSRem %u32vec2 %val4 %u32vec2_12
%val7 = OpSMod %u32vec2 %val4 %u32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, FNegateTypeIdU32) {
  const std::string body = R"(
%val = OpFNegate %u32 %u32_0
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected floating scalar or vector type as Result Type: FNegate"));
}

TEST_F(ValidateArithmetics, FNegateTypeIdVec2U32) {
  const std::string body = R"(
%val = OpFNegate %u32vec2 %u32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected floating scalar or vector type as Result Type: FNegate"));
}

TEST_F(ValidateArithmetics, FNegateWrongOperand) {
  const std::string body = R"(
%val = OpFNegate %f32 %u32_0
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "FNegate operand index 2"));
}

TEST_F(ValidateArithmetics, FMulTypeIdU32) {
  const std::string body = R"(
%val = OpFMul %u32 %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected floating scalar or vector type as Result Type: FMul"));
}

TEST_F(ValidateArithmetics, FMulTypeIdVec2U32) {
  const std::string body = R"(
%val = OpFMul %u32vec2 %u32vec2_01 %u32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected floating scalar or vector type as Result Type: FMul"));
}

TEST_F(ValidateArithmetics, FMulWrongOperand1) {
  const std::string body = R"(
%val = OpFMul %f32 %u32_0 %f32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "FMul operand index 2"));
}

TEST_F(ValidateArithmetics, FMulWrongOperand2) {
  const std::string body = R"(
%val = OpFMul %f32 %f32_0 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "FMul operand index 3"));
}

TEST_F(ValidateArithmetics, FMulWrongVectorOperand1) {
  const std::string body = R"(
%val = OpFMul %f64vec3 %f32vec3_123 %f64vec3_012
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "FMul operand index 2"));
}

TEST_F(ValidateArithmetics, FMulWrongVectorOperand2) {
  const std::string body = R"(
%val = OpFMul %f32vec3 %f32vec3_123 %f64vec3_012
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "FMul operand index 3"));
}

TEST_F(ValidateArithmetics, IMulFloatTypeId) {
  const std::string body = R"(
%val = OpIMul %f32 %u32_0 %s32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected int scalar or vector type as Result Type: IMul"));
}

TEST_F(ValidateArithmetics, IMulFloatOperand1) {
  const std::string body = R"(
%val = OpIMul %u32 %f32_0 %s32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected int scalar or vector type as operand: "
                        "IMul operand index 2"));
}

TEST_F(ValidateArithmetics, IMulFloatOperand2) {
  const std::string body = R"(
%val = OpIMul %u32 %s32_0 %f32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected int scalar or vector type as operand: "
                        "IMul operand index 3"));
}

TEST_F(ValidateArithmetics, IMulWrongBitWidthOperand1) {
  const std::string body = R"(
%val = OpIMul %u64 %u32_0 %s64_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same bit width "
                "as Result Type: IMul operand index 2"));
}

TEST_F(ValidateArithmetics, IMulWrongBitWidthOperand2) {
  const std::string body = R"(
%val = OpIMul %u32 %u32_0 %s64_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same bit width "
                "as Result Type: IMul operand index 3"));
}

TEST_F(ValidateArithmetics, IMulWrongBitWidthVector) {
  const std::string body = R"(
%val = OpIMul %u64vec3 %u32vec3_012 %u32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same bit width "
                "as Result Type: IMul operand index 2"));
}

TEST_F(ValidateArithmetics, IMulVectorScalarOperand1) {
  const std::string body = R"(
%val = OpIMul %u32vec2 %u32_0 %u32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same dimension "
                "as Result Type: IMul operand index 2"));
}

TEST_F(ValidateArithmetics, IMulVectorScalarOperand2) {
  const std::string body = R"(
%val = OpIMul %u32vec2 %u32vec2_01 %u32_0
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same dimension "
                "as Result Type: IMul operand index 3"));
}

TEST_F(ValidateArithmetics, IMulScalarVectorOperand1) {
  const std::string body = R"(
%val = OpIMul %s32 %u32vec2_01 %u32_0
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same dimension "
                "as Result Type: IMul operand index 2"));
}

TEST_F(ValidateArithmetics, IMulScalarVectorOperand2) {
  const std::string body = R"(
%val = OpIMul %u32 %u32_0 %s32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected arithmetic operands to have the same dimension "
                "as Result Type: IMul operand index 3"));
}

TEST_F(ValidateArithmetics, SNegateFloat) {
  const std::string body = R"(
%val = OpSNegate %s32 %f32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected int scalar or vector type as operand: "
                        "SNegate operand index 2"));
}

TEST_F(ValidateArithmetics, UDivFloatType) {
  const std::string body = R"(
%val = OpUDiv %f32 %u32_2 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected unsigned int scalar or vector type as Result Type: UDiv"));
}

TEST_F(ValidateArithmetics, UDivSignedIntType) {
  const std::string body = R"(
%val = OpUDiv %s32 %u32_2 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected unsigned int scalar or vector type as Result Type: UDiv"));
}

TEST_F(ValidateArithmetics, UDivWrongOperand1) {
  const std::string body = R"(
%val = OpUDiv %u64 %f64_2 %u64_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "UDiv operand index 2"));
}

TEST_F(ValidateArithmetics, UDivWrongOperand2) {
  const std::string body = R"(
%val = OpUDiv %u64 %u64_2 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected arithmetic operands to be of Result Type: "
                        "UDiv operand index 3"));
}

TEST_F(ValidateArithmetics, DotSuccess) {
  const std::string body = R"(
%val = OpDot %f32 %f32vec2_01 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, DotWrongTypeId) {
  const std::string body = R"(
%val = OpDot %u32 %u32vec2_01 %u32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float scalar type as Result Type: Dot"));
}

TEST_F(ValidateArithmetics, DotNotVectorTypeOperand1) {
  const std::string body = R"(
%val = OpDot %f32 %f32 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand '6[%float]' cannot be a "
                        "type"));
}

TEST_F(ValidateArithmetics, DotNotVectorTypeOperand2) {
  const std::string body = R"(
%val = OpDot %f32 %f32vec3_012 %f32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected float vector as operand: Dot operand index 3"));
}

TEST_F(ValidateArithmetics, DotWrongComponentOperand1) {
  const std::string body = R"(
%val = OpDot %f64 %f32vec2_01 %f64vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected component type to be equal to Result Type: "
                        "Dot operand index 2"));
}

TEST_F(ValidateArithmetics, DotWrongComponentOperand2) {
  const std::string body = R"(
%val = OpDot %f32 %f32vec2_01 %f64vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected component type to be equal to Result Type: "
                        "Dot operand index 3"));
}

TEST_F(ValidateArithmetics, DotDifferentVectorSize) {
  const std::string body = R"(
%val = OpDot %f32 %f32vec2_01 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected operands to have the same number of components: Dot"));
}

TEST_F(ValidateArithmetics, VectorTimesScalarSuccess) {
  const std::string body = R"(
%val = OpVectorTimesScalar %f32vec2 %f32vec2_01 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, VectorTimesScalarWrongTypeId) {
  const std::string body = R"(
%val = OpVectorTimesScalar %u32vec2 %f32vec2_01 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float vector type as Result Type: "
                        "VectorTimesScalar"));
}

TEST_F(ValidateArithmetics, VectorTimesScalarWrongVector) {
  const std::string body = R"(
%val = OpVectorTimesScalar %f32vec2 %f32vec3_012 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected vector operand type to be equal to Result Type: "
                "VectorTimesScalar"));
}

TEST_F(ValidateArithmetics, VectorTimesScalarWrongScalar) {
  const std::string body = R"(
%val = OpVectorTimesScalar %f32vec2 %f32vec2_01 %f64_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected scalar operand type to be equal to the component "
                "type of the vector operand: VectorTimesScalar"));
}

TEST_F(ValidateArithmetics, MatrixTimesScalarSuccess) {
  const std::string body = R"(
%val = OpMatrixTimesScalar %f32mat22 %f32mat22_1212 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, MatrixTimesScalarWrongTypeId) {
  const std::string body = R"(
%val = OpMatrixTimesScalar %f32vec2 %f32mat22_1212 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float matrix type as Result Type: "
                        "MatrixTimesScalar"));
}

TEST_F(ValidateArithmetics, MatrixTimesScalarWrongMatrix) {
  const std::string body = R"(
%val = OpMatrixTimesScalar %f32mat22 %f32vec2_01 %f32_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected matrix operand type to be equal to Result Type: "
                "MatrixTimesScalar"));
}

TEST_F(ValidateArithmetics, MatrixTimesScalarWrongScalar) {
  const std::string body = R"(
%val = OpMatrixTimesScalar %f32mat22 %f32mat22_1212 %f64_2
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected scalar operand type to be equal to the component "
                "type of the matrix operand: MatrixTimesScalar"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrix2x22Success) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f32vec2_12 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, VectorTimesMatrix3x32Success) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f32vec3_123 %f32mat32_123123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, VectorTimesMatrixWrongTypeId) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32mat22 %f32vec2_12 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float vector type as Result Type: "
                        "VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrixNotFloatVector) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %u32vec2_12 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float vector type as left operand: "
                        "VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrixWrongVectorComponent) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f64vec2_12 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected component types of Result Type and vector to be equal: "
          "VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrixWrongMatrix) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f32vec2_12 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float matrix type as right operand: "
                        "VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrixWrongMatrixComponent) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f32vec2_12 %f64mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected component types of Result Type and matrix to be equal: "
          "VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrix2eq2x23Fail) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f32vec2_12 %f32mat23_121212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected number of columns of the matrix to be equal to Result Type "
          "vector size: VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, VectorTimesMatrix2x32Fail) {
  const std::string body = R"(
%val = OpVectorTimesMatrix %f32vec2 %f32vec2_12 %f32mat32_123123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected number of rows of the matrix to be equal to the vector "
          "operand size: VectorTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesVector22x2Success) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec2 %f32mat22_1212 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, MatrixTimesVector23x3Success) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec2 %f32mat23_121212 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, MatrixTimesVectorWrongTypeId) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32mat22 %f32mat22_1212 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float vector type as Result Type: "
                        "MatrixTimesVector"));
}

TEST_F(ValidateArithmetics, MatrixTimesVectorWrongMatrix) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec3 %f32vec3_123 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float matrix type as left operand: "
                        "MatrixTimesVector"));
}

TEST_F(ValidateArithmetics, MatrixTimesVectorWrongMatrixCol) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec3 %f32mat23_121212 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected column type of the matrix to be equal to Result Type: "
          "MatrixTimesVector"));
}

TEST_F(ValidateArithmetics, MatrixTimesVectorWrongVector) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec2 %f32mat22_1212 %u32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float vector type as right operand: "
                        "MatrixTimesVector"));
}

TEST_F(ValidateArithmetics, MatrixTimesVectorDifferentComponents) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec2 %f32mat22_1212 %f64vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected component types of the operands to be equal: "
                        "MatrixTimesVector"));
}

TEST_F(ValidateArithmetics, MatrixTimesVector22x3Fail) {
  const std::string body = R"(
%val = OpMatrixTimesVector %f32vec2 %f32mat22_1212 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected number of columns of the matrix to be equal to the vector "
          "size: MatrixTimesVector"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrix22x22Success) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat22_1212 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, MatrixTimesMatrix23x32Success) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat23_121212 %f32mat32_123123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, MatrixTimesMatrix33x33Success) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat33 %f32mat33_123123123 %f32mat33_123123123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, MatrixTimesMatrixWrongTypeId) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32vec2 %f32mat22_1212 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected float matrix type as Result Type: MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrixWrongLeftOperand) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32vec2_12 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected float matrix type as left operand: MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrixWrongRightOperand) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat22_1212 %f32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected float matrix type as right operand: MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrix32x23Fail) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat32_123123 %f32mat23_121212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected column types of Result Type and left matrix to be equal: "
          "MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrixDifferentComponents) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat22_1212 %f64mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected component types of Result Type and right "
                        "matrix to be equal: "
                        "MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrix23x23Fail) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat23_121212 %f32mat23_121212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected number of columns of Result Type and right "
                        "matrix to be equal: "
                        "MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, MatrixTimesMatrix23x22Fail) {
  const std::string body = R"(
%val = OpMatrixTimesMatrix %f32mat22 %f32mat23_121212 %f32mat22_1212
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected number of columns of left matrix and number "
                        "of rows of right "
                        "matrix to be equal: MatrixTimesMatrix"));
}

TEST_F(ValidateArithmetics, OuterProduct2x2Success) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat22 %f32vec2_12 %f32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, OuterProduct3x2Success) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat32 %f32vec3_123 %f32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, OuterProduct2x3Success) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat23 %f32vec2_01 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, OuterProductWrongTypeId) {
  const std::string body = R"(
%val = OpOuterProduct %f32vec2 %f32vec2_01 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected float matrix type as Result Type: "
                        "OuterProduct"));
}

TEST_F(ValidateArithmetics, OuterProductWrongLeftOperand) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat22 %f32vec3_123 %f32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected column type of Result Type to be equal to the type "
                "of the left operand: OuterProduct"));
}

TEST_F(ValidateArithmetics, OuterProductRightOperandNotFloatVector) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat22 %f32vec2_12 %u32vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected float vector type as right operand: OuterProduct"));
}

TEST_F(ValidateArithmetics, OuterProductRightOperandWrongComponent) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat22 %f32vec2_12 %f64vec2_01
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected component types of the operands to be equal: "
                        "OuterProduct"));
}

TEST_F(ValidateArithmetics, OuterProductRightOperandWrongDimension) {
  const std::string body = R"(
%val = OpOuterProduct %f32mat22 %f32vec2_12 %f32vec3_123
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected number of columns of the matrix to be equal to the "
                "vector size of the right operand: OuterProduct"));
}

std::string GenerateBFloatCode(const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability BFloat16TypeKHR
OpCapability BFloat16DotProductKHR
OpCapability BFloat16CooperativeMatrixKHR
OpExtension "SPV_KHR_bfloat16"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpName %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bfloat16 = OpTypeFloat 16 BFloat16KHR
%_ptr_Function_bfloat16 = OpTypePointer Function %bfloat16
%v2bfloat16 = OpTypeVector %bfloat16 2
%_ptr_Function_v2bfloat16 = OpTypePointer Function %v2bfloat16
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + main_body + suffix;
}

TEST_F(ValidateArithmetics, DotBfloat16) {
  const std::string body = R"(
%v1 = OpVariable %_ptr_Function_v2bfloat16 Function
%v2 = OpVariable %_ptr_Function_v2bfloat16 Function
%12 = OpLoad %v2bfloat16 %v1
%14 = OpLoad %v2bfloat16 %v2
%15 = OpDot %bfloat16 %12 %14
)";

  CompileSuccessfully(GenerateBFloatCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

std::string GenerateCoopMatCode(const std::string& extra_types,
                                const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability Float16
OpCapability CooperativeMatrixNV
OpExtension "SPV_NV_cooperative_matrix"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1

%u32_8 = OpConstant %u32 8
%u32_16 = OpConstant %u32 16
%u32_4 = OpConstant %u32 4
%subgroup = OpConstant %u32 3

%f16mat = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_8 %u32_8
%u32mat = OpTypeCooperativeMatrixNV %u32 %subgroup %u32_8 %u32_8
%s32mat = OpTypeCooperativeMatrixNV %s32 %subgroup %u32_8 %u32_8

%f16_1 = OpConstant %f16 1
%f32_1 = OpConstant %f32 1
%u32_1 = OpConstant %u32 1
%s32_1 = OpConstant %s32 1

%f16mat_1 = OpConstantComposite %f16mat %f16_1
%u32mat_1 = OpConstantComposite %u32mat %u32_1
%s32mat_1 = OpConstantComposite %s32mat %s32_1

%u32_c1 = OpSpecConstant %u32 1
%u32_c2 = OpSpecConstant %u32 2

%f16matc = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_c1 %u32_c2
%f16matc_1 = OpConstantComposite %f16matc %f16_1

%mat16x4 = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_16 %u32_4
%mat4x16 = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_4 %u32_16
%mat16x16 = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_16 %u32_16
%f16mat_16x4_1 = OpConstantComposite %mat16x4 %f16_1
%f16mat_4x16_1 = OpConstantComposite %mat4x16 %f16_1
%f16mat_16x16_1 = OpConstantComposite %mat16x16 %f16_1)";

  const std::string func_begin =
      R"(
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + extra_types + func_begin + main_body + suffix;
}

TEST_F(ValidateArithmetics, CoopMatSuccess) {
  const std::string body = R"(
%val1 = OpFAdd %f16mat %f16mat_1 %f16mat_1
%val2 = OpFSub %f16mat %f16mat_1 %f16mat_1
%val3 = OpFDiv %f16mat %f16mat_1 %f16mat_1
%val4 = OpFNegate %f16mat %f16mat_1
%val5 = OpIAdd %u32mat %u32mat_1 %u32mat_1
%val6 = OpISub %u32mat %u32mat_1 %u32mat_1
%val7 = OpUDiv %u32mat %u32mat_1 %u32mat_1
%val8 = OpIAdd %s32mat %s32mat_1 %s32mat_1
%val9 = OpISub %s32mat %s32mat_1 %s32mat_1
%val10 = OpSDiv %s32mat %s32mat_1 %s32mat_1
%val11 = OpSNegate %s32mat %s32mat_1
%val12 = OpMatrixTimesScalar %f16mat %f16mat_1 %f16_1
%val13 = OpMatrixTimesScalar %u32mat %u32mat_1 %u32_1
%val14 = OpMatrixTimesScalar %s32mat %s32mat_1 %s32_1
%val15 = OpCooperativeMatrixMulAddNV %mat16x16 %f16mat_16x4_1 %f16mat_4x16_1 %f16mat_16x16_1
%val16 = OpCooperativeMatrixMulAddNV %f16matc %f16matc_1 %f16matc_1 %f16matc_1
)";

  CompileSuccessfully(GenerateCoopMatCode("", body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, CoopMatFMulFail) {
  const std::string body = R"(
%val1 = OpFMul %f16mat %f16mat_1 %f16mat_1
)";

  CompileSuccessfully(GenerateCoopMatCode("", body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Expected floating scalar or vector type as Result Type: FMul"));
}

TEST_F(ValidateArithmetics, CoopMatMatrixTimesScalarMismatchFail) {
  const std::string body = R"(
%val1 = OpMatrixTimesScalar %f16mat %f16mat_1 %f32_1
)";

  CompileSuccessfully(GenerateCoopMatCode("", body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected scalar operand type to be equal to the component "
                "type of the matrix operand: MatrixTimesScalar"));
}

TEST_F(ValidateArithmetics, CoopMatScopeFail) {
  const std::string types = R"(
%device = OpConstant %u32 1

%mat16x16_dv = OpTypeCooperativeMatrixNV %f16 %device %u32_16 %u32_16
%f16matdv_16x16_1 = OpConstantComposite %mat16x16_dv %f16_1
)";

  const std::string body = R"(
%val1 = OpCooperativeMatrixMulAddNV %mat16x16 %f16mat_16x4_1 %f16mat_4x16_1 %f16matdv_16x16_1
)";

  CompileSuccessfully(GenerateCoopMatCode(types, body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Cooperative matrix scopes must match: CooperativeMatrixMulAddNV"));
}

TEST_F(ValidateArithmetics, CoopMatDimFail) {
  const std::string body = R"(
%val1 = OpCooperativeMatrixMulAddNV %mat16x16 %f16mat_4x16_1 %f16mat_16x4_1 %f16mat_16x16_1
)";

  CompileSuccessfully(GenerateCoopMatCode("", body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cooperative matrix 'M' mismatch: CooperativeMatrixMulAddNV"));
}

TEST_F(ValidateArithmetics, CoopMatComponentTypeNotScalarNumeric) {
  const std::string types = R"(
%bad = OpTypeCooperativeMatrixNV %bool %subgroup %u32_8 %u32_8
)";

  CompileSuccessfully(GenerateCoopMatCode(types, "").c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpTypeCooperativeMatrix Component Type <id> "
                        "'4[%bool]' is not a scalar numerical type."));
}

TEST_F(ValidateArithmetics, CoopMatScopeNotConstantInt) {
  const std::string types = R"(
%bad = OpTypeCooperativeMatrixNV %f16 %f32_1 %u32_8 %u32_8
)";

  CompileSuccessfully(GenerateCoopMatCode(types, "").c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpTypeCooperativeMatrix Scope <id> '17[%float_1]' is not a "
                "constant instruction with scalar integer type."));
}

TEST_F(ValidateArithmetics, CoopMatRowsNotConstantInt) {
  const std::string types = R"(
%bad = OpTypeCooperativeMatrixNV %f16 %subgroup %f32_1 %u32_8
)";

  CompileSuccessfully(GenerateCoopMatCode(types, "").c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpTypeCooperativeMatrix Rows <id> '17[%float_1]' is not a "
                "constant instruction with scalar integer type."));
}

TEST_F(ValidateArithmetics, CoopMatColumnsNotConstantInt) {
  const std::string types = R"(
%bad = OpTypeCooperativeMatrixNV %f16 %subgroup %u32_8 %f32_1
)";

  CompileSuccessfully(GenerateCoopMatCode(types, "").c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpTypeCooperativeMatrix Cols <id> '17[%float_1]' is not a "
                "constant instruction with scalar integer type."));
}

TEST_F(ValidateArithmetics, IAddCarrySuccess) {
  const std::string body = R"(
%val1 = OpIAddCarry %struct_u32_u32 %u32_0 %u32_1
%val2 = OpIAddCarry %struct_u32vec2_u32vec2 %u32vec2_01 %u32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, IAddCarryResultTypeNotStruct) {
  const std::string body = R"(
%val = OpIAddCarry %u32 %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected a struct as Result Type: IAddCarry"));
}

TEST_F(ValidateArithmetics, IAddCarryResultTypeNotTwoMembers) {
  const std::string body = R"(
%val = OpIAddCarry %struct_u32_u32_u32 %u32_0 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Result Type struct to have two members: IAddCarry"));
}

TEST_F(ValidateArithmetics, IAddCarryResultTypeMemberNotUnsignedInt) {
  const std::string body = R"(
%val = OpIAddCarry %struct_s32_s32 %s32_0 %s32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type struct member types to be "
                        "unsigned integer scalar "
                        "or vector: IAddCarry"));
}

TEST_F(ValidateArithmetics, IAddCarryWrongLeftOperand) {
  const std::string body = R"(
%val = OpIAddCarry %struct_u32_u32 %s32_0 %u32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected both operands to be of Result Type member "
                        "type: IAddCarry"));
}

TEST_F(ValidateArithmetics, IAddCarryWrongRightOperand) {
  const std::string body = R"(
%val = OpIAddCarry %struct_u32_u32 %u32_0 %s32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected both operands to be of Result Type member "
                        "type: IAddCarry"));
}

TEST_F(ValidateArithmetics, OpSMulExtendedSuccess) {
  const std::string body = R"(
%val1 = OpSMulExtended %struct_u32_u32 %u32_0 %u32_1
%val2 = OpSMulExtended %struct_s32_s32 %s32_0 %s32_1
%val3 = OpSMulExtended %struct_u32vec2_u32vec2 %u32vec2_01 %u32vec2_12
%val4 = OpSMulExtended %struct_s32vec2_s32vec2 %s32vec2_01 %s32vec2_12
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, SMulExtendedResultTypeMemberNotInt) {
  const std::string body = R"(
%val = OpSMulExtended %struct_f32_f32 %f32_0 %f32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Result Type struct member types to be integer scalar "
                "or vector: SMulExtended"));
}

TEST_F(ValidateArithmetics, SMulExtendedResultTypeMembersNotIdentical) {
  const std::string body = R"(
%val = OpSMulExtended %struct_s32_u32 %s32_0 %s32_1
)";

  CompileSuccessfully(GenerateCode(body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected Result Type struct member types to be identical: "
                "SMulExtended"));
}

std::string GenerateCoopMatKHRCode(const std::string& extra_types,
                                   const std::string& main_body) {
  const std::string prefix = R"(
OpCapability Shader
OpCapability Float16
OpCapability CooperativeMatrixKHR
OpCapability CooperativeMatrixReductionsNV
OpCapability CooperativeMatrixPerElementOperationsNV
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_NV_cooperative_matrix2"
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1

%u32_8 = OpConstant %u32 8
%u32_16 = OpConstant %u32 16
%u32_4 = OpConstant %u32 4
%subgroup = OpConstant %u32 3
%useA = OpConstant %u32 0
%useB = OpConstant %u32 1
%useC = OpConstant %u32 2

%f16matA = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_16 %u32_16 %useA
%u32matA = OpTypeCooperativeMatrixKHR %u32 %subgroup %u32_16 %u32_16 %useA
%s32matA = OpTypeCooperativeMatrixKHR %s32 %subgroup %u32_16 %u32_16 %useA

%f16matB = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_16 %u32_16 %useB
%u32matB = OpTypeCooperativeMatrixKHR %u32 %subgroup %u32_16 %u32_16 %useB
%s32matB = OpTypeCooperativeMatrixKHR %s32 %subgroup %u32_16 %u32_16 %useB

%f16matC = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_16 %u32_16 %useC
%f32matC = OpTypeCooperativeMatrixKHR %f32 %subgroup %u32_16 %u32_16 %useC
%u32matC = OpTypeCooperativeMatrixKHR %u32 %subgroup %u32_16 %u32_16 %useC
%s32matC = OpTypeCooperativeMatrixKHR %s32 %subgroup %u32_16 %u32_16 %useC

%f16_1 = OpConstant %f16 1
%f32_1 = OpConstant %f32 1
%u32_1 = OpConstant %u32 1
%s32_1 = OpConstant %s32 1

%f16mat_A_1 = OpConstantComposite %f16matA %f16_1
%u32mat_A_1 = OpConstantComposite %u32matA %u32_1
%s32mat_A_1 = OpConstantComposite %s32matA %s32_1

%f16mat_B_1 = OpConstantComposite %f16matB %f16_1
%u32mat_B_1 = OpConstantComposite %u32matB %u32_1
%s32mat_B_1 = OpConstantComposite %s32matB %s32_1

%f16mat_C_1 = OpConstantComposite %f16matC %f16_1
%u32mat_C_1 = OpConstantComposite %u32matC %u32_1
%s32mat_C_1 = OpConstantComposite %s32matC %s32_1

)";

  const std::string func_begin = R"(
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix = R"(
OpReturn
OpFunctionEnd)";

  return prefix + extra_types + func_begin + main_body + suffix;
}

TEST_F(ValidateArithmetics, CoopMatKHRSuccess) {
  const std::string body = R"(
%val1 = OpFAdd %f16matA %f16mat_A_1 %f16mat_A_1
%val2 = OpFSub %f16matA %f16mat_A_1 %f16mat_A_1
%val3 = OpFMul %f16matA %f16mat_A_1 %f16mat_A_1
%val4 = OpFDiv %f16matA %f16mat_A_1 %f16mat_A_1
%val5 = OpFNegate %f16matA %f16mat_A_1
%val6 = OpIAdd %u32matA %u32mat_A_1 %u32mat_A_1
%val7 = OpISub %u32matA %u32mat_A_1 %u32mat_A_1
%val8 = OpUDiv %u32matA %u32mat_A_1 %u32mat_A_1
%val9 = OpIAdd %s32matA %s32mat_A_1 %s32mat_A_1
%val10 = OpISub %s32matA %s32mat_A_1 %s32mat_A_1
%val11 = OpSDiv %s32matA %s32mat_A_1 %s32mat_A_1
%val12 = OpSNegate %s32matA %s32mat_A_1
%val13 = OpMatrixTimesScalar %f16matA %f16mat_A_1 %f16_1
%val14 = OpMatrixTimesScalar %u32matA %u32mat_A_1 %u32_1
%val15 = OpMatrixTimesScalar %s32matA %s32mat_A_1 %s32_1
%val16 = OpCooperativeMatrixMulAddKHR %f32matC %f16mat_A_1 %f16mat_B_1 %f16mat_C_1
%val17 = OpCooperativeMatrixMulAddKHR %s32matC %s32mat_A_1 %s32mat_B_1 %s32mat_C_1
  MatrixASignedComponentsKHR|MatrixBSignedComponentsKHR|MatrixCSignedComponentsKHR|MatrixResultSignedComponentsKHR
%val18 = OpCooperativeMatrixMulAddKHR %u32matC %u32mat_A_1 %u32mat_B_1 %u32mat_C_1
)";

  CompileSuccessfully(GenerateCoopMatKHRCode("", body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateArithmetics, CoopMatMatrixKHRTimesScalarMismatchFail) {
  const std::string body = R"(
%val1 = OpMatrixTimesScalar %f16matA %f16mat_A_1 %f32_1
)";

  CompileSuccessfully(GenerateCoopMatKHRCode("", body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected scalar operand type to be equal to the component "
                "type of the matrix operand: MatrixTimesScalar"));
}

TEST_F(ValidateArithmetics, CoopMatKHRScopeFail) {
  const std::string types = R"(
%device = OpConstant %u32 1
%mat16x16_dv = OpTypeCooperativeMatrixKHR %f16 %device %u32_16 %u32_16 %useC
%f16matdv_16x16_1 = OpConstantComposite %mat16x16_dv %f16_1
)";

  const std::string body = R"(
%val1 = OpFAdd %f16matA %f16matdv_16x16_1 %f16mat_A_1
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected scopes of Matrix and Result Type to be identical"));
}

TEST_F(ValidateArithmetics, CoopMatKHRDimFail) {
  const std::string types = R"(
%mat16x4 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_16 %u32_4 %useC
%mat16x4_C_1 = OpConstantComposite %mat16x4 %f16_1
)";

  const std::string body = R"(
%val1 = OpCooperativeMatrixMulAddKHR %mat16x4 %f16mat_A_1 %f16mat_B_1 %mat16x4_C_1
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Cooperative matrix 'N' mismatch: CooperativeMatrixMulAddKHR"));
}

TEST_F(ValidateArithmetics, CoopMat2ReduceSuccess) {
  const std::string extra_types = R"(

%f16matC8 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_8 %u32_8 %useC
%f16matC16x8 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_16 %u32_8 %useC
%f16matC8x16 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_8 %u32_16 %useC

%functy = OpTypeFunction %f16 %f16 %f16
%reducefunc = OpFunction %f16 None %functy
%x =  OpFunctionParameter %f16
%y =  OpFunctionParameter %f16
%entry2 = OpLabel
%sum = OpFAdd %f16 %x %y
OpReturnValue %sum
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixReduceNV %f16matC8 %f16mat_C_1 2x2 %reducefunc
%val2 = OpCooperativeMatrixReduceNV %f16matC16x8 %f16mat_C_1 Row %reducefunc
%val3 = OpCooperativeMatrixReduceNV %f16matC8x16 %f16mat_C_1 Column %reducefunc
%val4 = OpCooperativeMatrixReduceNV %f16matC %f16mat_C_1 Row|Column %reducefunc
%val5 = OpCooperativeMatrixReduceNV %f16matC8 %f16mat_C_1 Row|Column %reducefunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateArithmetics, CoopMat2Reduce2x2DimFail) {
  const std::string extra_types = R"(

%functy = OpTypeFunction %f16 %f16 %f16
%reducefunc = OpFunction %f16 None %functy
%x =  OpFunctionParameter %f16
%y =  OpFunctionParameter %f16
%entry2 = OpLabel
%sum = OpFAdd %f16 %x %y
OpReturnValue %sum
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixReduceNV %f16matC %f16mat_C_1 2x2 %reducefunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For Reduce2x2, result rows/cols must be half of "
                        "matrix rows/cols: CooperativeMatrixReduceNV"));
}

TEST_F(ValidateArithmetics, CoopMat2ReduceRowDimFail) {
  const std::string extra_types = R"(

%f16matC8x16 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_8 %u32_16 %useC

%functy = OpTypeFunction %f16 %f16 %f16
%reducefunc = OpFunction %f16 None %functy
%x =  OpFunctionParameter %f16
%y =  OpFunctionParameter %f16
%entry2 = OpLabel
%sum = OpFAdd %f16 %x %y
OpReturnValue %sum
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixReduceNV %f16matC8x16 %f16mat_C_1 Row %reducefunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For ReduceRow, result rows must match matrix rows: "
                        "CooperativeMatrixReduceNV"));
}

TEST_F(ValidateArithmetics, CoopMat2ReduceColDimFail) {
  const std::string extra_types = R"(

%f16matC16x8 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_16 %u32_8 %useC

%functy = OpTypeFunction %f16 %f16 %f16
%reducefunc = OpFunction %f16 None %functy
%x =  OpFunctionParameter %f16
%y =  OpFunctionParameter %f16
%entry2 = OpLabel
%sum = OpFAdd %f16 %x %y
OpReturnValue %sum
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixReduceNV %f16matC16x8 %f16mat_C_1 Column %reducefunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For ReduceColumn, result cols must match matrix cols: "
                        "CooperativeMatrixReduceNV"));
}

TEST_F(ValidateArithmetics, CoopMat2ReduceMaskFail) {
  const std::string extra_types = R"(

%f16matC8 = OpTypeCooperativeMatrixKHR %f16 %subgroup %u32_8 %u32_8 %useC

%functy = OpTypeFunction %f16 %f16 %f16
%reducefunc = OpFunction %f16 None %functy
%x =  OpFunctionParameter %f16
%y =  OpFunctionParameter %f16
%entry2 = OpLabel
%sum = OpFAdd %f16 %x %y
OpReturnValue %sum
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixReduceNV %f16matC8 %f16mat_C_1 Row|Column|2x2 %reducefunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Reduce 2x2 must not be used with Row/Column: "
                        "CooperativeMatrixReduceNV"));
}

TEST_F(ValidateArithmetics, CoopMat2ReduceFuncTypeFail) {
  const std::string extra_types = R"(

%functy = OpTypeFunction %f32 %f32 %f32
%reducefunc = OpFunction %f32 None %functy
%x =  OpFunctionParameter %f32
%y =  OpFunctionParameter %f32
%entry2 = OpLabel
%sum = OpFAdd %f32 %x %y
OpReturnValue %sum
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixReduceNV %f16matC %f16mat_C_1 Row|Column %reducefunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("CombineFunc return type and parameters must match "
                        "matrix component type: CooperativeMatrixReduceNV"));
}

TEST_F(ValidateArithmetics, CoopMat2PerElementOpSuccess) {
  const std::string extra_types = R"(

%functy = OpTypeFunction %f16 %u32 %u32 %f16
%functy2 = OpTypeFunction %f16 %u32 %u32 %f16 %u32

%elemfunc = OpFunction %f16 None %functy
%row =  OpFunctionParameter %u32
%col =  OpFunctionParameter %u32
%el =  OpFunctionParameter %f16
%entry2 = OpLabel
OpReturnValue %el
OpFunctionEnd

%elemfunc2 = OpFunction %f16 None %functy2
%row2 =  OpFunctionParameter %u32
%col2 =  OpFunctionParameter %u32
%el2 =  OpFunctionParameter %f16
%x =  OpFunctionParameter %u32
%entry3 = OpLabel
OpReturnValue %el2
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixPerElementOpNV %f16matC %f16mat_C_1 %elemfunc
%val2 = OpCooperativeMatrixPerElementOpNV %f16matC %f16mat_C_1 %elemfunc2 %f16_1
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateArithmetics, CoopMat2PerElementOpElemTyFail) {
  const std::string extra_types = R"(

%functy = OpTypeFunction %f32 %u32 %u32 %f32

%elemfunc = OpFunction %f32 None %functy
%row =  OpFunctionParameter %u32
%col =  OpFunctionParameter %u32
%el =  OpFunctionParameter %f32
%entry2 = OpLabel
OpReturnValue %el
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixPerElementOpNV %f16matC %f16mat_C_1 %elemfunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must match matrix component type"));
}

TEST_F(ValidateArithmetics, CoopMat2PerElementOpRowTyFail) {
  const std::string extra_types = R"(

%functy = OpTypeFunction %f16 %f16 %u32 %f16

%elemfunc = OpFunction %f16 None %functy
%row =  OpFunctionParameter %f16
%col =  OpFunctionParameter %u32
%el =  OpFunctionParameter %f16
%entry2 = OpLabel
OpReturnValue %el
OpFunctionEnd

  )";
  const std::string body = R"(
%val1 = OpCooperativeMatrixPerElementOpNV %f16matC %f16mat_C_1 %elemfunc
)";

  CompileSuccessfully(GenerateCoopMatKHRCode(extra_types, body).c_str(),
                      SPV_ENV_UNIVERSAL_1_3);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a 32-bit integer"));
}

std::string GenerateCoopVecCode(const std::string& extra_types,
                                const std::string& main_body) {
  const std::string prefix =
      R"(
OpCapability Shader
OpCapability Float16
OpCapability CooperativeVectorNV
OpCapability ReplicatedCompositesEXT
OpExtension "SPV_NV_cooperative_vector"
OpExtension "SPV_EXT_replicated_composites"
%ext_inst = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f16 = OpTypeFloat 16
%f32 = OpTypeFloat 32
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1

%u32_8 = OpConstant %u32 8
%u32_16 = OpConstant %u32 16
%u32_4 = OpConstant %u32 4
%subgroup = OpConstant %u32 3

%f16vec = OpTypeCooperativeVectorNV %f16 %u32_8
%f16vec4 = OpTypeCooperativeVectorNV %f16 %u32_4
%u32vec = OpTypeCooperativeVectorNV %u32 %u32_8
%s32vec = OpTypeCooperativeVectorNV %s32 %u32_8

%f16_1 = OpConstant %f16 1
%f32_1 = OpConstant %f32 1
%u32_1 = OpConstant %u32 1
%s32_1 = OpConstant %s32 1

%f16vec4_1 = OpConstantComposite %f16vec4 %f16_1 %f16_1 %f16_1 %f16_1
%f16vec_1 = OpConstantComposite %f16vec %f16_1 %f16_1 %f16_1 %f16_1 %f16_1 %f16_1 %f16_1 %f16_1
%u32vec_1 = OpConstantComposite %u32vec %u32_1 %u32_1 %u32_1 %u32_1 %u32_1 %u32_1 %u32_1 %u32_1
%s32vec_1 = OpConstantComposite %s32vec %s32_1 %s32_1 %s32_1 %s32_1 %s32_1 %s32_1 %s32_1 %s32_1

%u32_c1 = OpSpecConstant %u32 1
%u32_c2 = OpSpecConstant %u32 2

%f16vecc = OpTypeCooperativeVectorNV %f16 %u32_c1
%f16vecc_1 = OpConstantCompositeReplicateEXT %f16vecc %f16_1
)";

  const std::string func_begin =
      R"(
%main = OpFunction %void None %func
%main_entry = OpLabel)";

  const std::string suffix =
      R"(
OpReturn
OpFunctionEnd)";

  return prefix + extra_types + func_begin + main_body + suffix;
}

TEST_F(ValidateArithmetics, CoopVecSuccess) {
  const std::string body = R"(
%val1 = OpFAdd %f16vec %f16vec_1 %f16vec_1
%val2 = OpFSub %f16vec %f16vec_1 %f16vec_1
%val3 = OpFDiv %f16vec %f16vec_1 %f16vec_1
%val4 = OpFNegate %f16vec %f16vec_1
%val5 = OpIAdd %u32vec %u32vec_1 %u32vec_1
%val6 = OpISub %u32vec %u32vec_1 %u32vec_1
%val7 = OpUDiv %u32vec %u32vec_1 %u32vec_1
%val8 = OpIAdd %s32vec %s32vec_1 %s32vec_1
%val9 = OpISub %s32vec %s32vec_1 %s32vec_1
%val10 = OpSDiv %s32vec %s32vec_1 %s32vec_1
%val11 = OpSNegate %s32vec %s32vec_1
%val12 = OpVectorTimesScalar %f16vec %f16vec_1 %f16_1
%val13 = OpExtInst %f16vec %ext_inst FMin %f16vec_1 %f16vec_1
%val14 = OpExtInst %f16vec %ext_inst FMax %f16vec_1 %f16vec_1
%val15 = OpExtInst %f16vec %ext_inst FClamp %f16vec_1 %f16vec_1 %f16vec_1
%val16 = OpExtInst %f16vec %ext_inst NClamp %f16vec_1 %f16vec_1 %f16vec_1
%val17 = OpExtInst %f16vec %ext_inst Step %f16vec_1 %f16vec_1
%val18 = OpExtInst %f16vec %ext_inst Exp %f16vec_1
%val19 = OpExtInst %f16vec %ext_inst Log %f16vec_1
%val20 = OpExtInst %f16vec %ext_inst Tanh %f16vec_1
%val21 = OpExtInst %f16vec %ext_inst Atan %f16vec_1
%val22 = OpExtInst %f16vec %ext_inst Fma %f16vec_1 %f16vec_1 %f16vec_1
%val23 = OpExtInst %u32vec %ext_inst UMin %u32vec_1 %u32vec_1
%val24 = OpExtInst %u32vec %ext_inst UMax %u32vec_1 %u32vec_1
%val25 = OpExtInst %u32vec %ext_inst UClamp %u32vec_1 %u32vec_1 %u32vec_1
%val26 = OpExtInst %s32vec %ext_inst SMin %s32vec_1 %s32vec_1
%val27 = OpExtInst %s32vec %ext_inst SMax %s32vec_1 %s32vec_1
%val28 = OpExtInst %s32vec %ext_inst SClamp %s32vec_1 %s32vec_1 %s32vec_1
%val29 = OpShiftRightLogical %u32vec %u32vec_1 %u32vec_1
%val30 = OpShiftRightArithmetic %u32vec %u32vec_1 %u32vec_1
%val31 = OpShiftLeftLogical %u32vec %u32vec_1 %u32vec_1
%val32 = OpBitwiseOr %u32vec %u32vec_1 %u32vec_1
%val33 = OpBitwiseXor %u32vec %u32vec_1 %u32vec_1
%val34 = OpBitwiseAnd %u32vec %u32vec_1 %u32vec_1
%val35 = OpNot %u32vec %u32vec_1
)";

  CompileSuccessfully(GenerateCoopVecCode("", body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, CoopVecFMulPass) {
  const std::string body = R"(
%val1 = OpFMul %f16vec %f16vec_1 %f16vec_1
)";

  CompileSuccessfully(GenerateCoopVecCode("", body).c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateArithmetics, CoopVecVectorTimesScalarMismatchFail) {
  const std::string body = R"(
%val1 = OpVectorTimesScalar %f16vec %f16vec_1 %f32_1
)";

  CompileSuccessfully(GenerateCoopVecCode("", body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected scalar operand type to be equal to the component "
                "type of the vector operand: VectorTimesScalar"));
}

TEST_F(ValidateArithmetics, CoopVecDimFail) {
  const std::string body = R"(
%val1 = OpFMul %f16vec %f16vec_1 %f16vec4_1
)";

  CompileSuccessfully(GenerateCoopVecCode("", body).c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected number of components to be identical"));
}

TEST_F(ValidateArithmetics, CoopVecComponentTypeNotScalarNumeric) {
  const std::string types = R"(
%bad = OpTypeCooperativeVectorNV %bool %u32_8
)";

  CompileSuccessfully(GenerateCoopVecCode(types, "").c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpTypeCooperativeVectorNV Component Type <id> "
                        "'5[%bool]' is not a scalar numerical type."));
}

TEST_F(ValidateArithmetics, CoopVecDimNotConstantInt) {
  const std::string types = R"(
%bad = OpTypeCooperativeVectorNV %f16 %f32_1
)";

  CompileSuccessfully(GenerateCoopVecCode(types, "").c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpTypeCooperativeVectorNV component count <id> "
                        "'19[%float_1]' is not a constant integer type"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
