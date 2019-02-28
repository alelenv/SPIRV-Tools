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
#include "source/opt/ir_builder.h"

namespace spvtools {
namespace opt {

Pass::Status LocalRootPass::Process() {

  Initialize();
  AddSBTVariable();
  ScanVariables();
  PatchVariables();
  return Status::SuccessWithChange;
}

void LocalRootPass::Initialize()
{
  type_mgr = context()->get_type_mgr();
  const_mgr = context()->get_constant_mgr();
  defuse_mgr = context()->get_def_use_mgr();
  dec_mgr = context()->get_decoration_mgr();
  feat_mgr = context()->get_feature_mgr();

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

  get_module()->AddCapability(MakeUnique<Instruction>(
    context(), SpvOpCapability, 0, 0,
    std::initializer_list<Operand>{
      {SPV_OPERAND_TYPE_CAPABILITY, { SpvCapabilityRuntimeDescriptorArrayEXT }}}));

  const std::string extension = "SPV_EXT_descriptor_indexing";
  std::vector<uint32_t> words(extension.size() / 4 + 1, 0);
  char* dst = reinterpret_cast<char*>(words.data());
  strncpy(dst, extension.c_str(), extension.size());
  get_module()->AddExtension(
    MakeUnique<Instruction>(context(), SpvOpExtension, 0, 0,
      std::initializer_list<Operand>{
        {SPV_OPERAND_TYPE_LITERAL_STRING, words}}));

  varSbtId = nextId;

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
  std::vector<RemapInfo *> varsToCreate;
  for (auto iter = get_module()->types_values_begin();
    iter != get_module()->types_values_end(); ++iter) {
    if (iter->opcode() == SpvOpVariable) {
      DescriptorType dType;
      uint32_t arraySize;
      GetVariableInfo(&*iter, &dType, &arraySize);
      if (dType != TypeInvalid) {
        RemapInfo *remapVar = new RemapInfo();
        remapVar->origInst = &*iter;
        remapVar->type = dType;
        remapVar->origId = iter->result_id();
        remapVar->arraySize = arraySize;
        remapVar->offset = curOffsets[dType];
        curOffsets[dType] += arraySize;
        varsToCreate.push_back(remapVar);
      }
    }
  }

  for(auto iter = varsToCreate.begin(); iter != varsToCreate.end();
    ++iter)
  {
    RemapInfo *remapVar = *iter;
    CreateVariable(remapVar);
    varsToRemap.push_back(remapVar);
  }
}

void LocalRootPass::CreateVariable(RemapInfo *remapVar)
{
  // Create a new variable with same base type but make it a runtime array
  // Decorate with some fixed set and binding = DescriptorType

  Instruction *origInst = remapVar->origInst;
  uint32_t varTypeId = origInst->type_id();
  analysis::Type *varType = type_mgr->GetType(varTypeId);
  assert(varType->AsPointer());
  analysis::Pointer *pointerType = varType->AsPointer();

  SpvStorageClass scPtr = pointerType->storage_class();
  const analysis::Type *baseType = pointerType->pointee_type();

  if (baseType->AsArray()) {
    baseType = baseType->AsArray()->element_type();
  }
  if (baseType->AsRuntimeArray()) {
    baseType = baseType->AsRuntimeArray()->element_type();
  }
  analysis::RuntimeArray newVarType(type_mgr->GetRegisteredType(baseType));
  uint32_t newVarTypeId = type_mgr->GetTypeInstruction(&newVarType);

  uint32_t pointerTypeId = type_mgr->FindPointerToType(newVarTypeId, scPtr);

  uint32_t nextId = TakeNextId();


  //Is storage class for variable same as pointer? Probably yes since global
  std::unique_ptr<Instruction> newVarInst(new Instruction(
    context(), SpvOpVariable, pointerTypeId, nextId,
    { { spv_operand_type_t::SPV_OPERAND_TYPE_LITERAL_INTEGER,
    { static_cast<unsigned int>(scPtr) } } }));

  context()->AddGlobalValue(std::move(newVarInst));

  dec_mgr->AddDecorationVal(nextId, SpvDecorationBinding, remapVar->type);
  dec_mgr->AddDecorationVal(nextId, SpvDecorationDescriptorSet, remapDescriptorSet);

  remapVar->newId = nextId;

}
void LocalRootPass::PatchVariables()
{
  for (auto iter = varsToRemap.begin(); iter != varsToRemap.end(); ++iter)
  {
    RemapInfo *remapVar = *iter;
    Instruction *oldVar = remapVar->origInst;
    std::vector<Instruction *> instToPatch;
    get_def_use_mgr()->ForEachUser(oldVar, [&instToPatch](Instruction* user) {
      if (user->opcode() == SpvOpLoad ||
        user->opcode() == SpvOpAccessChain)
        instToPatch.push_back(user);
      //else
      //  assert("Invalid use of variable to remap" && false);
    });

    for (auto iterInst = instToPatch.begin(); iterInst != instToPatch.end(); ++iterInst)
    {
      Instruction *user = *iterInst;
      InstructionBuilder builder(
        context(), user,
        IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);

      analysis::Integer uint(32, false);
      uint32_t uintTypeId = type_mgr->GetTypeInstruction(&uint);
      uint32_t ptrUintType = type_mgr->FindPointerToType(uintTypeId, SpvStorageClassShaderRecordBufferNV);
      analysis::Type *uintType = type_mgr->GetType(uintTypeId);

      const analysis::Constant *constZero = const_mgr->GetConstant(uintType, { 0 });
      Instruction *instZero = const_mgr->GetDefiningInstruction(constZero);
      uint32_t zeroId = instZero->result_id();


      const analysis::Constant *constDescType = const_mgr->GetConstant(uintType, { static_cast<unsigned int>(remapVar->type) });
      Instruction *instDescType = const_mgr->GetDefiningInstruction(constDescType);
      uint32_t descTypeId = instDescType->result_id();

      Instruction *sbtAccessChain = builder.AddAccessChain(ptrUintType, varSbtId, { zeroId, descTypeId });
      Instruction *sbtLoad = builder.AddLoad(uintTypeId, sbtAccessChain->result_id());

      const analysis::Constant *constOffset = const_mgr->GetConstant(uintType, { remapVar->offset });
      Instruction *instOffset = const_mgr->GetDefiningInstruction(constOffset);
      uint32_t offsetId = instOffset->result_id();

      Instruction *finalOffset = builder.AddIAdd(uintTypeId, sbtLoad->result_id(), offsetId);
      Instruction *instToReplace = nullptr;
      if (user->opcode() == SpvOpLoad) {
        //Accessing non array variable
        Instruction *remapAccessChain = builder.AddAccessChain(oldVar->type_id(), remapVar->newId, { finalOffset->result_id() });
        instToReplace = builder.AddLoad(user->type_id(), remapAccessChain->result_id());
      } else {
        assert(user->opcode() == SpvOpAccessChain);
        assert(remapVar->origId == user->GetSingleWordInOperand(0));
        uint32_t operandStart = 1;
        std::vector<uint32_t> operands;
        uint32_t baseId = remapVar->newId;
        if (remapVar->arraySize != 0) {
          uint32_t arrayIndex = user->GetSingleWordInOperand(1);
          Instruction *bitCast = builder.AddUnaryOp(uintTypeId, SpvOpBitcast, arrayIndex);
          finalOffset = builder.AddIAdd(uintTypeId, bitCast->result_id(), finalOffset->result_id());
          operandStart++;
        }
        operands.push_back(finalOffset->result_id());
        for (uint32_t i = operandStart; i < user->NumInOperands(); i++) {
          operands.push_back(user->GetSingleWordInOperand(i));
        }
        instToReplace = builder.AddAccessChain(user->type_id(), baseId, operands);
      }
      context()->ReplaceAllUsesWith(user->result_id(), instToReplace->result_id());
      //Index into SBT and get index
      //OpAccessChain sbtVar 0 DescriptorType
    }
  }
}
}  // namespace opt
}  // namespace spvtools
