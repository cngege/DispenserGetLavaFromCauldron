#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/BlockLegacy.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ItemStack.hpp>
#include <MC/DispenserBlock.hpp>
#include <MC/Container.hpp>
#include <MC/CauldronBlockActor.hpp>
#include <MC/CauldronBlock.hpp>
#include <MC/Item.hpp>
#include <MC/BucketItem.hpp>
#include <MC/CompoundTag.hpp>
#include "Version.h"
#include <LLAPI.h>
#include <ServerAPI.h>
//#pragma warning(suppress : 4996)

//Logger DispenserGetLavaFromCauldronLogger("DispenserGetLavaFromCauldron");
void AutoUprade(const std::string minebbs_resid);

inline void CheckProtocolVersion() {
#ifdef TARGET_BDS_PROTOCOL_VERSION
    auto currentProtocol = LL::getServerProtocolVersion();
    if (TARGET_BDS_PROTOCOL_VERSION != currentProtocol)
    {
        logger.warn("Protocol version not match, target version: {}, current version: {}.",
            TARGET_BDS_PROTOCOL_VERSION, currentProtocol);
        logger.warn("This will most likely crash the server, please use the Plugin that matches the BDS version!");
    }
#endif // TARGET_BDS_PROTOCOL_VERSION
}
/*
enum CauldronLiquidType
{
    water = 0,
    lava = 1
};
*/

void PluginInit()
{
    CheckProtocolVersion();
    AutoUprade("4049");
}
/*
// a7 发射的物品所在的格子数
THook(void, "?ejectItem@DispenserBlock@@IEBAXAEAVBlockSource@@AEBVVec3@@EAEBVItemStack@@AEAVContainer@@H@Z", DispenserBlock* a1,
    BlockSource* a2, Vec3* a3, FaceID a4, ItemStack* a5, Container* a6, unsigned int a7) {
    //auto pos = a3->toBlockPos();              //函数 有问题 位置有偏移
    //BlockPos pos = BlockPos::BlockPos(*a3);
    BlockPos pos(*a3);

    //DispenserGetLavaFromCauldronLogger.info("物品类型:{0}", a5->getTypeName());
    //DispenserGetLavaFromCauldronLogger.info("方块类型:{0}", a2->getBlock(pos).getTypeName());

    //空桶 从 岩浆炼药锅 中获取 岩浆桶
    //1.判断是不是 空的桶 并且 炼药锅是盛着岩浆的
    if (a1->getTypeName() == "minecraft:dispenser" && a5->getTypeName() == "minecraft:bucket" && a2->getBlock(pos).getTypeName() == "minecraft:lava_cauldron")
    {
        //2. 移除空桶
        a5->remove(1);

        //3. 岩浆炼药锅变成空炼药锅(minecraft:cauldron)
        Level::setBlock(pos, a2->getDimensionId(), "minecraft:cauldron", NULL);
        //4. 生成岩浆桶
        ItemStack* lava_bucket = ItemStack::create("minecraft:lava_bucket");
        bool spawned = a6->addItem(*lava_bucket);

        //5. 生成失败 也就是容器内满了 则将岩浆桶发射出去
        if (!spawned)
        {
            return DispenserBlock::ejectItem(*a2, *a3, (unsigned char)a4, *lava_bucket);
        }
        return;
    }

    //岩浆桶 将岩浆倒入空炼药锅
    //1. 判断是不是 岩浆桶 并且 炼药锅是空的
    if (a1->getTypeName() == "minecraft:dispenser" && a5->getTypeName() == "minecraft:lava_bucket" && a2->getBlock(pos).getTypeName() == "minecraft:cauldron")
    {
        auto block = const_cast<Block*>(&a2->getBlock(pos));

        //2. 获取炼药锅中的物品情况
        auto nbt = block->getNbt();
        auto nbtTag = nbt.get();
        auto statenbt = nbtTag->getCompoundTag("states");
        auto level = statenbt->getInt("fill_level");
        // std::string snbt = nbtTag->toJson(0);

        //3. 如果炼药锅中的物品是空的才装岩浆
        if (!level)
        {
            auto Cauldron = (CauldronBlock*)(&a2->getBlock(pos).getLegacyBlock());

            //4. 移除岩浆桶
            a5->remove(1);

            //5. 空炼药锅变成岩浆炼药锅 不能使用 Level::setBlock
            Cauldron->setLiquidLevel(*a2, pos, 6, (CauldronLiquidType)1);

            //6. 生成空桶
            ItemStack* lava_bucket = ItemStack::create("minecraft:bucket");
            bool spawned = a6->addItem(*lava_bucket);

            //7. 由于是 岩浆桶 变 空桶 所以不考虑添加物品失败的情况
            //   算了 我还是加上去,虽然理论上是不会添加失败的
            if (!spawned)
            {
                return DispenserBlock::ejectItem(*a2, *a3, (unsigned char)a4, *lava_bucket);
            }
            return;
        }

    }

    return original(a1, a2, a3, a4, a5, a6, a7);
}
*/

// 直接Hook 桶被发射的事件
// a4  发射物品在容器中的位置 0开始
// ret 是否拦截发射 true不发射
THook(bool, "?dispense@BucketItem@@UEBA_NAEAVBlockSource@@AEAVContainer@@HAEBVVec3@@E@Z", BucketItem* thi, BlockSource* a2, Container* a3, int a4, Vec3* a5, unsigned char a6)
{
    BlockPos pos(*a5);

    auto itemstack = a3->getSlot(a4);
    //发射的物品 名称
    auto itemN = itemstack->getTypeName();
    //发射器对着的方块 名称
    auto blockN = a2->getBlock(pos).getTypeName();
    //DispenserGetLavaFromCauldronLogger.info("发射物品名:{},发射方块名称:{}", itemN, blockN);
    //空桶 从 岩浆炼药锅 中获取 岩浆桶
    //1.判断是不是 空的桶 并且 炼药锅是盛着岩浆的
    if (itemN == "minecraft:bucket" && blockN == "minecraft:lava_cauldron")
    {
        //2. 移除空桶
        itemstack->remove(1);

        //3. 岩浆炼药锅变成空炼药锅(minecraft:cauldron)
        Level::setBlock(pos, a2->getDimensionId(), "minecraft:cauldron", NULL);
        //4. 生成岩浆桶
        ItemStack* lava_bucket = ItemStack::create("minecraft:lava_bucket");
        bool spawned = a3->addItem(*lava_bucket);

        //5. 生成失败 也就是容器内满了 则将岩浆桶发射出去
        if (!spawned)
        {
            DispenserBlock::ejectItem(*a2, *a5, a6, *lava_bucket);
            return true;
        }
        return true;
    }

    return original(thi, a2, a3, a4, a5, a6);
}