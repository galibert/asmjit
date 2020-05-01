// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// This file is used to test opcodes generated by AsmJit. Output can be
// disassembled in your IDE or by your favorite disassembler. Instructions
// are grouped by category and then sorted alphabetically.

#include <asmjit/x86.h>
#include <stdio.h>
#include <stdlib.h>

#include "./asmjit_test_opcode.h"

using namespace asmjit;

struct OpcodeDumpInfo {
  uint32_t archId;
  bool useRex1;
  bool useRex2;
};

static const char* archIdToString(uint32_t archId) {
  switch (archId) {
    case ArchInfo::kIdNone: return "None";
    case ArchInfo::kIdX86 : return "X86";
    case ArchInfo::kIdX64 : return "X64";
    case ArchInfo::kIdA32 : return "A32";
    case ArchInfo::kIdA64 : return "A64";

    default:
      return "<unknown>";
  }
}

struct TestErrorHandler : public ErrorHandler {
  virtual void handleError(Error err, const char* message, BaseEmitter* origin) {
    (void)origin;
    printf("ERROR 0x%08X: %s\n", err, message);
  }
};

typedef void (*VoidFunc)(void);

int main(int argc, char* argv[]) {
  ASMJIT_UNUSED(argc);
  ASMJIT_UNUSED(argv);

  TestErrorHandler eh;

  OpcodeDumpInfo infoList[] = {
    { ArchInfo::kIdX86, false, false },
    { ArchInfo::kIdX64, false, false },
    { ArchInfo::kIdX64, false, true  },
    { ArchInfo::kIdX64, true , false },
    { ArchInfo::kIdX64, true , true  }
  };

  for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(infoList); i++) {
    const OpcodeDumpInfo& info = infoList[i];

    printf("Opcodes [ARCH=%s REX1=%s REX2=%s]\n",
      archIdToString(info.archId),
      info.useRex1 ? "true" : "false",
      info.useRex2 ? "true" : "false");

    CodeHolder code;
    code.init(CodeInfo(info.archId));
    code.setErrorHandler(&eh);

    #ifndef ASMJIT_NO_LOGGING
    FileLogger logger(stdout);
    logger.addFlags(FormatOptions::kFlagMachineCode);
    code.setLogger(&logger);
    #endif

    x86::Assembler a(&code);
    asmtest::generateOpcodes(a.as<x86::Emitter>(), info.useRex1, info.useRex2);

    // If this is the host architecture the code generated can be executed
    // for debugging purposes (the first instruction is ret anyway).
    if (code.archId() == ArchInfo::kIdHost) {
      JitRuntime runtime;
      VoidFunc p;

      Error err = runtime.add(&p, &code);
      if (err == kErrorOk) p();
    }
  }

  return 0;
}
