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

#include "source/opt/local_root_pass.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

Pass::Status LocalRootPass::Process() {

  Initialize();
  AddSBTVariable();
  ScanVariables();
  return Status::SuccessWithChange;
}

void LocalRootPass::Initialize()
{
  type_mgr = context()->get_type_mgr();
  const_mgr = context()->get_constant_mgr();
  defuse_mgr = context()->get_def_use_mgr();
  dec_mgr = context()->get_decoration_mgr();

}
void LocalRootPass::AddSBTVariable()
{
  analysis::Integer uint(32, false);
  uint32_t uintTypeId = type_mgr->GetTypeInstruction(&uint);
  analysis::Type *uintType = type_mgr->GetType(uintTypeId);

  const analysis::Constant *arraySize = const_mgr->GetConstant(uintType, { DescriptorType::TypeMax });
  Instruction *arraySizeInst = const_mgr->GetDefiningInstruction(arraySize);
  uint32_t arraySizeId = arraySizeInst->result_id();

  analysis::Array uintArray(uintType, arraySizeId);
  uint32_t uintArrayTypeId = type_mgr->GetTypeInstruction(&uintArray);
  analysis::Type *arrayType = type_mgr->GetType(uintArrayTypeId);

  analysis::Struct sbtStruct({ arrayType });
  uint32_t sbtStructTypeId = type_mgr->GetTypeInstruction(&sbtStruct);
  //analysis::Type *sbtStructType = type_mgr->GetType(sbtStructTypeId);

  uint32_t pointerTypeId = type_mgr->FindPointerToType(sbtStructTypeId, SpvStorageClassShaderRecordBufferNV);

  uint32_t nextId = TakeNextId();

  std::unique_ptr<Instruction> sbtVarInst(new Instruction(
    context(), SpvOpVariable, pointerTypeId, nextId,
    { { spv_operand_type_t::SPV_OPERAND_TYPE_LITERAL_INTEGER,
    { SpvStorageClassShaderRecordBufferNV } } }));

  context()->AddGlobalValue(std::move(sbtVarInst));

}
void LocalRootPass::GetVariableInfo(Instruction *varInst, DescriptorType *type, uint32_t *arraySize)
{
  DescriptorType rvType = TypeInvalid;
  uint32_t rvArraySize = 0;

  uint32_t varTypeId = varInst->type_id();
  analysis::Type *varType = type_mgr->GetType(varTypeId);
  assert(varType->AsPointer());
  analysis::Pointer *pointerType = varType->AsPointer();

  SpvStorageClass sc = pointerType->storage_class();
  const analysis::Type *baseType = pointerType->pointee_type();

  if (baseType->AsArray()) {
    uint32_t arrayLenId = baseType->AsArray()->LengthId();
    Instruction *arrayLenInst = defuse_mgr->GetDef(arrayLenId);
    rvArraySize = const_mgr->GetConstantFromInst(arrayLenInst)->GetS32();
    baseType = baseType->AsArray()->element_type();
  }
  if (baseType->AsRuntimeArray()) {
    baseType = baseType->AsRuntimeArray()->element_type();
  }

  uint32_t baseTypeId = type_mgr->GetId(baseType);

  // Vulkan only supports 1D arrays
  assert(!baseType->AsArray());

  if (baseType->AsSampler()) {
    rvType = TypeSampler;
  } else if (baseType->AsSampledImage()) {
    rvType = TypeSampledImage;
  } else if (baseType->AsImage()) {
    const analysis::Image *imageType = baseType->AsImage();
    bool isBuffer = false;
    if (imageType->dim() == SpvDimBuffer) {
      isBuffer = true;
    }
    uint32_t sampled = baseType->AsImage()->sampled();
    if (sampled == 1)
      rvType = isBuffer ? TypeUniformTexelBuffer : TypeSampledImage;
    else
      rvType = isBuffer ? TypeStorageTexelBuffer : TypeStorageImage;
  } else if (baseType->AsStruct()) {
    bool isBlock = false;
    context()->get_decoration_mgr()->ForEachDecoration(
      baseTypeId, SpvDecorationBlock,
      [&isBlock](const Instruction&) { isBlock = true; });
    if (isBlock) {
      bool is_buffer_block = false;
      context()->get_decoration_mgr()->ForEachDecoration(
        baseTypeId, SpvDecorationBufferBlock,
        [&is_buffer_block](const Instruction&) { is_buffer_block = true; });
      if (is_buffer_block || sc == SpvStorageClassStorageBuffer)
        rvType = TypeStorageBuffer;
      else
        rvType = TypeUniformBuffer;
    }
  }

  if (type) {
    *type = rvType;
  }
  if (arraySize) {
    *arraySize = rvArraySize;
  }

}
void LocalRootPass::ScanVariables()
{
  for (auto iter = get_module()->types_values_begin();
    iter != get_module()->types_values_end(); ++iter) {
    if (iter->opcode() == SpvOpVariable) {
      DescriptorType dType;
      uint32_t size;
      GetVariableInfo(&*iter, &dType, &size);
    }
    
    }
}
}  // namespace opt
}  // namespace spvtools
