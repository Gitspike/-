#include "llvm-10/llvm/Pass.h"
#include "llvm-10/llvm/IR/Module.h"
#include "llvm-10/llvm/IR/IRBuilder.h"
#include "llvm-10/llvm/Support/raw_ostream.h"
#include "llvm-10/llvm/IR/Instructions.h"
#include "llvm-10/llvm/IR/Constants.h"
#include "llvm-10/llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace
{
	class LocalOpts final : public ModulePass
	{
	public:
		static char ID; // Pass identification

		LocalOpts() : ModulePass(ID) {}
		virtual ~LocalOpts() override {}

		virtual void getAnalysisUsage(AnalysisUsage &AU) const override
		{
			AU.setPreservesAll();
		}
		virtual bool runOnModule(Module &M) override
		{
			bool transform = false;
			for (Function &func : M)
				if (runOnFunction(func))
					transform = true;
			dumpInformation();
			return transform;
		}

	private:
		int algebraic_identity_num = 0;
		int strength_reduction_num = 0;
		int multi_inst_optimization_num = 0;

		bool runOnBasicBlock(BasicBlock &B)
		{
			std::list<Instruction *> deleteInst;
			for (BasicBlock::iterator iter = B.begin(); iter != B.end(); iter++)
			{
				Instruction &inst = *iter;

				if (inst.isBinaryOp())
				{
					switch (inst.getOpcode())
					{
					case Instruction::Add:
					case Instruction::Sub:
						// x + 0 = 0 + x = x
						for (int i = 0; i < 2; ++i)
						{
							if (ConstantInt *val = dyn_cast<ConstantInt>(inst.getOperand(i))) //类型转换
							{
								if (val->getZExtValue() == 0)
								{
									++algebraic_identity_num;
									Value *another_val = inst.getOperand(i == 0 ? 1 : 0);
									inst.replaceAllUsesWith(another_val);
									deleteInst.push_back(&inst);
									break;
								}
							}
						}
						// b = a + 1; c = b -1   ==>  b = a + 1; c = b;
						if (ConstantInt *val = dyn_cast<ConstantInt>(inst.getOperand(1)))
						{
							if (Instruction *another_inst = dyn_cast<Instruction>(inst.getOperand(0)))
							{
								// 如果这两条指令刚好一加一减，同时第二个操作数是一样的，则
								if (inst.getOpcode() + another_inst->getOpcode() == Instruction::Add + Instruction::Sub && another_inst->getOperand(1) == val)
								{
									++multi_inst_optimization_num;
									inst.replaceAllUsesWith(another_inst->getOperand(0));
									deleteInst.push_back(&inst);
									break;
								}
							}
						}

						break;
					case Instruction::Mul:
						// x * 2   ==>   x << 1;
						for (int i = 0; i < 2; i++)
						{
							if (ConstantInt *val = dyn_cast<ConstantInt>(inst.getOperand(i)))
							{
								if (val->getZExtValue() == 2)
								{
									++strength_reduction_num;
									//取当前指令的第（）个操作数
									Value *another_val = inst.getOperand(i == 0 ? 1 : 0);
									IRBuilder<> builder(&inst);
									Value *val = builder.CreateShl(another_val, 1); //建立新指令
									inst.replaceAllUsesWith(val);					//替换原来指令
									deleteInst.push_back(&inst);
									break;
								}
							}
						}
						break;
					}
				}
			}
			// Instruction必须在遍历BasicBlock之后再删除，否则可能会导致iterator指向错误的地方
			for (Instruction *inst : deleteInst)
				if (inst->isSafeToRemove()) //替换指令
					// 注意，用erase而不是remove
					inst->eraseFromParent();
			return true;
		}
		virtual bool runOnFunction(Function &F)
		{
			bool transform = false;
			for (BasicBlock &block : F)
				if (runOnBasicBlock(block))
					transform = true;
			return transform;
		}
		void dumpInformation()
		{
			outs() << "Transformations applied:\n";
			outs() << "\tAlgebraic Identity: " << algebraic_identity_num << "\n";
			outs() << "\tStrength Reduction: " << strength_reduction_num << "\n";
			outs() << "\tMulti-Inst Optimization: " << multi_inst_optimization_num << "\n";
		}
	};

	char LocalOpts::ID = 0;
	RegisterPass<LocalOpts> X(
		"Local-Opts",
		"Local Opts");
	

} // namespace anonymous
