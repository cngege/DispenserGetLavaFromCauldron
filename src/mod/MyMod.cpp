#include "mod/MyMod.h"

#include <memory>

#include "ll/api/mod/RegisterHelper.h"

#include <ll/api/service/Bedrock.h>
#include <ll/api/memory/Hook.h>
#include <mc/world/Container.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/DispenserBlock.h>
#include <mc/world/level/block/CauldronBlock.h>
#include <mc/world/item/Item.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Spawner.h>
#include <mc/math/Vec3.h>
#include <mc/nbt/CompoundTag.h>


BlockPos Vec3_toBlockPos(Vec3* v) {
    return BlockPos((int)std::floor((double)v->x), (int)std::floor((double)v->y), (int)std::floor((double)v->z));
}

bool Level_setBlock(BlockPos pos, int dimId, std::string block, ushort data) {
    BlockSource* blockSource = const_cast<BlockSource*>(
        &ll::service::getLevel()->getDimension(dimId).get()->getBlockSourceFromMainChunkSource()
    );
    optional_ref<const Block> bl = Block::tryGetFromRegistry(block, data);
    if (!bl.has_value()) {
        return false;
    }
    return blockSource->setBlock(pos, bl, 3, nullptr, nullptr);
}

LL_TYPE_INSTANCE_HOOK(
    BucketItemdispenseHook,
    ll::memory::HookPriority::Normal,
    BucketItem,
    "?dispense@BucketItem@@UEBA_NAEAVBlockSource@@AEAVContainer@@HAEBVVec3@@E@Z",
    bool,
    BlockSource* a2,
    Container* a3,
    int           a4,
    Vec3* a5,
    unsigned char a6
) {
    BlockPos pos = Vec3_toBlockPos(a5);

    auto itemstack = const_cast<ItemStack*>(&a3->getItem(a4));
    // 发射的物品 名称
    auto itemN = itemstack->getTypeName();
    // 拿到方块
    auto block = const_cast<Block*>(&a2->getBlock(pos));
    // 发射器对着的方块 名称
    auto blockN = block->getTypeName();
    //my_mod::MyMod::getInstance().getSelf().getLogger().debug("发射物品名:{},发射方块名称:{}", itemN, blockN);
    // 空桶 从 岩浆炼药锅 中获取 岩浆桶
    // 1.判断是不是 空的桶 并且 炼药锅是盛着岩浆的
    if (itemN == "minecraft:bucket" && blockN == "minecraft:cauldron") {
        // 2.判断炼药锅是不是包含岩浆炼药锅
        //my_mod::MyMod::getInstance().getSelf().getLogger().debug(block->getSerializationId().toSnbt());
        auto states = block->getSerializationId().getCompound("states");
        if (states->getStringTag("cauldron_liquid")->toString() == "lava" && states->getInt("fill_level") == 6) {
            // 3. 移除空桶
            itemstack->remove(1);
            // 4. 岩浆炼药锅变成空炼药锅(minecraft:cauldron)
            Level_setBlock(pos, a2->getDimensionId(), "minecraft:cauldron", NULL);
            // 5. 生成岩浆桶
            ItemStack lava_bucket = ItemStack("minecraft:lava_bucket", 1);
            bool      spawned     = a3->addItem(lava_bucket);

            // 6. 生成失败 也就是容器内满了 则将岩浆桶发射出去
            if (!spawned) {
                DispenserBlock::ejectItem(*a2, *a5, a6, lava_bucket, 1);
                return true;
            }
            return true;
        }
    }

    return origin(a2, a3, a4, a5, a6);
}




namespace my_mod {

static std::unique_ptr<MyMod> instance;

MyMod& MyMod::getInstance() { return *instance; }

bool MyMod::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool MyMod::enable() {
    getSelf().getLogger().debug("Enabling...");
    BucketItemdispenseHook::hook();
    return true;
}

bool MyMod::disable() {
    getSelf().getLogger().debug("Disabling...");
    BucketItemdispenseHook::unhook();
    return true;
}

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::instance);
