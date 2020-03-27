// (c) Copyright 2019 Xilinx Inc. All Rights Reserved.

#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Location.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Translation.h"
#include "AIEDialect.h"

using namespace mlir;
using namespace xilinx;
using namespace xilinx::aie;

static TranslateFromMLIRRegistration
    registration("aie-generate-xaie", [](ModuleOp module, raw_ostream &output) {
        // XAieTile_StrmConnectCct(&(TileInst[col+i][row]),
        //                         XAIETILE_STRSW_SPORT_TRACE((&(TileInst[col+i][row])), 1),
        //                         XAIETILE_STRSW_MPORT_NORTH((&(TileInst[col+i][row])), 0), XAIE_ENABLE);
        for(auto switchboxOp : module.getOps<SwitchboxOp>()) {
          Region &r = switchboxOp.connections();
          Block &b = r.front();
          output << "{auto inst = &(TileInst["
                     << switchboxOp.col().getZExtValue() << "]["
                     << switchboxOp.row().getZExtValue() << "]);\n";
          for (auto connectOp : b.getOps<ConnectOp>()) {
            output << "  XAieTile_StrmConnectCct(inst,\n";
            output << "\tXAIETILE_STRSW_SPORT_" << stringifyWireBundle(connectOp.sourceBundle()).upper() << "(inst, " << connectOp.sourceIndex() << "),\n";
            output << "\tXAIETILE_STRSW_MPORT_" << stringifyWireBundle(connectOp.destBundle()).upper() << "(inst, " << connectOp.destIndex() << "),\n";
            output << "\tXAIE_ENABLE);\n";
          }
          output << "}\n";
        }
        return success();
      });