// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#ifndef VM_FLOW_GRAPH_COMPILER_X64_H_
#define VM_FLOW_GRAPH_COMPILER_X64_H_

#ifndef VM_FLOW_GRAPH_COMPILER_H_
#error Include flow_graph_compiler.h instead of flow_graph_compiler_x64.h.
#endif

#include "vm/assembler.h"
#include "vm/code_generator.h"
#include "vm/intermediate_language.h"

namespace dart {

class Code;
template <typename T> class GrowableArray;
class ParsedFunction;

class FlowGraphCompiler : public FlowGraphVisitor {
 public:
  FlowGraphCompiler(Assembler* assembler,
                    const ParsedFunction& parsed_function,
                    const GrowableArray<BlockEntryInstr*>* blocks)
      : assembler_(assembler),
        parsed_function_(parsed_function),
        blocks_(blocks),
        pc_descriptors_list_(new CodeGenerator::DescriptorList()),
        stack_local_count_(0) { }

  virtual ~FlowGraphCompiler() { }

  void CompileGraph();

  // Infrastructure copied from class CodeGenerator or stubbed out.
  void FinalizePcDescriptors(const Code& code);
  void FinalizeVarDescriptors(const Code& code);
  void FinalizeExceptionHandlers(const Code& code);

 private:
  int stack_local_count() const { return stack_local_count_; }
  void set_stack_local_count(int count) { stack_local_count_ = count; }

  // Bail out of the flow graph compiler.  Does not return to the caller.
  void Bailout(const char* reason);

  // Emit code to perform a computation, leaving its value in RAX.
#define DECLARE_VISIT_COMPUTATION(ShortName, ClassName)                        \
  virtual void Visit##ShortName(ClassName* comp);

  // Each visit function compiles a type of instruction.
#define DECLARE_VISIT_INSTRUCTION(ShortName)                                   \
  virtual void Visit##ShortName(ShortName##Instr* instr);

  FOR_EACH_COMPUTATION(DECLARE_VISIT_COMPUTATION)
  FOR_EACH_INSTRUCTION(DECLARE_VISIT_INSTRUCTION)

#undef DECLARE_VISIT_COMPUTATION
#undef DECLARE_VISIT_INSTRUCTION

  // Emit code to load a Value into register RAX.
  void LoadValue(Value* value);

  // Emit an instance call.
  void EmitInstanceCall(intptr_t node_id,
                        intptr_t token_index,
                        const String& function_name,
                        intptr_t argument_count,
                        const Array& argument_names,
                        intptr_t checked_argument_count);

  // Infrastructure copied from class CodeGenerator.
  void GenerateCall(intptr_t token_index,
                    const ExternalLabel* label,
                    PcDescriptors::Kind kind);
  void GenerateCallRuntime(intptr_t node_id,
                           intptr_t token_index,
                           const RuntimeEntry& entry);
  void AddCurrentDescriptor(PcDescriptors::Kind kind,
                            intptr_t node_id,
                            intptr_t token_index);

  Assembler* assembler_;
  const ParsedFunction& parsed_function_;
  const GrowableArray<BlockEntryInstr*>* blocks_;

  CodeGenerator::DescriptorList* pc_descriptors_list_;
  int stack_local_count_;

  DISALLOW_COPY_AND_ASSIGN(FlowGraphCompiler);
};

}  // namespace dart

#endif  // VM_FLOW_GRAPH_COMPILER_X64_H_
