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

#ifndef SOURCE_OPT_LOCAL_ROOT_PASS_H_
#define SOURCE_OPT_LOCAL_ROOT_PASS_H_

#include "source/opt/ir_context.h"
#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

class LocalRootPass : public Pass {
 public:
  const char* name() const override { return "local-root-pass"; }
  Status Process() override;

  IRContext::Analysis GetPreservedAnalyses() override {
    return IRContext::kAnalysisNone;
  }

 private:
   analysis::TypeManager *type_mgr;
   analysis::ConstantManager *const_mgr;
   analysis::DefUseManager *defuse_mgr;
   analysis::DecorationManager *dec_mgr;
   FeatureManager *feat_mgr;

   const uint32_t remapDescriptorSet = 8;

   enum DescriptorType {
     TypeSampler = 0,
     TypeCombinedSampler = 1,
     TypeSampledImage = 2,
     TypeStorageImage = 3,
     TypeUniformTexelBuffer = 4,
     TypeStorageTexelBuffer = 5,
     TypeUniformBuffer = 6,
     TypeStorageBuffer = 7,
     TypeMax = 8,
     TypeInvalid = 0xFF
   };

   uint32_t varSbtId;

   struct RemapInfo
   {
     Instruction *origInst;
     uint32_t origId;
     uint32_t newId;
     uint32_t arraySize;
     uint32_t offset;
     DescriptorType type;
   };

   void Initialize();
   void AddSBTVariable();

   void GetVariableInfo(Instruction *varInst, DescriptorType *, uint32_t *);
   void ScanVariables();
   void CreateVariable(RemapInfo *);
   void PatchVariables();

   uint32_t curOffsets[TypeMax];
   std::vector<RemapInfo *> varsToRemap;
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_LOCAL_ROOT_PASS_H_
