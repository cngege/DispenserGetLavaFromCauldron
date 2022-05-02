#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ItemStack.hpp>
#include <MC/DispenserBlock.hpp>
//#include <MC/CauldronBlock.hpp>
//#include <MC/CauldronBlockActor.hpp>
#include <MC/Container.hpp>
#include <MC/Item.hpp>
#include "Version.h"
#include <LLAPI.h>
#include <ServerAPI.h>

Logger DispenserGetLavaFromCauldronLogger("DispenserGetLavaFromCauldron");

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

void PluginInit()
{
    CheckProtocolVersion();

}

// a7 发射的物品所在的格子数
THook(void, "?ejectItem@DispenserBlock@@IEBAXAEAVBlockSource@@AEBVVec3@@EAEBVItemStack@@AEAVContainer@@H@Z", DispenserBlock* a1,
    BlockSource* a2, Vec3* a3, FaceID a4, ItemStack* a5, Container* a6, unsigned int a7) {
    auto pos = a3->toBlockPos();

    //DispenserGetLavaFromCauldronLogger.info("物品类型:{0}", a5->getTypeName());
    //DispenserGetLavaFromCauldronLogger.info("方块类型:{0}", a2->getBlock(pos).getTypeName());

    //1.判断是不是 空的桶 并且 炼药锅是盛着岩浆的
    if (a5->getTypeName() == "minecraft:bucket" && a2->getBlock(pos).getTypeName() == "minecraft:lava_cauldron")
    {
        //2. 移除空桶
        a5->remove(1);

        //3. 岩浆炼药锅变成空炼药锅(minecraft:cauldron)
        Level::setBlock(pos, a2->getDimensionId(), std::string("minecraft:cauldron"), NULL);
        //4. 生成岩浆桶
        ItemStack* lava_bucket = ItemStack::create("minecraft:lava_bucket");
        bool spawned = a6->addItem(*lava_bucket);

        //5. 生成失败 则将岩浆桶发射出去
        if (!spawned)
        {
            return DispenserBlock::ejectItem(*a2, *a3, (unsigned char)a4, *lava_bucket);
        }
        return;
    }

    return original(a1, a2, a3, a4, a5, a6, a7);
}